//
// Created by Adrien BLANCHET on 21/05/2021.
//

#ifndef XSLLHFITTER_FITPARAMETER_H
#define XSLLHFITTER_FITPARAMETER_H

#include "vector"
#include "string"
#include "json.hpp"
#include "GenericToolbox.h"

#include "DialSet.h"
#include "AnaEvent.hh"

class FitParameter {

public:
  FitParameter();
  virtual ~FitParameter();

  void reset();

  void setName(const std::string &name);
  void setParameterIndex(int parameterIndex);
  void setParameterValue(double parameterValue);
  void setPriorValue(double priorValue);
  void setStdDevValue(double stdDevValue);
  void setDialSetConfigs(const std::vector<nlohmann::json> &jsonConfigList);
  void setEnableDialSetsSummary(bool enableDialSetsSummary);
  void setDialsWorkingDirectory(const std::string &dialsWorkingDirectory);

  void initialize();

  // Getters
  int getParameterIndex() const;
  const std::string &getName() const;
  double getParameterValue() const;
  DialSet *getCurrentDialSetPtr() const;

  // Core
  void selectDialSet(const std::string& dataSetName_);
  void reweightEvent(AnaEvent* eventPtr_);
  std::string getSummary();
  std::string getTitle();

private:
  // Parameters
  std::string _name_;
  int _parameterIndex_{-1}; // to get the right definition in the json config (in case "name" is not specified)
  double _parameterValue_{};
  double _priorValue_{};
  double _stdDevValue_{};
  nlohmann::json _dialDefinitionsList_;
  bool _enableDialSetsSummary_;
  std::string _dialsWorkingDirectory_;
  bool _isEnabled_{true};

  // Internals
  DialSet* _currentDialSetPtr_{nullptr};
  std::vector<DialSet> _dialSetList_; // one dial set per detector

};


#endif //XSLLHFITTER_FITPARAMETER_H
