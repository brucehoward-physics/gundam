//
// Created by Adrien BLANCHET on 30/07/2021.
//

#ifndef GUNDAM_SAMPLE_ELEMENT_H
#define GUNDAM_SAMPLE_ELEMENT_H

#include "DataBinSet.h"
#include "Event.h"

#include "TH1D.h"

#include <vector>
#include <memory>
#include <string>


class SampleElement{



public:
  SampleElement() = default;



  // mutable-getters
  std::vector<Event> &getEventList(){ return _eventList_; }

  // core
  void buildHistogram(const DataBinSet& binning_);
  void reserveEventMemory(size_t dataSetIndex_, size_t nEvents, const Event &eventBuffer_);
  void shrinkEventList(size_t newTotalSize_);
  void updateBinEventList(int iThread_ = -1);
  void refillHistogram(int iThread_ = -1);

  // event by event poisson throw -> takes into account the finite amount of stat in MC
  void throwEventMcError();

  // generate a toy experiment -> hist content as the asimov -> throw poisson for each bin
  void throwStatError(bool useGaussThrow_ = false);

  [[nodiscard]] double getSumWeights() const;
  [[nodiscard]] size_t getNbBinnedEvents() const;
  [[nodiscard]] std::shared_ptr<TH1D> generateRootHistogram() const; // for the plot generator or for TFile save

  // debug
  [[nodiscard]] std::string getSummary() const;
  friend std::ostream& operator <<( std::ostream& o, const SampleElement& this_ );


#ifdef GUNDAM_USING_CACHE_MANAGER
public:
  void setCacheManagerIndex(int i) {_CacheManagerIndex_ = i;}
  void setCacheManagerValuePointer(const double* v) {_CacheManagerValue_ = v;}
  void setCacheManagerValue2Pointer(const double* v) {_CacheManagerValue2_ = v;}
  void setCacheManagerValidPointer(const bool* v) {_CacheManagerValid_ = v;}
  void setCacheManagerUpdatePointer(void (*p)()) {_CacheManagerUpdate_ = p;}

  [[nodiscard]] int getCacheManagerIndex() const {return _CacheManagerIndex_;}
private:
  // An "opaque" index into the cache that is used to simplify bookkeeping.
  int _CacheManagerIndex_{-1};
  // A pointer to the cached result.
  const double* _CacheManagerValue_{nullptr};
  // A pointer to the cached result.
  const double* _CacheManagerValue2_{nullptr};
  // A pointer to the cache validity flag.
  const bool* _CacheManagerValid_{nullptr};
  // A pointer to a callback to force the cache to be updated.
  void (*_CacheManagerUpdate_)(){nullptr};
#endif

};


#endif //GUNDAM_SAMPLE_ELEMENT_H
