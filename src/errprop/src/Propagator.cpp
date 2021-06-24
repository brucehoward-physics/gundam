//
// Created by Adrien BLANCHET on 11/06/2021.
//

#include <AnaTreeMC.hh>
#include "vector"

#include "GenericToolbox.h"
#include "GenericToolboxRootExt.h"

#include "JsonUtils.h"
#include "Propagator.h"
#include "GlobalVariables.h"
#include "Dial.h"
#include "FitParameterSet.h"

#include "NormalizationDial.h"
#include "SplineDial.h"

LoggerInit([](){
  Logger::setUserHeaderStr("[Propagator]");
})

Propagator::Propagator() { this->reset(); }
Propagator::~Propagator() { this->reset(); }

void Propagator::reset() {
  _isInitialized_ = false;
  _parameterSetsList_.clear();
  _nbThreads_ = 1;

  _stopThreads_ = true;
  for( auto& thread : _threadsList_ ){
    thread.get();
  }
  _threadTriggersList_.clear();
  _threadsList_.clear();
  _stopThreads_ = false;
}

void Propagator::setParameterSetConfig(const json &parameterSetConfig) {
  _parameterSetsConfig_ = parameterSetConfig;
  while( _parameterSetsConfig_.is_string() ){
    // forward json definition in external files
    LogDebug << "Forwarding config with file: " << _parameterSetsConfig_.get<std::string>() << std::endl;
    _parameterSetsConfig_ = JsonUtils::readConfigFile(_parameterSetsConfig_.get<std::string>());
  }
}
void Propagator::setSamplesConfig(const json &samplesConfig) {
  _samplesConfig_ = samplesConfig;
  while( _samplesConfig_.is_string() ){
    // forward json definition in external files
    LogDebug << "Forwarding config with file: " << _samplesConfig_.get<std::string>() << std::endl;
    _samplesConfig_ = JsonUtils::readConfigFile(_samplesConfig_.get<std::string>());
  }
}
void Propagator::setSamplePlotGeneratorConfig(const json &samplePlotGeneratorConfig) {
  _samplePlotGeneratorConfig_ = samplePlotGeneratorConfig;
  while( _samplePlotGeneratorConfig_.is_string() ){
    // forward json definition in external files
    LogDebug << "Forwarding config with file: " << _samplesConfig_.get<std::string>() << std::endl;
    _samplePlotGeneratorConfig_ = JsonUtils::readConfigFile(_samplePlotGeneratorConfig_.get<std::string>());
  }
}
void Propagator::setDataTree(TTree *dataTree_) {
  dataTree = dataTree_;
}
void Propagator::setMcFilePath(const std::string &mcFilePath) {
  mc_file_path = mcFilePath;
}

void Propagator::initialize() {
  LogWarning << __METHOD_NAME__ << std::endl;

  LogTrace << "Parameters..." << std::endl;
  for( const auto& parameterSetConfig : _parameterSetsConfig_ ){
    _parameterSetsList_.emplace_back();
    _parameterSetsList_.back().setJsonConfig(parameterSetConfig);
    _parameterSetsList_.back().initialize();
    LogInfo << _parameterSetsList_.back().getSummary() << std::endl;
  }

  LogTrace << "Samples..." << std::endl;
  for( const auto& sampleConfig : _samplesConfig_ ){
    if( JsonUtils::fetchValue(sampleConfig, "isEnabled", true) ){
      _samplesList_.emplace_back();
      _samplesList_.back().setupWithJsonConfig(sampleConfig);
      _samplesList_.back().setDataTree(dataTree);
      _samplesList_.back().Initialize();
    }
  }

  AnaTreeMC selected_events_AnaTreeMC(mc_file_path, "selectedEvents");
  LogInfo << "Reading and collecting events..." << std::endl;
  std::vector<SignalDef> buf;
  std::vector<AnaSample*> samplePtrList;
  for( auto& sample : _samplesList_ ) samplePtrList.emplace_back(&sample);
  selected_events_AnaTreeMC.GetEvents(samplePtrList, buf, false);


  LogTrace << "Other..." << std::endl;
  _plotGenerator_.setConfig(_samplePlotGeneratorConfig_);
  _plotGenerator_.setSampleListPtr( &_samplesList_ );
  _plotGenerator_.initialize();


  initializeThreads();
  initializeCaches();

  fillEventDialCaches();
  propagateParametersOnSamples();

  for( auto& sample : _samplesList_ ){
    sample.FillEventHist(
      DataType::kAsimov,
      false
    );
  }

  fillSampleHistograms(); // for benchmark

  auto* f = TFile::Open("test.root", "RECREATE");
  _plotGenerator_.generateSamplePlots(GenericToolbox::mkdirTFile(f, "prefit"));

  auto refHistList = _plotGenerator_.getHistHolderList(); // current buffer
  // +1 sigma
  for( auto& parSet : _parameterSetsList_ ){
    for( auto& par : parSet.getParameterList() ){
      par.setParameterValue( par.getPriorValue() + par.getStdDevValue() );
      LogInfo << "+1 sigma on " << parSet.getName() + "/" + par.getTitle() << " -> " << par.getParameterValue() << std::endl;
      propagateParametersOnSamples();
      fillSampleHistograms();
      _plotGenerator_.generateSamplePlots(
        GenericToolbox::mkdirTFile(f, "oneSigma/" + parSet.getName() + "/" + par.getTitle() + "/hist"));
      auto oneSigmaHistList = _plotGenerator_.getHistHolderList();
      _plotGenerator_.generateComparisonPlots(oneSigmaHistList, refHistList, GenericToolbox::mkdirTFile(f, "oneSigma/" + parSet.getName() + "/" + par.getTitle() ));
//      break;
    }
    break;
  }

  LogTrace << "Closing output file..." << std::endl;
  f->Close();
  LogTrace << "Closed" << std::endl;

  _isInitialized_ = true;
}

