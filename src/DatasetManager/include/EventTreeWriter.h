//
// Created by Nadrino on 19/11/2021.
//

#ifndef GUNDAM_EVENTTREEWRITER_H
#define GUNDAM_EVENTTREEWRITER_H

#include "SampleSet.h"
#include "ParameterSet.h"
#include "EventDialCache.h"
#include "PhysicsEvent.h"

#include "GenericToolbox.Utils.h"

#include <TDirectory.h>

#include <vector>
#include <string>

class EventTreeWriter : public GenericToolbox::ConfigBaseClass<JsonType> {

public:
  EventTreeWriter() = default;

  void setFitSampleSetPtr(const SampleSet *fitSampleSetPtr){ _fitSampleSetPtr_ = fitSampleSetPtr; }
  void setEventDialCachePtr(const EventDialCache *eventDialCachePtr_){ _eventDialCachePtr_ = eventDialCachePtr_; }
  void setParSetListPtr(const std::vector<ParameterSet> *parSetListPtr){ _parSetListPtr_ = parSetListPtr; }

  void writeSamples(TDirectory* saveDir_) const;

  void writeEvents(TDirectory* saveDir_, const std::string& treeName_, const std::vector<PhysicsEvent> & eventList_) const;
  void writeEvents(TDirectory* saveDir_, const std::string& treeName_, const std::vector<const EventDialCache::CacheElem_t*>& cacheSampleList_) const;

protected:
  void readConfigImpl() override;

  // templates related -> ensure the exact same code is used to write standard vars
  template<typename T> void writeEventsTemplate(TDirectory* saveDir_, const std::string& treeName_, const T& eventList_) const;

  static const PhysicsEvent* getEventPtr(const PhysicsEvent& ev_){ return &ev_; }
  static const PhysicsEvent* getEventPtr(const EventDialCache::CacheElem_t* ev_){ return ev_->event; }

  static const std::vector<EventDialCache::DialsElem_t>* getDialElementsPtr(const PhysicsEvent& ev_){ return nullptr; }
  static const std::vector<EventDialCache::DialsElem_t>* getDialElementsPtr(const EventDialCache::CacheElem_t* ev_){ return &ev_->dials; }

private:
  // config
  bool _writeDials_{false};
  int _nPointsPerDial_{3};

  // parameters
  const SampleSet* _fitSampleSetPtr_{nullptr};
  const EventDialCache* _eventDialCachePtr_{nullptr};
  const std::vector<ParameterSet>* _parSetListPtr_{nullptr};


};


#endif //GUNDAM_EVENTTREEWRITER_H
