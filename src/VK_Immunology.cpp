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
    viral_load_time(0), vk_profile(VKProfile::NONE), 
    vk_profile_id(0) {

    max_num_daa_treatments = chi_sim::Parameters::instance()->getDoubleParameter(MAX_NUM_DAA_TREATMENTS);
    treatment_repeatable = chi_sim::Parameters::instance()->getBooleanParameter(TREATMENT_REPEATABLE);

    treatment_svr = chi_sim::Parameters::instance()->getDoubleParameter(TREATMENT_SVR);
    treatment_duration = chi_sim::Parameters::instance()->getDoubleParameter(TREATMENT_DURATION);

    mean_days_acute_naive = chi_sim::Parameters::instance()->getDoubleParameter(MEAN_DAYS_ACUTE_NAIVE);
}

void VK_Immunology::deactivate(){
    purgeActions();
}

void VK_Immunology::step(){
    viral_load_time++;

    // Check acute clearance profiles VL level and reset to recovered
    // when log VL <= 0.  This signals end of infection and reset to 
    // recovered when re-infection can occur.

    // TODO It probably makes sense to just check if person is acute
    //      because VL proviles acute incompleted/chronic are just set to
    //      HCVState::CHRONIC at time of infection.
    if (hcv_state == HCVState::INFECTIOUS_ACUTE){

    // if (vk_profile == VKProfile::ACUTE_INFECTION_CLEARANCE ||
    //     vk_profile == VKProfile::REINFECT_HIGH_CLEARANCE ||
    //     vk_profile == VKProfile::REINFECT_LOW_CLEARANCE){

        double viral_load = ViralKinetics::instance() -> get_viral_load(vk_profile, vk_profile_id, viral_load_time);

        if (viral_load <= 0){
            hcv_state = HCVState::RECOVERED;
            Statistics::instance()->logStatusChange(LogType::RECOVERED, idu_, "");
        }
    }


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
    // hcv_state = HCVState::INFECTIOUS_ACUTE;
    // Statistics::instance()->logStatusChange(LogType::INFECTIOUS, idu_, "");
}

bool VK_Immunology::leaveAcute() {
    
    // TODO VK doesnt need this so we could remove from the Immunology base class
    
    // hcv_state = HCVState::RECOVERED;
    // Statistics::instance()->logStatusChange(LogType::RECOVERED, idu_, "");
    // return true;

    return false;
}

void VK_Immunology::purgeActions() {
   for (auto evt : scheduled_actions) {
        evt->cancel();
    }
    scheduled_actions.clear();
    in_treatment = false;
}