const std::vector<FitParameterSet> &Propagator::getParameterSetsList() const {
  return _parameterSetsList_;
}

void Propagator::propagateParametersOnSamples() {
  LogDebug << __METHOD_NAME__ << std::endl;

  GenericToolbox::getElapsedTimeSinceLastCallInMicroSeconds(1);

  // dispatch the job on each thread
  for( int iThread = 0 ; iThread < _nbThreads_-1 ; iThread++ ){
    _threadTriggersList_.at(iThread).propagateOnSampleEvents = true; // triggering the workers
  }
  // last thread is always this one
  this->propagateParametersOnSamples(_nbThreads_-1);

  for( int iThread = 0 ; iThread < _nbThreads_-1 ; iThread++ ){
    while( _threadTriggersList_.at(iThread).propagateOnSampleEvents ){
      // wait
    }
  }

  LogTrace << "Reweight took: " << GenericToolbox::getElapsedTimeSinceLastCallStr(1) << std::endl;
}
void Propagator::fillSampleHistograms(){
  LogDebug << __METHOD_NAME__ << std::endl;

  GenericToolbox::getElapsedTimeSinceLastCallStr(1);

  // dispatch the job on each thread
  for( int iThread = 0 ; iThread < _nbThreads_-1 ; iThread++ ){
    _threadTriggersList_.at(iThread).fillSampleHistograms = true; // triggering the workers
  }
  // last thread is always this one

  for( auto& sample : _samplesList_ ){
    if( _nbThreads_ > 1 ) {
      sample.FillMcHistograms(_nbThreads_ - 1);
    }
    else{
      sample.FillMcHistograms();
    }
  }


  for( int iThread = 0 ; iThread < _nbThreads_-1 ; iThread++ ){
    while( _threadTriggersList_.at(iThread).fillSampleHistograms ){
      // wait
    }
  }

  for( auto& sample : _samplesList_ ){
    sample.MergeMcHistogramsThread();
  }

  LogTrace << "Histogram fill took: " << GenericToolbox::getElapsedTimeSinceLastCallStr(1) << std::endl;
}


