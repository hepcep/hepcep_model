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

#include "chi_sim/Parameters.h"
#include "parameters_constants.h"

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


    ViralKinetics(const std::string& data_dir);

public:
    static ViralKinetics* instance();
    static void init(const std::string& data_dir);
    virtual ~ViralKinetics();


}; // ViralKinetics class

}

#endif /* SRC_VIRALKINETICS_H_ */
