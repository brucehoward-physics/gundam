//
// Created by Nadrino on 22/07/2021.
//

#ifndef GUNDAM_SAMPLE_H
#define GUNDAM_SAMPLE_H


#include "SampleElement.h"
#include "DataBinSet.h"
#include "Event.h"
#include "JsonBaseClass.h"

#include "nlohmann/json.hpp"
#include <TH1D.h>
#include <TTreeFormula.h>

#include <vector>
#include <string>
#include <memory>


class Sample : public JsonBaseClass {

public:
  struct DatasetProperties{
    size_t dataSetIndex{0};
    size_t eventOffSet{0};
    size_t eventNb{0};
  };

  struct Histogram{
    struct Bin{
      int index{-1};
      double content{0};
      double error{0};
      const DataBin* dataBinPtr{nullptr};
      std::vector<Event*> eventPtrList{};
    };
    std::vector<Bin> binList{};
    int nBins{0};
  };

protected:
  // called through public JsonBaseClass::readConfig() and JsonBaseClass::initialize()
  void readConfigImpl() override;
  void initializeImpl() override;

public:
  // setters
  void setIndex(int index){ _index_ = index; }
  void setName(const std::string &name){ _name_ = name; }
  void setBinningFilePath(const std::string &binningFilePath_){ _binningFilePath_ = binningFilePath_; }
  void setSelectionCutStr(const std::string &selectionCutStr_){ _selectionCutStr_ = selectionCutStr_; }
  void setEnabledDatasetList(const std::vector<std::string>& enabledDatasetList_){ _enabledDatasetList_ = enabledDatasetList_; }

  // const getters
  [[nodiscard]] bool isEnabled() const{ return _isEnabled_; }
  [[nodiscard]] int getIndex() const{ return _index_; }
  [[nodiscard]] const std::string &getName() const{ return _name_; }
  [[nodiscard]] const std::string &getBinningFilePath() const{ return _binningFilePath_; }
  [[nodiscard]] const std::string &getSelectionCutsStr() const{ return _selectionCutStr_; }
  [[nodiscard]] const DataBinSet &getBinning() const{ return _binning_; }
  [[nodiscard]] const Histogram &getHistogram() const{ return _histogram_; }
  [[nodiscard]] const std::vector<Event> &getEventList() const{ return _eventList_; }

  // mutable getters
  DataBinSet &getBinning() { return _binning_; }
  std::vector<Event> &getEventList(){ return _eventList_; }

  // misc
  bool isDatasetValid(const std::string& datasetName_);

private:
  // Yaml
  bool _isEnabled_{false};
  int _index_{-1};
  std::string _name_;
  std::string _selectionCutStr_;
  std::string _binningFilePath_;
  std::vector<std::string> _enabledDatasetList_;

  // Internals
  DataBinSet _binning_;
  std::vector<size_t> _dataSetIndexList_;


  Histogram _histogram_{};
  std::vector<Event> _eventList_{};
  std::vector<DatasetProperties> _loadedDatasetList_{};

};


#endif //GUNDAM_SAMPLE_H
