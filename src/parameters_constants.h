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
const std::string EVENTS_OUTPUT_FILE = "events.output.file";

//const std::string NETWORK_FILE = "network.file";
const std::string ZONES_FILE = "zones.file";
const std::string ZONES_DISTANCE_FILE = "zones.distance.file";

const std::string LOG_INITIAL_NETWORK = "log.initial.network";

const std::string AB_PROB_CHRONIC = "ab_prob_chronic";
const std::string AB_PROB_ACUTE = "ab_prob_acute";
const std::string ATTRITION_RATE = "attrition_rate";

const std::string BURN_IN_DAYS = "burn_in_days";

const std::string HOMOPHILY_STRENGTH = "homophily_strength";
const std::string INITIAL_PWID_COUNT = "initial_pwid_count";
const std::string INTERACTION_HOME_CUTOFF = "interaction_home_cutoff";
const std::string INTERACTION_RATE_CONST = "interaction_rate_constant";
const std::string INTERACTION_RATE_DRUG_SITES = "interaction_rate_at_drug_sites";
const std::string INTERACTION_RATE_EXZONE = "interaction_rate_exzone";
const std::string LINKING_TIME_WINDOW = "linking_time_window";

const std::string MATURITY_THRESHOLD = "pwid_maturity_threshold";
const std::string MEAN_CAREER_DURATION = "mean_career_duration";
const std::string MEAN_EDGE_LIFETIME = "mean_tie_lifetime";
const std::string MEAN_DAYS_ACUTE_NAIVE = "mean_days_acute_naive";
const std::string MEAN_DAYS_ACUTE_RECHALLENGED = "mean_days_acute_rechallenged";
const std::string MEAN_DAYS_NAIVE_TO_INFECTIOUS = "mean_days_naive_to_infectious";
const std::string MEAN_DAYS_RESIDUAL_HCV_INFECTIVITY = "mean_days_residual_hcv_infectivity";
const std::string NET_INFLOW = "net_inflow";
const std::string PROB_INFECTED_NEW_ARRIVING = "prob_infected_when_arrive";
const std::string PROB_SELF_LIMITING_FEMALE = "prob_self_limiting_female";
const std::string PROB_SELF_LIMITING_MALE = "prob_self_limiting_male";
const std::string PROB_CLEARING = "prob_clearing";
const std::string PROB_CESSATION = "prob_cessation";

const std::string STATUS_REPORT_FREQUENCY = "status_report_frequency";

const std::string TRANSMISSIBILITY = "transmissibility";
const std::string TREATMENT_DURATION = "treatment_duration";
const std::string TREATMENT_REPEATABLE = "treatment_repeatable";
const std::string TREATMENT_SVR = "treatment_svr";
const std::string TREATMENT_SUSCEPTIBILITY = "treatment_susceptibility";
const std::string TREATMENT_NONADHERENCE = "treatment_nonadherence";
const std::string TREATMENT_ENROLLMENT_START_DELAY = "treatment_enrollment_start_delay";
const std::string TREATMENT_ENROLLMENT_PER_PY = "treatment_enrollment_per_PY";

const std::string TREATMENT_ENROLLMENT_PROBABILITY_UNBIASED = "treatment_enrollment_probability_unbiased";
const std::string TREATMENT_ENROLLMENT_PROBABILITY_HRP = "treatment_enrollment_probability_HRP";
const std::string TREATMENT_ENROLLMENT_PROBABILITY_FULLNETWORK = "treatment_enrollment_probability_fullnetwork";
const std::string TREATMENT_ENROLLMENT_PROBABILITY_INPARTNER = "treatment_enrollment_probability_inpartner";
const std::string TREATMENT_ENROLLMENT_PROBABILITY_OUTPARTNER = "treatment_enrollment_probability_outpartner";

}


#endif /* PARAMETER_CONSTANTS_H_ */
