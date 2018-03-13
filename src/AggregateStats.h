/*
 * AggregateStat.h
 *
 *  Created on: Mar 5, 2018
 *      Author: nick
 */

#ifndef SRC_AGGREGATESTATS_H_
#define SRC_AGGREGATESTATS_H_

#include <map>
#include <vector>
#include <memory>

#include "HCPerson.h"
#include "AgeGroup.h"
#include "AreaType.h"
#include "AgeDecade.h"

namespace hepcep {

struct StatKeySuffix {

    Gender gender;
    Race race;
    HCVState hcv_state;
    HarmReduction syr_src;
    AgeGroup age_grp;
    AgeDecade age_dec;
    AreaType area_type;

    StatKeySuffix();

    void set(std::shared_ptr<HCPerson> person);
};

class AggregateStats {
    
private:
    std::string key;
    std::map<std::string, double> stats;
    
public:
    AggregateStats(std::string base_key, std::vector<std::string> key_suffix);
    virtual ~AggregateStats();

    void reset();
    void increment(StatKeySuffix& suffixes);
};

} /* namespace hepcep */

#endif /* SRC_AGGREGATESTATS_H_ */
