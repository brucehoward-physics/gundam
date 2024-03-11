//
// Created by Clark McGrew 24/1/23
//

#include "LikelihoodInterface.h"
#include "GundamGlobals.h"

#include "GenericToolbox.Utils.h"
#include "GenericToolbox.Root.h"
#include "GenericToolbox.Json.h"
#include "Logger.h"


LoggerInit([]{
  Logger::setUserHeaderStr("[LikelihoodInterface]");
});


void LikelihoodInterface::readConfigImpl(){
  LogWarning << "Configuring LikelihoodInterface..." << std::endl;

  // First taking care of the DataSetManager
  JsonType dataSetManagerConfig{};
  GenericToolbox::Json::deprecatedAction(_dataSetManager_.getPropagator().getConfig(), {{"fitSampleSetConfig"}, {"dataSetList"}}, [&]{
    LogAlert << R"("dataSetList" should now be set under "likelihoodInterfaceConfig" instead of "fitSampleSet".)" << std::endl;
    dataSetManagerConfig = GenericToolbox::Json::fetchValue<JsonType>(_dataSetManager_.getPropagator().getConfig(), "fitSampleSetConfig"); // DataSetManager will look for "dataSetList"
  });
  GenericToolbox::Json::deprecatedAction(_dataSetManager_.getPropagator().getConfig(), "dataSetList", [&]{
    LogAlert << R"("dataSetList" should now be set under "likelihoodInterfaceConfig" instead of "propagatorConfig".)" << std::endl;
    dataSetManagerConfig = _dataSetManager_.getPropagator().getConfig();
  });
  dataSetManagerConfig = GenericToolbox::Json::fetchValue(_config_, "dataSetManagerConfig", dataSetManagerConfig);
  _dataSetManager_.readConfig( dataSetManagerConfig );

  //
  JsonType configJointProbability;
  std::string jointProbabilityTypeStr;

  GenericToolbox::Json::deprecatedAction(_dataSetManager_.getPropagator().getSampleSet().getConfig(), "llhStatFunction", [&]{
    LogAlert << R"("llhStatFunction" should now be set under "likelihoodInterfaceConfig/jointProbabilityConfig/type".)" << std::endl;
    jointProbabilityTypeStr = GenericToolbox::Json::fetchValue( _dataSetManager_.getPropagator().getSampleSet().getConfig(), "llhStatFunction", jointProbabilityTypeStr );
  });
  GenericToolbox::Json::deprecatedAction(_dataSetManager_.getPropagator().getSampleSet().getConfig(), "llhConfig", [&]{
    LogAlert << R"("llhConfig" should now be set under "likelihoodInterfaceConfig/jointProbabilityConfig".)" << std::endl;
    configJointProbability = GenericToolbox::Json::fetchValue( _dataSetManager_.getPropagator().getSampleSet().getConfig(), "llhConfig", configJointProbability );
  });

  // new config structure
  configJointProbability = GenericToolbox::Json::fetchValue(_config_, "jointProbabilityConfig", configJointProbability);
  jointProbabilityTypeStr = GenericToolbox::Json::fetchValue(configJointProbability, "type", jointProbabilityTypeStr);

  LogInfo << "Using \"" << jointProbabilityTypeStr << "\" JointProbabilityType." << std::endl;
  _jointProbabilityPtr_ = std::shared_ptr<JointProbability::JointProbabilityBase>( JointProbability::makeJointProbability( jointProbabilityTypeStr ) );
  _jointProbabilityPtr_->readConfig( configJointProbability );

  /// local config
  GenericToolbox::Json::deprecatedAction(_dataSetManager_.getPropagator().getConfig(), "showEventBreakdown", [&]{
    LogAlert << R"("showEventBreakdown" should now be set under "likelihoodInterfaceConfig" instead of "propagatorConfig".)" << std::endl;
    _showEventBreakdown_ = GenericToolbox::Json::fetchValue(_dataSetManager_.getPropagator().getConfig(), "showEventBreakdown", _showEventBreakdown_);
  });
  _showEventBreakdown_ = GenericToolbox::Json::fetchValue(_config_, "showEventBreakdown", _showEventBreakdown_);

  LogWarning << "LikelihoodInterface configured." << std::endl;
}
void LikelihoodInterface::initializeImpl() {
  LogWarning << "Initializing LikelihoodInterface..." << std::endl;

  /// Initialize dataset manager in order to use it
  _dataSetManager_.initialize();

  /// Now fill the sample with the data
  this->loadData();

  /// Grab general info about the likelihood
  LogInfo << "Fetching the effective number of fit parameters..." << std::endl;
  _nbParameters_ = 0;
  for( auto& parSet : _dataSetManager_.getPropagator().getParametersManager().getParameterSetsList() ){
    _nbParameters_ += int( parSet.getNbParameters() );
  }
  LogInfo << "Fetching the number of bins parameters..." << std::endl;
  _nbSampleBins_ = 0;
  for( auto& sample : _dataSetManager_.getPropagator().getSampleSet().getSampleList() ){
    _nbSampleBins_ += int( sample.getBinning().getBinList().size() );
  }

  /// Initialize the joint probability function
  _jointProbabilityPtr_->initialize();

  /// some joint fit probability might need to save the value of the nominal histogram.
  /// here we know every parameter is at its nominal value
  LogInfo << "First evaluation of the LLH at the nominal value..." << std::endl;
  _dataSetManager_.getPropagator().getParametersManager().moveParametersToPrior();
  this->propagateAndEvalLikelihood();

  /// move the parameter away from the prior if needed
  if( not _dataSetManager_.getPropagator().getParameterInjectorMc().empty() ){
    LogWarning << "Injecting parameters on MC samples..." << std::endl;
    _dataSetManager_.getPropagator().getParametersManager().injectParameterValues(
        ConfigUtils::getForwardedConfig(_dataSetManager_.getPropagator().getParameterInjectorMc())
    );
    _dataSetManager_.getPropagator().reweightMcEvents();
  }

  //////////////////////////////////////////
  // DON'T MOVE PARAMETERS FROM THIS POINT
  //////////////////////////////////////////

  /// Now printout the event breakdowns
  _dataSetManager_.getPropagator().printBreakdowns();

  LogInfo << "LikelihoodInterface initialized." << std::endl;
}
void LikelihoodInterface::loadData(){
  LogInfo << "Loading data to the defined samples..." << std::endl;

  /// temporarily disable the cache manager while loading. Only using CPU for the initialization
  bool cacheManagerState = GundamGlobals::getEnableCacheManager();
  GundamGlobals::setEnableCacheManager(false);

  /// Load the data slot
  LoadPreset loadPreset{ LoadPreset::Data };
  if( _generateToyExperiment_ ){ loadPreset = LoadPreset::Toy; }
  if( _useAsimovData_ )        { loadPreset = LoadPreset::Asimov; }
  _dataSetManager_.loadPropagator( loadPreset );

  /// do other things
  if( _generateToyExperiment_ ){

    // propagate
    LogInfo << "Propagating prior parameters on the initially loaded events..." << std::endl;
    _dataSetManager_.getPropagator().reweightMcEvents();

    if( _showEventBreakdown_ ){
      LogInfo << "Sample breakdown prior to the throwing:" << std::endl;
      std::cout << getSampleBreakdown() << std::endl;

      // TODO: show events
    }

    _dataSetManager_.getPropagator().getParametersManager().throwParameters();

    // Handling possible masks
    for( auto& parSet : _dataSetManager_.getPropagator().getParametersManager().getParameterSetsList() ){
      if( not parSet.isEnabled() ) continue;

      if( parSet.isMaskForToyGeneration() ){
        LogWarning << parSet.getName() << " will be masked for the toy generation." << std::endl;
        parSet.setMaskedForPropagation( true );
      }
    }

    LogInfo << "Propagating parameters on events..." << std::endl;
    // Make sure before the copy to the data:
    // At this point, MC events have been reweighted using their prior
    // but when using eigen decomp, the conversion eigen -> original has a small computational error
    for( auto& parSet: _dataSetManager_.getPropagator().getParametersManager().getParameterSetsList() ) {
      if( parSet.isEnableEigenDecomp() ) { parSet.propagateEigenToOriginal(); }
    }

  }

  /// copying to the data slot
  auto& propagatorSamples = _dataSetManager_.getPropagator().getSampleSet().getSampleList();
  _dataSampleList_.reserve( propagatorSamples.size() );
  for( auto& sample : propagatorSamples ){ _dataSampleList_.emplace_back( sample.getMcContainer() ); }

  if( not _useAsimovData_ ){
    /// now load the Asimov to te propagator
    _dataSetManager_.loadPropagator( LoadPreset::Asimov );
  }


  /// Re-activating the cache manager if selected
  GundamGlobals::setEnableCacheManager( cacheManagerState );

  LogInfo << "Data loaded." << std::endl;
}

