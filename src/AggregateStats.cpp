/*
 * AggregateStat.cpp
 *
 *  Created on: Mar 5, 2018
 *      Author: nick
 */

#include "AggregateStats.h"

namespace hepcep {

void StatKeySuffix::set(std::shared_ptr<HCPerson> person) {
    gender = person->getGender();
    //hcvstate = hcv_state_to_string(person->getHCVState());
    race = person->getRace();
    syrsrc = person->getSyringeSource();
    //agegrp = person->getAgeGroup();
    //agedec = person->getAgeDecade();
    //areatype = person->getAreaType();
}

AggregateStats::AggregateStats(std::string base_key, std::vector<std::string> key_suffixes) : key(base_key), stats() {
    for (auto suffix : key_suffixes) {
        stats.emplace(key + "_" + suffix, 0.0);
    }
}

void AggregateStats::reset() {
    for (auto kv : stats) {
        stats[kv.first] = 0;
    }
}

void AggregateStats::increment(StatKeySuffix& suffixes) {
    ++stats[key + "_" + suffixes.agedec];
    ++stats[key + "_" + suffixes.agegrp];
    ++stats[key + "_" + suffixes.areatype];
    ++stats[key + "_" + suffixes.syrsrc];
    ++stats[key + "_" + suffixes.race];
    ++stats[key + "_" + suffixes.hcvstate];
    ++stats[key + "_" + suffixes.gender];
    ++stats[key + "_ALL"];
}

AggregateStats::~AggregateStats() {
}

} /* namespace hepcep */
