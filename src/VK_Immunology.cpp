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
#include "ViralKinetics.h"
#include "VKProfile.h"

namespace hepcep {

// Schedule uses old boost::shared_ptrs so we use that here
// for scheduling events
using EventPtr = boost::shared_ptr<Event>;

// The intial VK profile types for any agent with an acute infection (defined in HCPersonData)
const std::vector<VKProfile> initial_acute_profiles({ 
    VKProfile::ACUTE_INFECTION_CLEARANCE, 
    VKProfile::ACUTE_INFECTION_INCOMPLETE, 
    VKProfile::ACUTE_INFECTION_PERSISTENCE, 
    VKProfile::REINFECT_HIGH_CLEARANCE, 
    VKProfile::REINFECT_LOW_CLEARANCE, 
    VKProfile::REINFECT_CHRONIC});

// The intial VK profile types for any agent with a chronic infection (defined in HCPersonData)
const std::vector<VKProfile> initial_chronic_profiles({ 
    VKProfile::ACUTE_INFECTION_INCOMPLETE, 
    VKProfile::ACUTE_INFECTION_PERSISTENCE, 
    VKProfile::REINFECT_CHRONIC});

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
    double viral_load = 0;

    return viral_load;
}

bool VK_Immunology::exposePartner(std::shared_ptr<Immunology> partner_imm, double tick) {
    Statistics* stats = Statistics::instance();
    stats->logStatusChange(LogType::EXPOSED, partner_imm->idu_, "by agent " + std::to_string(idu_->id()));

    if (! isHcvRNA(tick)) {
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

    // TODO VK
    return false;

    // //returns true iff self limiting
    // double prob_self_limiting = idu_->getGender() == Gender::MALE ? params_->prob_self_limiting_male : params_->prob_self_limiting_female;
    //         //agent.getGender() == Gender.Male? prob_self_limiting_male : prob_self_limiting_female;
    
    // if (((!past_recovered) && prob_self_limiting > repast::Random::instance()->nextDouble()) ||
    //      ((past_recovered) && params_->prob_clearing > repast::Random::instance()->nextDouble()) ||
    //          ((past_cured) && params_->treatment_susceptibility < repast::Random::instance()->nextDouble()))    {
        
    //     hcv_state = HCVState::RECOVERED;
    //     Statistics::instance()->logStatusChange(LogType::RECOVERED, idu_, "");
    //     return true;
    // } else {
    //     hcv_state = HCVState::CHRONIC;
    //     Statistics::instance()->logStatusChange(LogType::CHRONIC, idu_, "");
    //     return false;
    // }
}

void VK_Immunology::purgeActions() {
  
}

bool VK_Immunology::receiveInfectiousDose(double now) {
    if (isHcvRNA(now) || isInTreatment()) {
        return false;
    }
    
    // TODO VK if not past_cured/recovered, then it is a new infection, so select from
    // one of the three acute-infection profiles, else if its a re-infection, select
    // from one of the re-infection profiles.

    //note: this long memory changes the behavior compared to original APK, even in the default (no treatment) case.
    past_recovered = past_recovered | (hcv_state == HCVState::RECOVERED);
    past_cured     = past_cured     | (hcv_state == HCVState::CURED);

    hcv_state = HCVState::EXPOSED;
    Statistics::instance()->logStatusChange(LogType::INFECTED, idu_, "");

    idu_->setLastInfectionDate(now);

    // repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
    // double exposed_end_time = now + repast::Random::instance()->createExponentialGenerator(1.0 / params_->mean_days_naive_to_infectious).next();

    // EventPtr leave_exposed_evt = boost::make_shared<Event>(exposed_end_time, EventFuncType::LEAVE_EXPOSED,
    //     new MethodFunctor<APK_Immunology, void>(this, &APK_Immunology::leaveExposed));
    // scheduled_actions.push_back(leave_exposed_evt);
    // runner.scheduleEvent(exposed_end_time, leave_exposed_evt);

    // double acute_end_time;
    // if (! past_recovered) {
    //     acute_end_time =  now + repast::Random::instance()->createExponentialGenerator(1.0 / params_->mean_days_acute_naive).next();

    //             //time_now + RandomHelper.createExponential(1./mean_days_acute_naive).nextDouble();
    // } else {
    //     acute_end_time =  now + repast::Random::instance()->createExponentialGenerator(1.0 / params_->mean_days_acute_rechallenged).next();
    // }

    // EventPtr leave_acute_evt = boost::make_shared<Event>(acute_end_time, EventFuncType::LEAVE_ACUTE,
    //     new MethodFunctor<APK_Immunology, bool>(this, &APK_Immunology::leaveAcute));
    // scheduled_actions.push_back(leave_acute_evt);
    // runner.scheduleEvent(acute_end_time, leave_acute_evt);

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
            // TODO VK select from one of all six profile and set the time to 0ish
            // This assumes we don't know any past history of recover/past infection

            // Select from one of the acute profiles and set time to 0
            // TODO set time to another value or randomize?
            
            if (logging > 0) {
                Statistics::instance()->logStatusChange(LogType::INFECTIOUS, idu_, "");
            }
            break;
        }


        case HCVState::Value::chronic:
        {
             // TODO VK select from one of the three chronic profiles and set the time to 0ish

            // Select from one of the chronic profiles and set time to 0
            // TODO set time to another value or randomize?
            // TODO the google doc suggests setting to a time in the chronic phase

            if (logging > 0) {
                Statistics::instance()->logStatusChange(LogType::CHRONIC, idu_, "");
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

    // TODO VK

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