void LikelihoodInterface::propagateAndEvalLikelihood(){
  _dataSetManager_.getPropagator().propagateParameters();
  this->evalLikelihood();
}

double LikelihoodInterface::evalLikelihood() const {
  this->evalStatLikelihood();
  this->evalPenaltyLikelihood();

  _buffer_.updateTotal();
  return _buffer_.totalLikelihood;
}
double LikelihoodInterface::evalStatLikelihood() const {
  _buffer_.statLikelihood = 0.;
  for( auto &sample: _dataSetManager_.getPropagator().getSampleSet().getSampleList()){
    _buffer_.statLikelihood += this->evalStatLikelihood( sample );
  }
  return _buffer_.statLikelihood;
}
double LikelihoodInterface::evalPenaltyLikelihood() const {
  _buffer_.penaltyLikelihood = 0;
  for( auto& parSet : _dataSetManager_.getPropagator().getParametersManager().getParameterSetsList() ){
    _buffer_.penaltyLikelihood += this->evalPenaltyLikelihood( parSet );
  }
  return _buffer_.penaltyLikelihood;
}
double LikelihoodInterface::evalStatLikelihood(const Sample& sample_) const {
  return _jointProbabilityPtr_->eval( sample_ );
}
double LikelihoodInterface::evalPenaltyLikelihood(const ParameterSet& parSet_) const {
  if( not parSet_.isEnabled() ){ return 0; }

  double buffer = 0;

  if( parSet_.getPriorCovarianceMatrix() != nullptr ){
    if( parSet_.isEnableEigenDecomp() ){
      for( const auto& eigenPar : parSet_.getEigenParameterList() ){
        if( eigenPar.isFixed() ){ continue; }
        buffer += TMath::Sq( (eigenPar.getParameterValue() - eigenPar.getPriorValue()) / eigenPar.getStdDevValue() ) ;
      }
    }
    else{
      // make delta vector
      parSet_.updateDeltaVector();

      // compute penalty term with covariance
      buffer =
          (*parSet_.getDeltaVectorPtr())
          * ( (*parSet_.getInverseStrippedCovarianceMatrix()) * (*parSet_.getDeltaVectorPtr()) );
    }
  }

  return buffer;
}
std::string LikelihoodInterface::getSummary() const {
  std::stringstream ss;

  this->evalLikelihood(); // make sure the buffer is up-to-date

  ss << "Total likelihood = " << _buffer_.totalLikelihood;
  ss << std::endl << "Stat likelihood = " << _buffer_.statLikelihood;
  ss << " = sum of: " << GenericToolbox::toString(
      _dataSetManager_.getPropagator().getSampleSet().getSampleList(), [&]( const Sample& sample_){
        std::stringstream ssSub;
        ssSub << sample_.getName() << ": ";
        if( sample_.isEnabled() ){ ssSub << this->evalStatLikelihood( sample_ ); }
        else                     { ssSub << "disabled."; }
        return ssSub.str();
      }
  );
  ss << std::endl << "Penalty likelihood = " << _buffer_.penaltyLikelihood;
  ss << " = sum of: " << GenericToolbox::toString(
      _dataSetManager_.getPropagator().getParametersManager().getParameterSetsList(), [&](const ParameterSet& parSet_){
        std::stringstream ssSub;
        ssSub << parSet_.getName() << ": ";
        if( parSet_.isEnabled() ){ ssSub << this->evalPenaltyLikelihood( parSet_ ); }
        else                     { ssSub << "disabled."; }
        return ssSub.str();
      }
  );
  return ss.str();
}
std::string LikelihoodInterface::getSampleBreakdown() const {

  bool withData{not _dataSampleList_.empty()};

  GenericToolbox::TablePrinter t;

  t << "Sample" << GenericToolbox::TablePrinter::NextColumn;
  t << "MC (# binned event)" << GenericToolbox::TablePrinter::NextColumn;
  if( withData ){ t << "Data (# binned event)" << GenericToolbox::TablePrinter::NextColumn; }
  t << "MC (weighted)" << GenericToolbox::TablePrinter::NextColumn;
  if( withData ){ t << "Data (weighted)" << GenericToolbox::TablePrinter::NextLine; }

  for( auto& sample : _dataSetManager_.getPropagator().getSampleSet().getSampleList() ){
    t << "\"" << sample.getName() << "\"" << GenericToolbox::TablePrinter::NextColumn;
    t << sample.getMcContainer().getNbBinnedEvents() << GenericToolbox::TablePrinter::NextColumn;
    if( withData ){ t << sample.getDataContainer().getNbBinnedEvents() << GenericToolbox::TablePrinter::NextColumn; }
    t << sample.getMcContainer().getSumWeights() << GenericToolbox::TablePrinter::NextColumn;
    if( withData ){ t << sample.getDataContainer().getSumWeights() << GenericToolbox::TablePrinter::NextLine; }
  }

  std::stringstream ss;
  ss << t.generateTableString();
  return ss.str();

}

// An MIT Style License

// Copyright (c) 2022 GUNDAM DEVELOPERS

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// Local Variables:
// mode:c++
// c-basic-offset:2
// compile-command:"$(git rev-parse --show-toplevel)/cmake/gundam-build.sh"
// End:
