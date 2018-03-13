/*
 * AggregateStat.cpp
 *
 *  Created on: Mar 5, 2018
 *      Author: nick
 */

#include "AggregateStats.h"

namespace hepcep {

StatKeySuffix::StatKeySuffix() : gender(Gender::FEMALE), race(Race::HISPANIC), hcv_state(HCVState::UNKNOWN),
        syr_src(HarmReduction::HARM_REDUCTION), age_grp(AgeGroup::LEQ_30), age_dec(AgeDecade::AGE_21_30),
        area_type(AreaType::CITY) {}

void StatKeySuffix::set(std::shared_ptr<HCPerson> person) {
    gender = person->getGender();
    hcv_state = person->getHCVState();
    race = person->getRace();
    syr_src = person->getSyringeSource();
    age_grp = AgeGroup::getAgeGroup(person->getAge());
    age_dec = AgeDecade::getAgeDecade(person->getAge());
    area_type = AreaType::getAreaType(person->getZipcode());
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
    ++stats[key + "_" + suffixes.age_dec.stringValue()];
    ++stats[key + "_" + suffixes.age_grp.stringValue()];
    ++stats[key + "_" + suffixes.area_type.stringValue()];
    ++stats[key + "_" + suffixes.syr_src.stringValue()];
    ++stats[key + "_" + suffixes.race.stringValue()];
    ++stats[key + "_" + suffixes.hcv_state.stringValue()];
    ++stats[key + "_" + suffixes.gender.stringValue()];
    ++stats[key + "_ALL"];
}

AggregateStats::~AggregateStats() {
}

} /* namespace hepcep */
