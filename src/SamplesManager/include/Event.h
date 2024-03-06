//
// Created by Nadrino on 22/07/2021.
//

#ifndef GUNDAM_EVENT_H
#define GUNDAM_EVENT_H

#include "ParameterSet.h"
#include "DataBinSet.h"
#include "DataBin.h"

#include "GenericToolbox.Root.h"
#include "GenericToolbox.Utils.h"

#include "TTree.h"
#include "TFormula.h"

#include <map>
#include <mutex>
#include <vector>
#include <string>
#include <sstream>

class Event{

public:
  struct Indices{
    int dataset{-1}; // which DatasetDefinition?
    Long64_t entry{-1}; // which entry of the TChain?
    int sample{-1}; // this information is lost in the EventDialCache manager
    int bin{-1}; // which bin of the sample?

    [[nodiscard]] std::string getSummary() const{
      std::stringstream ss;
      ss << "dataset(" << dataset << ")";
      ss << ", " << "entry(" << entry << ")";
      ss << ", " << "sample(" << sample << ")";
      ss << ", " << "bin(" << bin << ")";
      return ss.str();
    }
    friend std::ostream& operator <<( std::ostream& o, const Indices& this_ ){ o << this_.getSummary(); return o; }
  };
  struct Weights{
    double base{1};
    double current{1};

    void resetCurrentWeight(){ current = base; }
    [[nodiscard]] std::string getSummary() const{
      std::stringstream ss;
      ss << "base(" << base << ")";
      ss << ", " << "current(" << current << ")";
      return ss.str();
    }
    friend std::ostream& operator <<( std::ostream& o, const Weights& this_ ){ o << this_.getSummary(); return o; }
  };

public:
  Event() = default;

  // setters
  void setCommonVarNameListPtr(const std::shared_ptr<std::vector<std::string>>& commonVarNameListPtr_);
  template<typename T> void setVariable(const T& value_, const std::string& leafName_, size_t arrayIndex_ = 0);

  // const getters
  double getEventWeight() const;
  const Indices& getIndices() const{ return _indices_; }
  const Weights& getWeights() const{ return _weights_; }
  const GenericToolbox::AnyType& getVar(int varIndex_, size_t arrayIndex_ = 0) const { return _varHolderList_[varIndex_][arrayIndex_]; }
  const std::vector<GenericToolbox::AnyType>& getVarHolder(int index_) const { return _varHolderList_[index_]; }
  const std::vector<GenericToolbox::AnyType>& getVarHolder(const std::string &leafName_) const;
  const std::vector<std::vector<GenericToolbox::AnyType>> &getVarHolderList() const { return _varHolderList_; }
  const std::shared_ptr<std::vector<std::string>>& getCommonVarNameListPtr() const { return _commonVarNameListPtr_; }
  const GenericToolbox::AnyType& getVariableAsAnyType(const std::string& leafName_, size_t arrayIndex_ = 0) const;
  template<typename T> auto getVarValue(const std::string& leafName_, size_t arrayIndex_ = 0) const -> T;
  template<typename T> auto getVariable(const std::string& leafName_, size_t arrayIndex_ = 0) const -> const T&;

  // mutable getters
  void* getVariableAddress(const std::string& leafName_, size_t arrayIndex_ = 0);
  Indices& getIndices(){ return _indices_; }
  Weights& getWeights(){ return _weights_; }
  std::vector<std::vector<GenericToolbox::AnyType>> &getVarHolderList(){ return _varHolderList_; }
  GenericToolbox::AnyType& getVariableAsAnyType(const std::string& leafName_, size_t arrayIndex_ = 0);

  // core
  void resizeVarToDoubleCache();
  void invalidateVarToDoubleCache();
  void copyData(const std::vector<const GenericToolbox::LeafForm*>& leafFormList_);
  void allocateMemory(const std::vector<const GenericToolbox::LeafForm*>& leafFormList_);
  bool isInBin(const DataBin& bin_) const;
  int findBinIndex(const DataBinSet& binSet_) const;
  int findBinIndex(const std::vector<DataBin>& binSet_) const;
  int findVarIndex(const std::string& leafName_, bool throwIfNotFound_ = true) const;
  double getVarAsDouble(int varIndex_, size_t arrayIndex_ = 0) const;
  double getVarAsDouble(const std::string& leafName_, size_t arrayIndex_ = 0) const;
  double evalFormula(const TFormula* formulaPtr_, std::vector<int>* indexDict_ = nullptr) const;

  // misc
  void copyVarHolderList(const Event& ref_);
  void copyOnlyExistingVarHolders(const Event& other_);
  void fillBuffer(const std::vector<int>& indexList_, std::vector<double>& buffer_) const;
  void fillBinIndex(const DataBinSet& binSet_){ _indices_.bin = findBinIndex(binSet_); }

  [[nodiscard]] std::string getSummary() const;
  friend std::ostream& operator <<( std::ostream& o, const Event& this_ ){ o << this_.getSummary(); return o; }

private:
  // internals
  Indices _indices_{};
  Weights _weights_{};

  // Data storage variables
  std::shared_ptr<std::vector<std::string>> _commonVarNameListPtr_{nullptr};
  std::vector<std::vector<GenericToolbox::AnyType>> _varHolderList_{};
  mutable std::vector<std::vector<double>> _varToDoubleCache_{};


#ifdef GUNDAM_USING_CACHE_MANAGER
public:
  struct Cache{
    // An "opaque" index into the cache that is used to simplify bookkeeping.
    int index{-1};
    // A pointer to the cached result.
    const double* valuePtr{nullptr};
    // A pointer to the cache validity flag.
    const bool* isValidPtr{nullptr};
    // A pointer to a callback to force the cache to be updated.
    void (*updateCallbackPtr)(){nullptr};

    double getWeight() const;
  };

private:
  Cache _cache_{};

public:
  const Cache& getCache() const{ return _cache_; }
  Cache& getCache(){ return _cache_; }
#endif

};


// TEMPLATES IMPLEMENTATION
template<typename T> auto Event::getVarValue( const std::string &leafName_, size_t arrayIndex_) const -> T {
  return this->getVariable<T>(leafName_, arrayIndex_);
}
template<typename T> auto Event::getVariable( const std::string& leafName_, size_t arrayIndex_) const -> const T&{
  return this->getVariableAsAnyType(leafName_, arrayIndex_).template getValue<T>();
}
template<typename T> void Event::setVariable( const T& value_, const std::string& leafName_, size_t arrayIndex_){
  int index = this->findVarIndex(leafName_, true);
  _varHolderList_[index][arrayIndex_].template getValue<T>() = value_;
  if( not _varToDoubleCache_.empty() ){ _varToDoubleCache_[index][arrayIndex_] = std::nan("unset"); }
}


#endif //GUNDAM_EVENT_H
