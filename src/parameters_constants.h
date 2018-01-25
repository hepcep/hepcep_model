/**
 * Parameter names used as keys in Parameters object
 */

#ifndef PARAMETER_CONSTANTS_H_
#define PARAMETER_CONSTANTS_H_

#include <string>

namespace hepcep {

const std::string CNEP_PLUS_FILE = "cnep_plus.file";
const std::string CNEP_PLUS__EARLY_FILE = "cnep_plus_early.file";
const std::string RUN = "run.number";
const std::string OUTPUT_DIRECTORY = "output.directory";
const std::string STATS_OUTPUT_FILE = "stats.output.file";

//const std::string NETWORK_FILE = "network.file";
const std::string ZONES_FILE = "zones.file";
const std::string ZONES_DISTANCE_FILE = "zones.distance.file";

const std::string AB_PROB_CHRONIC = "ab_prob_chronic";
const std::string AB_PROB_ACUTE = "ab_prob_acute";

const std::string INITIAL_PWID_COUNT = "initial_pwid_count";

}


#endif /* PARAMETER_CONSTANTS_H_ */
