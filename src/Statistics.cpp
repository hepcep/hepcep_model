/*
 * Statistics.cpp
 */
#include <iostream>
#include <memory>

#include "Statistics.h"
#include "HCPerson.h"

namespace hepcep {

Statistics* Statistics::instance_ = new Statistics();

Statistics::Statistics() {
    std::string key = "population";

    std::vector<std::string> suffixes;
    for (auto& gender : Gender::values()) {
        suffixes.push_back("_gender_" + gender.stringValue());
    }

    //for (std::string state : HCV_STATES) {
    //    suffixes.push_back("_hcv_" + state);
    //}


    stats.push_back(AggregateStats(key, suffixes));


}

Statistics::~Statistics() {
}

Statistics* Statistics::instance() {
    return instance_;
}

void Statistics::collectStats(std::map<unsigned int, std::shared_ptr<HCPerson>>& persons) {
    StatKeySuffix sks;
    for (auto& kv : persons) {
        sks.set(kv.second);

        for (auto& stat : stats) {
            stat.increment(sks);
        }
    }

    // TODO write the stats in here somewhere

    for (auto& stat : stats) {
        stat.reset();
    }
}

void Statistics::logStatusChange(LogType logType, HCPerson* person, const std::string& msg) {

}

} /* namespace seir */


