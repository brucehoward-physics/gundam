#include "MinimizerBase.h"
#include "FitterEngine.h"

#include "JsonUtils.h"
#include "Logger.h"

LoggerInit([]{
  Logger::setUserHeaderStr("[MinimizerBase]");
});


MinimizerBase::MinimizerBase(FitterEngine* owner_): _owner_(owner_){}

void MinimizerBase::readConfigImpl(){
  LogInfo << "Reading MinimizerBase config..." << std::endl;

  _enablePostFitErrorEval_ = JsonUtils::fetchValue(_config_, "enablePostFitErrorFit", _enablePostFitErrorEval_);

  bool useNormalizedFitSpace = getLikelihood().getUseNormalizedFitSpace();
  useNormalizedFitSpace = JsonUtils::fetchValue(_config_, "useNormalizedFitSpace", useNormalizedFitSpace);
  getLikelihood().setUseNormalizedFitSpace(useNormalizedFitSpace);

  bool showParametersOnFitMonitor = getLikelihood().getShowParametersOnFitMonitor();
  showParametersOnFitMonitor = JsonUtils::fetchValue(_config_, "showParametersOnFitMonitor", showParametersOnFitMonitor);
  getLikelihood().setShowParametersOnFitMonitor(showParametersOnFitMonitor);

  bool maxNbParametersPerLineOnMonitor = getLikelihood().getMaxNbParametersPerLineOnMonitor();
  maxNbParametersPerLineOnMonitor = JsonUtils::fetchValue(_config_, "maxNbParametersPerLineOnMonitor", maxNbParametersPerLineOnMonitor);
  getLikelihood().setMaxNbParametersPerLineOnMonitor(maxNbParametersPerLineOnMonitor);

  if( GenericToolbox::getTerminalWidth() == 0 ){
    // batch mode
    double monitorBashModeRefreshRateInS = JsonUtils::fetchValue(_config_, "monitorBashModeRefreshRateInS", 30.0);
    getConvergenceMonitor().setMaxRefreshRateInMs(monitorBashModeRefreshRateInS * 1000.);
  }
  else{
    int monitorRefreshRateInMs = JsonUtils::fetchValue(_config_, "monitorRefreshRateInMs", 5000);
    getConvergenceMonitor().setMaxRefreshRateInMs(monitorRefreshRateInMs);
  }

}

void MinimizerBase::initializeImpl(){
  LogInfo << "Initializing the minimizer..." << std::endl;
  LogThrowIf( _owner_== nullptr, "FitterEngine ptr not set." );
}

void MinimizerBase::scanParameters(TDirectory* saveDir_) {
  LogWarning << "Parameter scanning is not implemented for this minimizer"
             << std::endl;
}

std::vector<FitParameter *>& MinimizerBase::getMinimizerFitParameterPtr() {
  return getLikelihood().getMinimizerFitParameterPtr();
}

GenericToolbox::VariablesMonitor &MinimizerBase::getConvergenceMonitor() {
  return getLikelihood().getConvergenceMonitor();
}

Propagator& MinimizerBase::getPropagator() {return owner().getPropagator();}
const Propagator& MinimizerBase::getPropagator() const { return owner().getPropagator(); }

LikelihoodInterface& MinimizerBase::getLikelihood() {return owner().getLikelihood();}
const LikelihoodInterface& MinimizerBase::getLikelihood() const {return owner().getLikelihood();}

void MinimizerBase::printMinimizerFitParameters () {
  // This prints the same set of parameters as are in the vector returned by
  // getMinimizerFitParameterPtr(), but does it by parameter set so that the
  // output is a little more clear.

  LogWarning << std::endl << GenericToolbox::addUpDownBars("Summary of the fit parameters:") << std::endl;
  for( const auto& parSet : getPropagator().getParameterSetsList() ){

    GenericToolbox::TablePrinter t;
    t.setColTitles({ {"Title"}, {"Starting"}, {"Prior"}, {"StdDev"}, {"Min"}, {"Max"}, {"Status"} });

    auto& parList = parSet.getEffectiveParameterList();
    LogWarning << parSet.getName() << ": " << parList.size() << " parameters" << std::endl;
    if( parList.empty() ) continue;

    for( const auto& par : parList ){
      std::string colorStr;
      std::string statusStr;

      if( not par.isEnabled() ) { statusStr = "Disabled"; colorStr = GenericToolbox::ColorCodes::yellowBackground; }
      else if( par.isFixed() )  { statusStr = "Fixed";    colorStr = GenericToolbox::ColorCodes::redBackground; }
      else                      {
        statusStr = PriorType::PriorTypeEnumNamespace::toString(par.getPriorType(), true) + " Prior";
        if(par.getPriorType()==PriorType::Flat) colorStr = GenericToolbox::ColorCodes::blueBackground;
      }

#ifdef NOCOLOR
      colorStr = "";
#endif

      t.addTableLine({
                         par.getTitle(),
                         std::to_string( par.getParameterValue() ),
                         std::to_string( par.getPriorValue() ),
                         std::to_string( par.getStdDevValue() ),
                         std::to_string( par.getMinValue() ),
                         std::to_string( par.getMaxValue() ),
                         statusStr
                     }, colorStr);
    }

    t.printTable();
  }
}


// An MIT Style License

// Copyright (c) 2022 GUNDUM DEVELOPERS

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
