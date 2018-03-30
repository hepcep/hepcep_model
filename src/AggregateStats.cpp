/*
 * AggregateStat.cpp
 *
 *  Created on: Mar 5, 2018
 *      Author: nick
 */

#include "AggregateStats.h"

namespace hepcep {

bool filter_true(double tick, std::shared_ptr<HCPerson> person) {
    return true;
}

bool filter_hcv_rna(double tick,  std::shared_ptr<HCPerson> person) {
    return person->isHcvRNA();
}

bool filter_hcv_abpos(double tick,  std::shared_ptr<HCPerson> person) {
    return person->isHcvABpos();
}

bool filter_in_treatment(double tick, std::shared_ptr<HCPerson> person) {
    return person->isInTreatment();
}

bool filter_cured(double tick, std::shared_ptr<HCPerson> person) {
    return person->isCured();
}

void StatKeySuffix::set(std::shared_ptr<HCPerson> person) {
    gender = GENDER_INFIX + person->getGender().stringValue();
    hcv_state = HCV_INFIX + person->getHCVState().stringValue();
    race = RACE_INFIX + person->getRace().stringValue();
    syr_src = SYRSRC_INFIX + person->getSyringeSource().stringValue();
    age_grp = AGEGRP_INFIX + AgeGroup::getAgeGroup(person->getAge()).stringValue();
    age_dec = AGEDEC_INFIX + AgeDecade::getAgeDecade(person->getAge()).stringValue();
    area_type = AREATYPE_INFIX + AreaType::getAreaType(person->getZipcode()).stringValue();
}

AggregateStats::AggregateStats(std::string base_key, std::vector<std::string>& metrics) : key(base_key), stats(), filter_(&filter_true) {
    for (auto metric : metrics) {
        stats.emplace(key + metric, 0.0);
    }
}

AggregateStats::AggregateStats(std::string base_key, std::vector<std::string>& metrics, bool (*filter)(double, std::shared_ptr<HCPerson>)) : key(base_key), stats(),
        filter_(filter) {
    for (auto metric : metrics) {
        stats.emplace(key + metric, 0.0);
    }
}

void AggregateStats::reset() {
    for (auto& kv : stats) {
        stats[kv.first] = 0;
    }
}

void AggregateStats::increment(std::shared_ptr<HCPerson> person, StatKeySuffix& suffixes, double tick) {
    if (filter_(tick, person)) {
        ++stats[key + suffixes.age_dec];
        ++stats[key + suffixes.age_grp];
        ++stats[key + suffixes.area_type];
        ++stats[key + suffixes.syr_src];
        ++stats[key + suffixes.race];
        ++stats[key + suffixes.hcv_state];
        ++stats[key + suffixes.gender];
        ++stats[key + "_ALL"];
    }
}

void AggregateStats::write(FileOut& out) {
    for (auto& kv : stats) {
        out << ",";
        out << stats[kv.first];
    }
}

void AggregateStats::writeHeader(FileOut& out) {
    for (auto& kv : stats) {
        out << ",";
        out << kv.first;
    }
}

double AggregateStats::get(const std::string& metric) {
    // TODO replace with [] when we know this
    // does not throw any exceptions
    return stats.at(key + metric);
}

AggregateStats::~AggregateStats() {
}


} /* namespace hepcep */
