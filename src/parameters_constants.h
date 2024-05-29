/**
 * Parameter names used as keys in Parameters object
 */

#ifndef PARAMETER_CONSTANTS_H_
#define PARAMETER_CONSTANTS_H_

#include <string>

namespace hepcep {

const std::string DATA_DIR = "data.dir";
const std::string PWID_INPUT_FILE = "pwid.input.file";
const std::string PWID_NETWORK_EDGES_INPUT_FILE = "pwid.network.edges.input.file";
const std::string RUN = "run.number";
const std::string OUTPUT_DIRECTORY = "output.directory";
const std::string STATS_OUTPUT_FILE = "stats.output.file";
const std::string EVENTS_OUTPUT_FILE = "events.output.file";
const std::string PERSONS_OUTPUT_FILE = "persons.output.file";
const std::string NEEDLESHARING_OUTPUT_FILE = "needle_sharing.output.file";
const std::string EVENT_FILTERS = "log.events";

//const std::string NETWORK_FILE = "network.file";
const std::string ZONES_FILE = "zones.file";
const std::string ZONES_DISTANCE_FILE = "zones.distance.file";

const std::string VK_TRANSMIT_PROB_FILE = "viral.kinetics.transmit.prob.file";
const std::string VK_ACUTE_INFECT_CLEAR_FILE = "viral.kinetics.acute.infection.clearance.file";
const std::string VK_ACUTE_INFECT_INCOMP_FILE = "viral.kinetics.acute.infection.incomplete.file";
const std::string VK_ACUTE_INFECT_PERSIST_FILE = "viral.kinetics.acute.infection.persistence.file";
const std::string VK_REINFECT_CHRONIC_FILE = "viral.kinetics.reinfection.chronic.file";
const std::string VK_REINFECT_HIT_CLEAR_FILE = "viral.kinetics.reinfection.high.titer.clearance.file";
const std::string VK_REINFECT_LOWT_CLEAR_FILE = "viral.kinetics.reinfection.low.titer.clearance.file";
const std::string VK_TREATMENT_FILE = "viral.kinetics.treatment.file";

const std::string IMMUNOLOGY_TYPE = "immunology.type";

const std::string PWID_DATA_INPUT_TYPE = "pwid.data.input.type";

const std::string RESUME_FROM_SAVED = "resume.from.saved.net";
const std::string RESUME_FROM_SAVED_FILE = "resume.from.saved.net.file";

//const std::string LOG_INITIAL_NETWORK = "log.initial.network";
const std::string LOG_NETWORK_AT = "log.network.at";

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
const std::string TREATMENT_ENROLLMENT_STRATEGY = "treatment_enrollment_strategy";
const std::string TREATMENT_DURATION = "treatment_duration";
const std::string TREATMENT_REPEATABLE = "treatment_repeatable";
const std::string TREATMENT_SVR = "treatment_svr";
const std::string TREATMENT_SUSCEPTIBILITY = "treatment_susceptibility";
const std::string TREATMENT_NONADHERENCE = "treatment_nonadherence";
const std::string TREATMENT_ENROLLMENT_START_DELAY = "treatment_enrollment_start_delay";
const std::string TREATMENT_ENROLLMENT_PER_PY = "treatment_enrollment_per_PY";
const std::string REDUCED_TREATMENT_ENROLLMENT_PER_PY = "reduced_treatment_enrollment_per_PY";
const std::string HCV_SCREENING_INTERVAL = "hcv_screening_interval";
const std::string TREATMENT_ENROLLMENT_REDUCE_AT = "treatment_enrollment_reduce_at";
const std::string MAX_NUM_DAA_TREATMENTS = "max_num_daa_treatments";

const std::string TREATMENT_ENROLLMENT_PROBABILITY_UNBIASED = "treatment_enrollment_probability_unbiased";
const std::string TREATMENT_ENROLLMENT_PROBABILITY_HRP = "treatment_enrollment_probability_HRP";
const std::string TREATMENT_ENROLLMENT_PROBABILITY_FULLNETWORK = "treatment_enrollment_probability_fullnetwork";
const std::string TREATMENT_ENROLLMENT_PROBABILITY_INPARTNER = "treatment_enrollment_probability_inpartner";
const std::string TREATMENT_ENROLLMENT_PROBABILITY_OUTPARTNER = "treatment_enrollment_probability_outpartner";

const std::string OPIOID_TREATMENT_ENROLLMENT_START_DELAY = "opioid_treatment_enrollment_start_delay";
const std::string OPIOID_TREATMENT_ENROLLMENT_PER_PY  = "opioid_treatment_enrollment_per_PY";
const std::string OPIOID_TREATMENT_ENROLLMENT_PROBABILITY_METHADONE = "opioid_treatment_probability_methadone";
const std::string OPIOID_TREATMENT_ENROLLMENT_PROBABILITY_NALTREXONE = "opioid_treatment_probability_naltrexone";
const std::string OPIOID_TREATMENT_ENROLLMENT_PROBABILITY_BUPRENORPHINE = "opioid_treatment_probability_buprenorphine";

const std::string OT_URBAN_MAX_THRESHOLD = "opioid_treatment_urban_max_threshold";
const std::string OT_NON_URBAN_MAX_THRESHOLD = "opioid_treatment_non_urban_max_threshold";

const std::string METHADONE_MAX_INJECTION_INTENSITY = "methadone_max_injection_intensity";
const std::string METHADONE_DURATION = "methadone_duration";
const std::string METHADONE_URBAN_THRESHOLD = "methadone_urban_threshold";
const std::string METHADONE_NON_URBAN_THRESHOLD = "methadone_non_urban_threshold";
const std::string METHADONE_P_CLOSE = "methadone_p_close";
const std::string METHADONE_P_FAR = "methadone_p_far";

const std::string NALTREXONE_MAX_INJECTION_INTENSITY = "naltrexone_max_injection_intensity";
const std::string NALTREXONE_DURATION = "naltrexone_duration";
const std::string NALTREXONE_URBAN_THRESHOLD = "naltrexone_urban_threshold";
const std::string NALTREXONE_NON_URBAN_THRESHOLD = "naltrexone_non_urban_threshold";
const std::string NALTREXONE_P_CLOSE = "naltrexone_p_close";
const std::string NALTREXONE_P_FAR = "naltrexone_p_far";

const std::string BUPRENORPHINE_MAX_INJECTION_INTENSITY = "buprenorphine_max_injection_intensity";
const std::string BUPRENORPHINE_DURATION = "buprenorphine_duration";
const std::string BUPRENORPHINE_URBAN_THRESHOLD = "buprenorphine_urban_threshold";
const std::string BUPRENORPHINE_NON_URBAN_THRESHOLD = "buprenorphine_non_urban_threshold";
const std::string BUPRENORPHINE_P_CLOSE = "buprenorphine_p_close";
const std::string BUPRENORPHINE_P_FAR = "buprenorphine_p_far";

const std::string OPIOID_TREATMENT_ACCESS_SCENARIO = "opioid_treatment_access_scenario";
const std::string OPIOID_TREATMENT_ZONE_DISTANCE_FILE = "opioid_treatment_zone_distance_file";

}


#endif /* PARAMETER_CONSTANTS_H_ */