bool VK_Immunology::receiveInfectiousDose(double now) {

    // Cannot be re-infected if currently infected or in-treatment
    if (isHcvRNA(now) || isInTreatment()) {
        return false;
    }

    purgeActions();
    
    // New Infection (NI) only occur in naive (susceptible) PWID with the folliwing probability:
    //  - N1 (acute self-clearing):         20%
    //  - N2 (acute incomplete chronic):    40%
    //  - N3 (acute persist chronic):       40%

    // Re-infection (RI) can only occur in individuals who are acute cleared or treated with probability:
    //  - R1 (reinfection low titer)   : 12/27 = 0.444
    //  - R2 (reinfection high titer)  :  5/27 = 0.185
    //  - R3 (reinfection chronic)     : 10/27 = 0.371

    Statistics::instance()->logStatusChange(LogType::INFECTED, idu_, "");

    // NOTE VK this state check is different from APK since it only considers if the
    //      current state is recovered and not if any recovered occured in the past.
    if (hcv_state == HCVState::RECOVERED){
        // TODO put in model props
        // Reinfection R1,R2,R3
        // Draw on options R1, R2, R3 from integer values 0 (44.4%), 1 (18.5%), 2 (37.1%)
        double probabilities[] = {0.444, 0.185, 0.371};
        DiscreteGen re_infection_gen(repast::Random::instance()->engine(), boost::random::discrete_distribution<>(probabilities));
        repast::DefaultNumberGenerator<DiscreteGen> gen(re_infection_gen);

        int draw = gen.next();  // int 0, 1, or 2
        vk_profile = re_infection_profiles[draw];
    }
    else{
        // TODO put in model props
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

    // If the infection is chronic type VK profile 
    if (vk_profile == VKProfile::ACUTE_INFECTION_INCOMPLETE ||
        vk_profile == VKProfile::ACUTE_INFECTION_PERSISTENCE ||
        vk_profile == VKProfile::REINFECT_CHRONIC){

            // TODO VK Can we immediately set to CHRONIC, or first set to acute and then chronic
            hcv_state = HCVState::CHRONIC;
            Statistics::instance()->logStatusChange(LogType::CHRONIC, idu_, "");
    }
    else{ 
        // Schedule the end of the acute infectious time
        // double acute_end_time;

        hcv_state == HCVState::INFECTIOUS_ACUTE;

        // NOTE: VK step() checks the acute profile VL level and resets to recovered when VL <= 0.

        // // TODO VK Should be based on ? criteria
        // acute_end_time =  now + repast::Random::instance()->createExponentialGenerator(1.0 / mean_days_acute_naive).next();
        
        // repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
        // EventPtr leave_acute_evt = boost::make_shared<Event>(acute_end_time, EventFuncType::LEAVE_ACUTE,
        // new MethodFunctor<VK_Immunology, bool>(this, &VK_Immunology::leaveAcute));
        // scheduled_actions.push_back(leave_acute_evt);
        // runner.scheduleEvent(acute_end_time, leave_acute_evt);
    }
    
    return true;
}


bool VK_Immunology::isHcvRNA(double now) {

    // TODO VK Should check against viral load level and not state value

    return (hcv_state == HCVState::EXPOSED ||
            hcv_state == HCVState::INFECTIOUS_ACUTE ||
            hcv_state == HCVState::CHRONIC) 
            &&
            (!isInTreatmentViralSuppression(now));
}

double VK_Immunology::get_transmissibility(){
    double viral_load = get_viral_load();
    double transmissibility = ViralKinetics::instance() -> get_transmission_probability(viral_load);

    return transmissibility;
}

bool VK_Immunology::isInTreatmentViralSuppression(double tick) {
    if (!in_treatment) {
        return false;
    }
    return true;
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

    // TODO check purge actions.  APK doesn't do this but the initial HCV state
	//      is called twice for new arriving Persons so it may add additional
	//      schedule actions that are not valid after the second call.
	purgeActions();

    switch (state.value()) {
        case HCVState::Value::susceptible:
        {
            hcv_state = HCVState::SUSCEPTIBLE;
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

            // Set HCV state to acute or chronic depending on what type of VK profile is selected.
            // NOTE this differs from APK becuase VK does not track acute->chronic state transitions.
            if (vk_profile == VKProfile::ACUTE_INFECTION_CLEARANCE ||
                vk_profile == VKProfile::REINFECT_HIGH_CLEARANCE ||
                vk_profile == VKProfile::REINFECT_LOW_CLEARANCE){

                    hcv_state = HCVState::INFECTIOUS_ACUTE;

                    if (logging > 0) {
                        Statistics::instance()->logStatusChange(LogType::INFECTIOUS, idu_, "");
                    }
            }
            else{
                hcv_state = HCVState::CHRONIC;

                if (logging > 0) {
                    Statistics::instance()->logStatusChange(LogType::CHRONIC, idu_, "");
                }
            }

            // Select a random VK time series from the vk_profile type
            // TODO There are 50 VK series in each profile typy, but we could treat this as a parameter for safety
            // NOTE that profiles are numbered 1 through 50 inclusive
            int num_profiles = 50;
            repast::IntUniformGenerator generator_2 = repast::Random::instance() -> createUniIntGenerator(1, num_profiles);

            vk_profile_id = (int)generator_2.next();

            // Select from one of the acute profiles and set time to 0
            // TODO set time to another value or randomize?
            // TODO we know that each viral load profile is 800 time steps, but it would be better to 
            //      get the array size instead of using it hard coded here.
            int max_viral_load_time = 800;
            repast::IntUniformGenerator generator_time = repast::Random::instance() -> createUniIntGenerator(0, max_viral_load_time - 1);
            viral_load_time = (int)generator_time.next();
            
            if (logging > 0) {
                string msg = vk_profile.stringValue() + ":" + std::to_string(vk_profile_id);
                Statistics::instance()->logStatusChange(LogType::VK_PROFILE, idu_, msg);
            }
            break;
        }

        case HCVState::Value::chronic:
        {
            hcv_state = HCVState::CHRONIC;

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

            // TODO make a parameter input
            // viral_load_time = 14;  // Set to 14 days to make sure we get past the non-infectious acute phase
            
            // TODO we know that each viral load profile is 800 time steps, but it would be better to 
            //      get the array size instead of using it hard coded here.
            int max_viral_load_time = 800;
            repast::IntUniformGenerator generator_time = repast::Random::instance() -> createUniIntGenerator(0, max_viral_load_time - 1);
            viral_load_time = (int)generator_time.next();

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
        // NOTE: VK CURED and SUSCEPTIBLE have the same effect on re-infection dynamics
        hcv_state = HCVState::CURED;
        Statistics::instance()->logStatusChange(LogType::CURED, idu_, "");

        // hcv_state = HCVState::SUSCEPTIBLE;
        vk_profile = VKProfile::NONE;
        vk_profile_id = 0;
        viral_load_time = 0;
    }
    else {
        Statistics::instance()->logStatusChange(LogType::FAILED_TREATMENT, idu_, "");
        hcv_state = HCVState::CHRONIC; //even if entered as acute.  ignore the case where was about to self-limit
        treatment_failed = true;

        // Set VK profile to CHRONIC
        // Select from one of the three chronic profiles
        repast::IntUniformGenerator generator = repast::Random::instance() -> createUniIntGenerator(0, initial_chronic_profiles.size() - 1);
        
        int i = (int)generator.next();
        vk_profile = initial_chronic_profiles[i];

        // TODO There are 50 VK series in each profile typy, but we could treat this as a parameter for safety
        // NOTE that profiles are numbered 1 through 50 inclusive
        int num_profiles = 50;
        repast::IntUniformGenerator generator_2 = repast::Random::instance() -> createUniIntGenerator(1, num_profiles);

        vk_profile_id = (int)generator_2.next();
        viral_load_time = 0;

    }
}

void VK_Immunology::startTreatment(bool adherent, double now) {

    //prevent any accidental switch to chronic during treatment
    purgeActions(); //here - to purge any residual actions, such as switch to chronic

    // select from a treatment VK profile

    vk_profile = VKProfile::TREATMENT;
    viral_load_time = 0;

    // Select a random VK time series from the vk_profile type
    // TODO There are 276 VK series in each profile typy, but we could treat this as a parameter for safety
    // NOTE that profiles are numbered 0 through 275 inclusive
    int num_profiles = 275;
    repast::IntUniformGenerator generator_2 = repast::Random::instance() -> createUniIntGenerator(0, num_profiles);

    vk_profile_id = (int)generator_2.next();

    // TODO VK what is a failed treatment?
    // TODO VK treatment time duration?    
    // treatment_failed = false;
    treatment_start_date = now;
    in_treatment = true;
    Statistics::instance()->logStatusChange(LogType::STARTED_TREATMENT, idu_, "");

    // TODO VK determine how the end the treatment period
    repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
    double treatment_end_time = now + repast::Random::instance()->createNormalGenerator(treatment_duration, 1).next();
    
    bool treatment_succeeds = (repast::Random::instance()->nextDouble() < treatment_svr) && adherent;
    EventPtr treatment_end_evt = boost::make_shared<Event>(treatment_end_time, EventFuncType::END_TREATMENT,
        new EndTreatmentFunctor(treatment_succeeds, this));
    scheduled_actions.push_back(treatment_end_evt);
    runner.scheduleEvent(treatment_end_time, treatment_end_evt);
    
    num_daa_treatments++;
}


VK_Immunology::~VK_Immunology() {
}


} /* namespace crx */
