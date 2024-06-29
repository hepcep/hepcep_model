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
bool filter_infected_today(double tick, std::shared_ptr<HCPerson> person);

bool filter_in_opioid_treatment(double tick, std::shared_ptr<HCPerson> person);

bool filter_in_opioid_treatment_M(double tick, std::shared_ptr<HCPerson> person);
bool filter_in_opioid_treatment_B(double tick, std::shared_ptr<HCPerson> person);
bool filter_in_opioid_treatment_N(double tick, std::shared_ptr<HCPerson> person);

bool filter_hcv_state_susceptible(double tick, std::shared_ptr<HCPerson> person);
bool filter_hcv_state_acute(double tick, std::shared_ptr<HCPerson> person);
bool filter_hcv_state_chronic(double tick, std::shared_ptr<HCPerson> person);
bool filter_hcv_state_cured(double tick, std::shared_ptr<HCPerson> person);
bool filter_hcv_state_recovered(double tick, std::shared_ptr<HCPerson> person);

bool filter_vkprofile_none(double tick, std::shared_ptr<HCPerson> person);
bool filter_vkprofile_acute_infect_clear(double tick, std::shared_ptr<HCPerson> person);
bool filter_vkprofile_acute_infect_incomplete(double tick, std::shared_ptr<HCPerson> person);
bool filter_vkprofile_acute_infect_persist(double tick, std::shared_ptr<HCPerson> person);
bool filter_vkprofile_reinfect_high_clearance(double tick, std::shared_ptr<HCPerson> person);
bool filter_vkprofile_reinfect_low_clearance(double tick, std::shared_ptr<HCPerson> person);
bool filter_vkprofile_reinfect_chronic(double tick, std::shared_ptr<HCPerson> person);
bool filter_vkprofile_treatment(double tick, std::shared_ptr<HCPerson> person);

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