// Protected
void Propagator::initializeThreads() {

  _nbThreads_ = GlobalVariables::getNbThreads();
  if( _nbThreads_ == 1 ){
    return;
  }

  _threadTriggersList_.resize(_nbThreads_-1);

  std::function<void(int)> asyncLoop = [this](int iThread_){
    while(not _stopThreads_){
      // Pending state loop

      if     ( _threadTriggersList_.at(iThread_).propagateOnSampleEvents ){
        this->propagateParametersOnSamples(iThread_);
        _threadTriggersList_.at(iThread_).propagateOnSampleEvents = false; // toggle off the trigger
      }
      else if( _threadTriggersList_.at(iThread_).fillDialCaches ){
        this->fillEventDialCaches(iThread_);
        _threadTriggersList_.at(iThread_).fillDialCaches = false;
      }
      else if( _threadTriggersList_.at(iThread_).fillSampleHistograms ){
        for( auto& sample : _samplesList_ ){
          sample.FillMcHistograms(iThread_);
        }
        _threadTriggersList_.at(iThread_).fillSampleHistograms = false;
      }

      // Add other jobs there
    }
    _propagatorMutex_.lock();
    LogDebug << "Thread " << iThread_ << " will end now." << std::endl;
    _propagatorMutex_.unlock();
  };

  for( int iThread = 0 ; iThread < _nbThreads_-1 ; iThread++ ){
    _threadsList_.emplace_back(
      std::async( std::launch::async, std::bind(asyncLoop, iThread) )
    );
  }

}
void Propagator::initializeCaches() {
  LogInfo << __METHOD_NAME__ << std::endl;

  for( auto& sample : _samplesList_ ){
    int nEvents = sample.GetN();
    for( int iEvent = 0 ; iEvent < nEvents ; iEvent++ ){
      for( auto& parSet : _parameterSetsList_ ){
        auto* dialCache = sample.GetEvent(iEvent)->getDialCachePtr();
        (*dialCache)[&parSet] = std::vector<Dial*>(parSet.getNbParameters(), nullptr);
      } // parSet
    } // event
  } // sample


}
void Propagator::fillEventDialCaches(){
  LogInfo << __METHOD_NAME__ << std::endl;

  // dispatch the job on each thread
  for( int iThread = 0 ; iThread < _nbThreads_-1 ; iThread++ ){
    _threadTriggersList_.at(iThread).fillDialCaches = true; // triggering the workers
  }
  // first thread is always this one
  this->fillEventDialCaches(_nbThreads_-1);

  for( int iThread = 0 ; iThread < _nbThreads_-1 ; iThread++ ){
    while ( _threadTriggersList_.at(iThread).fillDialCaches ){
      // wait triggering the workers
    }
  }

}

void Propagator::fillEventDialCaches(int iThread_){

  DialSet* parameterDialSetPtr;
  AnaEvent* eventPtr;

  for( auto& sample : _samplesList_ ){

    int nEvents = sample.GetN();
    std::stringstream ss;
    ss << "Filling dial cache for sample: \"" << sample.GetName() << "\"";
    for( int iEvent = 0 ; iEvent < nEvents ; iEvent++ ){
      if( iEvent % _nbThreads_ != iThread_ ){
        continue;
      }

      if( iThread_ == _nbThreads_-1 ){
        GenericToolbox::displayProgressBar(iEvent, nEvents, ss.str());
      }

      eventPtr = sample.GetEvent(iEvent);

      for( auto& parSetPair : *eventPtr->getDialCachePtr() ){
        for( size_t iPar = 0 ; iPar < parSetPair.first->getNbParameters() ; iPar++ ){

          parameterDialSetPtr = parSetPair.first->getFitParameter(iPar).findDialSet(sample.GetDetector());

          // If a formula is defined
          if( parameterDialSetPtr->getApplyConditionFormula() != nullptr
              and eventPtr->evalFormula(parameterDialSetPtr->getApplyConditionFormula()) == 0
          ){
            continue; // SKIP
          }

          for( const auto& dial : parameterDialSetPtr->getDialList() ){
            if( eventPtr->isInBin( dial->getApplyConditionBin() ) ){
              parSetPair.second.at(iPar) = dial.get();
              break; // ok, next parameter
            }
          } // dial

        }
      }

    } // event

  } // sample

}
void Propagator::propagateParametersOnSamples(int iThread_) {
  AnaEvent* eventPtr;
  double weight;

  for( auto& sample : _samplesList_ ){
    int nEvents = sample.GetN();
    for( int iEvent = 0 ; iEvent < nEvents ; iEvent++ ){

      if( iEvent % _nbThreads_ != iThread_ ){
        continue;
      }

      eventPtr = sample.GetEvent(iEvent);
      eventPtr->ResetEvWght();

      // Loop over the parSet that are cached (missing ones won't apply on this event anyway)
      for( auto& parSetDialCache : *eventPtr->getDialCachePtr() ){

        weight = 1;
        for( size_t iPar = 0 ; iPar < parSetDialCache.first->getNbParameters() ; iPar++ ){

          Dial* dialPtr = parSetDialCache.second.at(iPar);
          if( dialPtr == nullptr ) continue;

          // No need to recast dialPtr as a NormDial or whatever, it will automatically fetch the right method
          weight = dialPtr->evalResponse( parSetDialCache.first->getFitParameter(iPar).getParameterValue() );

        }

        // TODO: check if weight cap
        if( weight == 0 ){
          LogError << "0" << std::endl;
          throw std::runtime_error("0 weight");
        }

        eventPtr->AddEvWght(weight);

      } // parSetCache

    } // event
  } // sample

}



