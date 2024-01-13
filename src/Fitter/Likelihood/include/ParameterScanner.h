//
// Created by Adrien BLANCHET on 07/04/2022.
//

#ifndef GUNDAM_PARAMETERSCANNER_H
#define GUNDAM_PARAMETERSCANNER_H

#include "LikelihoodInterface.h"
#include "Parameter.h"
#include "JsonBaseClass.h"

#include "nlohmann/json.hpp"
#include "TDirectory.h"
#include "TGraph.h"

#include <utility>


class ParameterScanner : public JsonBaseClass {

private:
  struct ScanData;
  struct GraphEntry;

protected:
  void readConfigImpl() override;
  void initializeImpl() override;

public:
  // setters
  void setNbPoints(int nbPoints_){ _nbPoints_ = nbPoints_; }
  void setNbPointsLineScan(int nbPointsLineScan_){ _nbPointsLineScan_ = nbPointsLineScan_; }
  void setLikelihoodInterfacePtr(LikelihoodInterface* likelihoodInterfacePtr_){ _likelihoodInterfacePtr_ = likelihoodInterfacePtr_; }

  // const getters
  [[nodiscard]] bool isUseParameterLimits() const{ return _useParameterLimits_; }
  [[nodiscard]] int getNbPoints() const{ return _nbPoints_; }
  [[nodiscard]] const std::pair<double, double> &getParameterSigmaRange() const{ return _parameterSigmaRange_; }
  [[nodiscard]] const JsonType &getVarsConfig() const { return _varsConfig_; };
  [[nodiscard]] const std::vector<GraphEntry> &getGraphEntriesBuf() const { return _graphEntriesBuf_; };

  // Core
  void scanFitParameters(std::vector<Parameter>& par_, TDirectory* saveDir_);
  void scanFitParameter(Parameter& par_, TDirectory* saveDir_);
  void scanSegment(TDirectory *saveDir_, const JsonType &end_, const JsonType &start_ = JsonType(), int nSteps_=-1);
  void generateOneSigmaPlots(TDirectory* saveDir_);
  void varyEvenRates(const std::vector<double>& paramVariationList_, TDirectory* saveDir_);

  // Statics
  static void muteLogger();
  static void unmuteLogger();
  static void writeGraphEntry(GraphEntry& entry_, TDirectory* saveDir_);

private:
  // Config
  bool _useParameterLimits_{true};
  int _nbPoints_{100};
  int _nbPointsLineScan_{_nbPoints_};
  std::pair<double, double> _parameterSigmaRange_{-3,3};
  JsonType _varsConfig_{};

  // Internals
  LikelihoodInterface* _likelihoodInterfacePtr_{nullptr};

  struct ScanData{
    std::string folder{};
    std::string title{};
    std::string yTitle{};
    std::vector<double> yPoints{};
    std::function<double()> evalY{};
  };
  std::vector<ScanData> _scanDataDict_;

  struct GraphEntry{
    ScanData* scanDataPtr{nullptr};
    Parameter* fitParPtr{nullptr};
    TGraph graph{};
  };
  std::vector<GraphEntry> _graphEntriesBuf_;


};


#endif //GUNDAM_PARAMETERSCANNER_H