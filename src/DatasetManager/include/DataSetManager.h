//
// Created by Nadrino on 04/03/2024.
//

#ifndef GUNDAM_DATASET_MANAGER_H
#define GUNDAM_DATASET_MANAGER_H

#include "DatasetDefinition.h"
#include "EventTreeWriter.h"
#include "Propagator.h"
#include "JsonBaseClass.h"


#define ENUM_NAME LoadPreset
#define ENUM_FIELDS \
  ENUM_FIELD( Unset, 0 ) \
  ENUM_FIELD( Asimov ) \
  ENUM_FIELD( Data ) \
  ENUM_FIELD( Toy )
#include "GenericToolbox.MakeEnum.h"


class DataSetManager : public JsonBaseClass {

protected:
  void readConfigImpl() override;
  void initializeImpl() override;

public:
  DataSetManager() = default;

  // const-getters
  [[nodiscard]] const Propagator& getPropagator() const{ return _propagator_; }
  [[nodiscard]] const EventTreeWriter& getTreeWriter() const{ return _treeWriter_; }
  [[nodiscard]] const std::vector<DatasetDefinition>& getDataSetList() const{ return _dataSetList_; }

  // mutable-getters
  Propagator& getPropagator(){ return _propagator_; }
  EventTreeWriter& getTreeWriter(){ return _treeWriter_; }
  std::vector<DatasetDefinition>& getDataSetList(){ return _dataSetList_; }

  // core
  void loadPropagator(LoadPreset loadPreset_);

protected:
  void loadData();

private:
  // internals
  Propagator _propagator_{};
  EventTreeWriter _treeWriter_{};
  std::vector<DatasetDefinition> _dataSetList_{};
  LoadPreset _loadPreset_{LoadPreset::Unset};

};


#endif //GUNDAM_DATASET_MANAGER_H
