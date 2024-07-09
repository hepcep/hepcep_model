/*
 * ViralKinetics.h
 *
 *  
 */

#ifndef SRC_VIRALKINETICS_H_
#define SRC_VIRALKINETICS_H_

#include <map>
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <exception>
#include <cmath>

#include "chi_sim/Parameters.h"
#include "parameters_constants.h"
#include "VKProfile.h"

namespace hepcep {

class ViralKinetics{

private:
    static ViralKinetics* instance_;

    // Map of viral load to transmission probability
    std::map<double, double> transmit_prob_map;

    // Maps with time series of each viral load type.  The keys are simply series ID
    //   and the vectors are the time series.
    
    // Naive individual: NI1, NI2, NI3 viral loads
    std::unordered_map<unsigned int, std::vector<double>> viral_loads_acute_infection_clearance;
    std::unordered_map<unsigned int, std::vector<double>> viral_loads_acute_infection_incomplete;
    std::unordered_map<unsigned int, std::vector<double>> viral_loads_acute_infection_persistence;

    // Re-infected individual RI1, RI2, RI3 viral loads
    std::unordered_map<unsigned int, std::vector<double>> viral_loads_reinfectin_chronic;
    std::unordered_map<unsigned int, std::vector<double>> viral_loads_reinfection_low_titer_clearance;
    std::unordered_map<unsigned int, std::vector<double>> viral_loads_reinfection_high_titer_clearance;

    // Treated individual viral loads
    std::unordered_map<unsigned int, std::vector<double>> viral_loads_treated;

    // Map of all viral load profile types and associated VL series
    std::unordered_map<VKProfile, std::unordered_map<unsigned int, std::vector<double>>> viral_load_profiles;

    ViralKinetics(const std::string& data_dir);

public:
    static ViralKinetics* instance();
    static void init(const std::string& data_dir);
    virtual ~ViralKinetics();

    double get_transmission_probability(double viral_load);

    double get_viral_load(VKProfile vk_profile, int profile_id, double viral_load_time);


}; // ViralKinetics class

}

#endif /* SRC_VIRALKINETICS_H_ */
