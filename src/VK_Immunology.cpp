/**
 * @file VK_Immunology.cpp
 *
 * Class for the viral kinetics immunology that includes in-host dynamic time series.
 *
 **/
#include <limits>
#include <memory>

#include "repast_hpc/Random.h"
#include "repast_hpc/RepastProcess.h"

#include "chi_sim/Parameters.h"

#include "Immunology.h"
#include "Statistics.h"
#include "LogType.h"
#include "HCPerson.h"
#include "EndTreatmentFunctor.h"
#include "parameters_constants.h"
#include "VK_Immunology.h"
#include "ViralKinetics.h"
#include "VKProfile.h"
#include "Distributions.h"

namespace hepcep {

// Schedule uses old boost::shared_ptrs so we use that here
// for scheduling events
using EventPtr = boost::shared_ptr<Event>;

// The intial VK profile types for any agent with an acute infection (defined in HCPersonData)
const std::vector<VKProfile> VK_Immunology::initial_acute_profiles({
    VKProfile::ACUTE_INFECTION_CLEARANCE, 
    VKProfile::ACUTE_INFECTION_INCOMPLETE, 
    VKProfile::ACUTE_INFECTION_PERSISTENCE, 
    VKProfile::REINFECT_LOW_CLEARANCE,
    VKProfile::REINFECT_HIGH_CLEARANCE, 
    VKProfile::REINFECT_CHRONIC });

// The intial VK profile types for any agent with a chronic infection (defined in HCPersonData)
const std::vector<VKProfile> VK_Immunology::initial_chronic_profiles({ 
    VKProfile::ACUTE_INFECTION_INCOMPLETE, 
    VKProfile::ACUTE_INFECTION_PERSISTENCE, 
    VKProfile::REINFECT_CHRONIC});

// The VK profile types for new infections (N1, N2, N3) for Naive PWID
const std::vector<VKProfile> VK_Immunology::new_infection_profiles({
    VKProfile::ACUTE_INFECTION_CLEARANCE, 
    VKProfile::ACUTE_INFECTION_INCOMPLETE, 
    VKProfile::ACUTE_INFECTION_PERSISTENCE});

// The VK profile types for new infections (R1, R2, R3) for previously cleared PWID
const std::vector<VKProfile> VK_Immunology::re_infection_profiles({
    VKProfile::REINFECT_LOW_CLEARANCE,
    VKProfile::REINFECT_HIGH_CLEARANCE, 
    VKProfile::REINFECT_CHRONIC });

VK_Immunology::VK_Immunology(HCPerson* idu) : Immunology(idu), 
    viral_load_time(0), vk_profile(VKProfile::NONE), vk_profile_id(0) {

    max_num_daa_treatments = chi_sim::Parameters::instance()->getDoubleParameter(MAX_NUM_DAA_TREATMENTS);
    treatment_repeatable = chi_sim::Parameters::instance()->getBooleanParameter(TREATMENT_REPEATABLE);
}

void VK_Immunology::deactivate(){
    
}

void VK_Immunology::step(){
    viral_load_time++;
}

void VK_Immunology::reset_viral_load_time(){
    viral_load_time = 0;
}

double VK_Immunology::get_viral_load(){
    double viral_load = ViralKinetics::instance() -> get_viral_load(vk_profile, vk_profile_id, viral_load_time);
    
    return viral_load;
}

bool VK_Immunology::exposePartner(std::shared_ptr<Immunology> partner_imm, double tick) {
    Statistics* stats = Statistics::instance();
    stats->logStatusChange(LogType::EXPOSED, partner_imm->idu_, "by agent " + std::to_string(idu_->id()));

    // TODO VK check the best way to determine....
    // If not currently infected, then no chance of infection
    // if (! isHcvRNA(tick)) {
    if (vk_profile == VKProfile::NONE){
        return false;
    }

    // Get from time series viral load probability of transmission
    double viral_load = get_viral_load();
    double transmissibility = ViralKinetics::instance() -> get_transmission_probability(viral_load);

    if (repast::Random::instance()->nextDouble() > transmissibility ) {
        stats->logStatusChange(LogType::EXPOSED, idu_, "transmission failed");
        return false;
    }
    
    bool established_new_infection = partner_imm->receiveInfectiousDose(tick); //receive_infectious_dose();
    if (established_new_infection) {
        partner_imm->idu_->setLastExposureDate(tick); //important: must follow Statistics.fire()
    }
    return established_new_infection;
}

void VK_Immunology::leaveExposed() {
    hcv_state = HCVState::INFECTIOUS_ACUTE;
    Statistics::instance()->logStatusChange(LogType::INFECTIOUS, idu_, "");
}

bool VK_Immunology::leaveAcute() {
    
    // TODO VK doesnt need this so we could remove from the Immunology base class
    
    return false;
}

void VK_Immunology::purgeActions() {
  
}

bool VK_Immunology::receiveInfectiousDose(double now) {

    // Cannot be re-infected if currently infected or in-treatment
    if (isHcvRNA(now) || isInTreatment()) {
        return false;
    }
    
    // New Infection (NI) only occur in naive (susceptible) PWID with the folliwing probability:
    //  - N1 (acute self-clearing):         20%
    //  - N2 (acute incomplete chronic):    40%
    //  - N3 (acute persist chronic):       40%

    // Re-infection (RI) can only occur in individuals who are acute cleared or treated with probability:
    //  - R1 (reinfection low titer)   : 12/27 = 0.444
    //  - R2 (reinfection high titer)  :  5/27 = 0.185
    //  - R3 (reinfection chronic)     : 10/27 = 0.371

    // TODO VK 

    // TODO VK check if "past recovered/cured" means just the last infection, or at any time in the past
    // NOTE: this long memory changes the behavior compared to original APK, even in the default (no treatment) case.
    past_recovered = past_recovered | (hcv_state == HCVState::RECOVERED);
    past_cured     = past_cured     | (hcv_state == HCVState::CURED);

    hcv_state = HCVState::EXPOSED;
    Statistics::instance()->logStatusChange(LogType::INFECTED, idu_, "");

    if (past_recovered || past_cured){
        // TODO VK Reinfection R1,R2,R3

        // Draw on options R1, R2, R3 from integer values 0 (44.4%), 1 (18.5%), 2 (37.1%)
        double probabilities[] = {0.444, 0.185, 0.371};
        DiscreteGen re_infection_gen(repast::Random::instance()->engine(), boost::random::discrete_distribution<>(probabilities));
        repast::DefaultNumberGenerator<DiscreteGen> gen(re_infection_gen);

        int draw = gen.next();  // int 0, 1, or 2
        vk_profile = re_infection_profiles[draw];
    }
    else{
        // Naive infection N1, N2, N3
        // Draw on options N1, N2, N3 from integer values 0 (20%), 1 (40%), 2 (40%)
        double probabilities[] = {0.2, 0.4, 0.4};
        DiscreteGen new_infection_gen(repast::Random::instance()->engine(), boost::random::discrete_distribution<>(probabilities));
        repast::DefaultNumberGenerator<DiscreteGen> gen(new_infection_gen);

        int draw = gen.next();  // int 0, 1, or 2
        vk_profile = new_infection_profiles[draw];
    }

    // Select a random VK time series from the vk_profile type
    // TODO There are 50 VK series in each profile typy, but we could treat this as a parameter for safety
    // NOTE that profiles are numbered 1 through 50 inclusive
    int num_profiles = 50;
    repast::IntUniformGenerator generator_2 = repast::Random::instance() -> createUniIntGenerator(1, num_profiles);

    vk_profile_id = (int)generator_2.next();
    viral_load_time = 0;

    idu_->setLastInfectionDate(now);

    // TODO VK after some time, the HCVState needs to be set to either HCVState::RECOVERED or HCVState::CHRONIC

    //  hcv_state = HCVState::RECOVERED;
    //  Statistics::instance()->logStatusChange(LogType::RECOVERED, idu_, "");

    //  hcv_state = HCVState::CHRONIC;
    //  Statistics::instance()->logStatusChange(LogType::CHRONIC, idu_, "");

    // TODO VK Testing!!!!!!!!! 
    if (vk_profile == VKProfile::ACUTE_INFECTION_INCOMPLETE ||
        vk_profile == VKProfile::ACUTE_INFECTION_PERSISTENCE ||
        vk_profile == VKProfile::REINFECT_CHRONIC){
            Statistics::instance()->logStatusChange(LogType::CHRONIC, idu_, "");
        }
    
    return true;
}


bool VK_Immunology::isHcvRNA(double now) {
    return (hcv_state == HCVState::EXPOSED ||
            hcv_state == HCVState::INFECTIOUS_ACUTE ||
            hcv_state == HCVState::CHRONIC) &&
            (!isInTreatmentViralSuppression(now));
}

bool VK_Immunology::isInTreatmentViralSuppression(double tick) {
    if (!in_treatment) {
        return false;
    }
    
    // TODO VK
    return false;

    // return (treatment_start_date + params_->mean_days_residual_hcv_infectivity) < tick;
}


// Similar to isTreatable() check, but logs an HCV test event which is typically
//   called when testing a person for treatment, but not in other cases, e.g. simple logging.
bool VK_Immunology::getTestedHCV(double now){
	 // Log that an HCV RNA test has occured.
	  Statistics::instance()->logStatusChange(LogType::HCVRNA_TEST, idu_, "");

	  return (isTreatable(now));
}

/*
 * for initializing agents
 * this function overrides the normal course of an infection
 * it should not be used for natural exposures
 *
 * censored_acute = in the middle of an acute infection
 */
void VK_Immunology::setHCVInitState(double now, HCVState state, int logging) {

    hcv_state = state;
    switch (state.value()) {
        case HCVState::Value::susceptible:
        {
            vk_profile = VKProfile::NONE;
            vk_profile_id = 0;

            if (logging > 0) {
                Statistics::instance()->logStatusChange(LogType::INFO, idu_, "new_hcv_state="+state.stringValue());
            }
            break;
        }

        case HCVState::Value::infectious_acute:
        {
            // Select from one of all six profile
            // This assumes we don't know any past history of recover/past infection
            repast::IntUniformGenerator generator = repast::Random::instance() -> createUniIntGenerator(0, initial_acute_profiles.size() - 1);
            
            int i = (int)generator.next();
            vk_profile = initial_acute_profiles[i];

            // Select a random VK time series from the vk_profile type
            // TODO There are 50 VK series in each profile typy, but we could treat this as a parameter for safety
            // NOTE that profiles are numbered 1 through 50 inclusive
            int num_profiles = 50;
            repast::IntUniformGenerator generator_2 = repast::Random::instance() -> createUniIntGenerator(1, num_profiles);

            vk_profile_id = (int)generator_2.next();

            // Select from one of the acute profiles and set time to 0
            // TODO set time to another value or randomize?
            viral_load_time = 0;
            
            if (logging > 0) {
                Statistics::instance()->logStatusChange(LogType::INFECTIOUS, idu_, "");

                string msg = vk_profile.stringValue() + ":" + std::to_string(vk_profile_id);
                Statistics::instance()->logStatusChange(LogType::VK_PROFILE, idu_, msg);
            }
            break;
        }

        case HCVState::Value::chronic:
        {
            // Select from one of the three chronic profiles
            repast::IntUniformGenerator generator = repast::Random::instance() -> createUniIntGenerator(0, initial_chronic_profiles.size() - 1);
            
            int i = (int)generator.next();
            vk_profile = initial_chronic_profiles[i];

            // TODO There are 50 VK series in each profile typy, but we could treat this as a parameter for safety
            // NOTE that profiles are numbered 1 through 50 inclusive
            int num_profiles = 50;
            repast::IntUniformGenerator generator_2 = repast::Random::instance() -> createUniIntGenerator(1, num_profiles);

            vk_profile_id = (int)generator_2.next();

            // TODO set time to another value or randomize?
            // TODO the google doc suggests setting to a time in the chronic phase

            // TODO make a parameter input
            viral_load_time = 14;  // Set to 14 days to make sure we get past the non-infectious acute phase

            if (logging > 0) {
                Statistics::instance()->logStatusChange(LogType::CHRONIC, idu_, "");

                string msg = vk_profile.stringValue() + ":" + std::to_string(vk_profile_id);
                Statistics::instance()->logStatusChange(LogType::VK_PROFILE, idu_, msg);
            }
            break;
        }
        case HCVState::Value::recovered:
        {
            // TODO VK nothing?  The recovered state will determine to select from a reinfection profile.

            if (logging > 0) {
                Statistics::instance()->logStatusChange(LogType::RECOVERED, idu_, "");
            }

            break;
        }
        default:
            throw std::invalid_argument("Unexpected state passed to setHCVInitState: " + state.stringValue());

    }
}

void VK_Immunology::leaveTreatment(bool treatment_succeeded) {
    in_treatment = false;
    if (treatment_succeeded) {
        hcv_state = HCVState::CURED;
        Statistics::instance()->logStatusChange(LogType::CURED, idu_, "");

        //std::cout << "Treatment success: " << idu_->id() << std::endl;
    }
    else {
        Statistics::instance()->logStatusChange(LogType::FAILED_TREATMENT, idu_, "");
        hcv_state = HCVState::CHRONIC; //even if entered as acute.  ignore the case where was about to self-limit
        treatment_failed = true;

//        std::cout << "Treatment failed: " << idu_->id() << std::endl;
    }
}

void VK_Immunology::startTreatment(bool adherent, double now) {

    // TODO VK select from a treatment VK profile



    //prevent any accidental switch to chronic during treatment
    // purgeActions(); //here - to purge any residual actions, such as switch to chronic
    // treatment_failed = false;
    // treatment_start_date = now;
    // in_treatment = true;
    // Statistics::instance()->logStatusChange(LogType::STARTED_TREATMENT, idu_, "");

    // repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
    // double treatment_end_time = now + repast::Random::instance()->createNormalGenerator(params_->treatment_duration, 1).next();
    
    // bool treatment_succeeds = (repast::Random::instance()->nextDouble() < params_->treatment_svr) && adherent;
    // EventPtr treatment_end_evt = boost::make_shared<Event>(treatment_end_time, EventFuncType::END_TREATMENT,
    //     new EndTreatmentFunctor(treatment_succeeds, this));
    // scheduled_actions.push_back(treatment_end_evt);
    // runner.scheduleEvent(treatment_end_time, treatment_end_evt);
    
    num_daa_treatments++;
}


VK_Immunology::~VK_Immunology() {
}


} /* namespace crx */
