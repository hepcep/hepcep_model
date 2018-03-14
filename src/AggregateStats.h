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
#include "FileOut.h"

namespace hepcep {


const std::string GENDER_INFIX = "_gender_";
const std::string HCV_INFIX = "_hcv_";
const std::string RACE_INFIX = "_race_";
const std::string AGEDEC_INFIX = "_agedec_";
const std::string AGEGRP_INFIX = "_agegrp_";
const std::string AREATYPE_INFIX = "_areatype_";
const std::string SYRSRC_INFIX = "_syringesource_";

struct StatKeySuffix {

    std::string gender, race, hcv_state, syr_src, age_grp, age_dec, area_type;

    void set(std::shared_ptr<HCPerson> person);
};

bool filter_true(double tick, std::shared_ptr<HCPerson> person);
bool filter_hcv_rna(double tick,  std::shared_ptr<HCPerson> person);
bool filter_hcv_abpos(double tick,  std::shared_ptr<HCPerson> person);
bool filter_in_treatment(double tick, std::shared_ptr<HCPerson> person);
bool filter_cured(double tick, std::shared_ptr<HCPerson> person);

class AggregateStats {
    
private:
    std::string key;
    std::map<std::string, double> stats;
    bool (*filter_)(double, std::shared_ptr<HCPerson>);

public:
    AggregateStats(std::string base_key, std::vector<std::string>& metrics);
    AggregateStats(std::string base_key, std::vector<std::string>& metrics,  bool (*filter)(double, std::shared_ptr<HCPerson>));
    virtual ~AggregateStats();

    void reset();
    void increment(std::shared_ptr<HCPerson> person, StatKeySuffix& suffixes, double tick);

    double get(const std::string& metric);
    void write(FileOut& out);
    void writeHeader(FileOut& out);

    const std::string baseMetric() const {
        return key;
    }
};


} /* namespace hepcep */

#endif /* SRC_AGGREGATESTATS_H_ */
