//
// Created by Nadrino on 11/02/2021.
//

#ifndef GUNDAM_GUNDAMGLOBALS_H
#define GUNDAM_GUNDAMGLOBALS_H

#include "GenericToolbox.Thread.h"

#include <TTree.h>
#include <TChain.h>
#include <TRandom3.h>

#include <map>
#include <mutex>
#include <memory>

#ifndef GUNDAM_BATCH
#define GUNDAM_SIGMA "σ"
#define GUNDAM_CHI2 "χ²"
#define GUNDAM_DELTA "Δ"
#else
#define GUNDAM_SIGMA "sigma"
#define GUNDAM_CHI2 "chi-squared"
#define GUNDAM_DELTA "delta-"
#endif


#define MAKE_ENUM \
  ENUM_NAME(VerboseLevel) \
  ENUM_ENTRY(NORMAL_MODE, 0) \
  ENUM_ENTRY(MORE_PRINTOUT) \
  ENUM_ENTRY(DEBUG_TRACE) \
  ENUM_ENTRY(INLOOP_TRACE) \
  ENUM_ENTRY(DEV_TRACE)
#include "GenericToolbox.MakeEnum.h"
#undef MAKE_ENUM


class GundamGlobals{

public:

  // Setters
  static void setEnableCacheManager(bool enable = true){ _enableCacheManager_ = enable; }
  static void setLightOutputMode(bool enable_){ _lightOutputMode_ = enable_; }
  static void setDisableDialCache(bool disableDialCache_){ _disableDialCache_ = disableDialCache_; }
  static void setVerboseLevel(VerboseLevel verboseLevel_);
  static void setVerboseLevel(int verboseLevel_);

  // Getters
  static bool getEnableCacheManager(){ return _enableCacheManager_; }
  static bool isDisableDialCache(){ return _disableDialCache_; }
  static bool isLightOutputMode(){ return _lightOutputMode_; }
  static VerboseLevel::EnumType getVerboseLevel(){ return _verboseLevel_.value; }
  static std::mutex& getThreadMutex(){ return _threadMutex_; }
  static GenericToolbox::ParallelWorker &getParallelWorker(){ return _threadPool_; }

private:

  static bool _disableDialCache_;
  static bool _enableCacheManager_;
  static bool _lightOutputMode_;
  static std::mutex _threadMutex_;
  static VerboseLevel _verboseLevel_;
  static GenericToolbox::ParallelWorker _threadPool_;


};

#endif
