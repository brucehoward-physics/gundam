//
// Created by Nadrino on 22/07/2021.
//

#include "Sample.h"
#include "GundamGlobals.h"

#include "GenericToolbox.Json.h"
#include "Logger.h"

#include <vector>
#include <string>
#include <memory>


LoggerInit([]{
  Logger::setUserHeaderStr("[Sample]");
});


void Sample::readConfigImpl(){
  _name_ = GenericToolbox::Json::fetchValue(_config_, "name", _name_);
  LogThrowIf(
      GenericToolbox::hasSubStr(_name_, "/"),
      "Invalid sample name: \"" << _name_ << "\": should not have '/'.");

  LogScopeIndent;
  LogInfo << "Defining sample: " << _name_ << std::endl;

  _binningFilePath_ = GenericToolbox::Json::fetchValue(_config_, {{"binningFilePath"}, {"binningFile"}, {"binning"}}, _binningFilePath_);

  _isEnabled_ = GenericToolbox::Json::fetchValue(_config_, "isEnabled", true);
  LogReturnIf(not _isEnabled_, "\"" << _name_ << "\" is disabled.");

  _selectionCutStr_ = GenericToolbox::Json::fetchValue(_config_, {{"selectionCutStr"}, {"selectionCuts"}}, _selectionCutStr_);
  _enabledDatasetList_ = GenericToolbox::Json::fetchValue(_config_, std::vector<std::string>{"datasets", "dataSets"}, _enabledDatasetList_);
}
void Sample::initializeImpl() {
  if( not _isEnabled_ ) return;

  LogInfo << "Initializing FitSample: " << _name_ << std::endl;

  _binning_.readBinningDefinition( _binningFilePath_ );
  _binning_.sortBins();

  _mcContainer_.setName("MC_" + _name_);
  _mcContainer_.buildHistogram( _binning_ );

  _dataContainer_.setName("Data_" + _name_);
  _dataContainer_.buildHistogram(_binning_);
}

bool Sample::isDatasetValid(const std::string& datasetName_){
  if( _enabledDatasetList_.empty() ){ return true; }
  return std::any_of(_enabledDatasetList_.begin(), _enabledDatasetList_.end(), [&](const std::string& dataset_){
    return dataset_ == "*" or dataset_ == datasetName_;
  });
}

