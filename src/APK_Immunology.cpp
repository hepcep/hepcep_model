/**
 * @file APK_Immunology.cpp
 *
 * Immunology class using the original APK infection mechanics.
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

namespace hepcep {

// these were private static in APK Immunology.java
const double CONTACT_RISK = 1.0;
const double ACUTE_BOOST = 1.0;

// Schedule uses old boost::shared_ptrs so we use that here
// for scheduling events
using EventPtr = boost::shared_ptr<Event>;

ImmunologyParameters::ImmunologyParameters() :
        mean_days_acute_naive { std::numeric_limits<double>::signaling_NaN() },
				mean_days_acute_rechallenged {std::numeric_limits<double>::signaling_NaN() },
				mean_days_naive_to_infectious { std::numeric_limits<double>::signaling_NaN() },
				mean_days_residual_hcv_infectivity { std::numeric_limits<double>::signaling_NaN() },
				prob_self_limiting_female { std::numeric_limits<double>::signaling_NaN() },
				prob_self_limiting_male { std::numeric_limits<double>::signaling_NaN() },
				prob_clearing { std::numeric_limits<double>::signaling_NaN() },
				transmissibility { std::numeric_limits<double>::signaling_NaN() },
				treatment_duration { std::numeric_limits<double>::signaling_NaN() },
				treatment_svr { std::numeric_limits<double>::signaling_NaN() },
				treatment_susceptibility { std::numeric_limits<double>::signaling_NaN() }
{}

APK_Immunology::APK_Immunology(HCPerson* idu) : Immunology(idu), 
    scheduled_actions() {

	params_ = std::make_shared<ImmunologyParameters>();

	params_->mean_days_acute_naive = chi_sim::Parameters::instance()->getDoubleParameter(MEAN_DAYS_ACUTE_NAIVE);
	params_->mean_days_acute_rechallenged  = chi_sim::Parameters::instance()->getDoubleParameter(MEAN_DAYS_ACUTE_RECHALLENGED);
	params_->mean_days_naive_to_infectious = chi_sim::Parameters::instance()->getDoubleParameter(MEAN_DAYS_NAIVE_TO_INFECTIOUS);
	params_->mean_days_residual_hcv_infectivity = chi_sim::Parameters::instance()->getDoubleParameter(MEAN_DAYS_RESIDUAL_HCV_INFECTIVITY);
	params_->prob_self_limiting_female = chi_sim::Parameters::instance()->getDoubleParameter(PROB_SELF_LIMITING_FEMALE);
	params_->prob_self_limiting_male = chi_sim::Parameters::instance()->getDoubleParameter(PROB_SELF_LIMITING_MALE);
	params_->prob_clearing = chi_sim::Parameters::instance()->getDoubleParameter(PROB_CLEARING);
	params_->transmissibility = chi_sim::Parameters::instance()->getDoubleParameter(TRANSMISSIBILITY);
	params_->treatment_duration = chi_sim::Parameters::instance()->getDoubleParameter(TREATMENT_DURATION);	
	params_->treatment_svr = chi_sim::Parameters::instance()->getDoubleParameter(TREATMENT_SVR);
	params_->treatment_susceptibility = chi_sim::Parameters::instance()->getDoubleParameter(TREATMENT_SUSCEPTIBILITY);

    max_num_daa_treatments = chi_sim::Parameters::instance()->getDoubleParameter(MAX_NUM_DAA_TREATMENTS);
    treatment_repeatable = chi_sim::Parameters::instance()->getBooleanParameter(TREATMENT_REPEATABLE);
}

// TODO These other constructors were used in serialization, but need to be updated 
//      to work with the new Immunology interface

// APK_Immunology::APK_Immunology(HCPerson* idu, IPPtr params) : Immunology(idu),
//     params_(params), 
//     // hcv_state(HCVState::SUSCEPTIBLE),
//     scheduled_actions() {

//     max_num_daa_treatments =  params_->max_num_daa_treatments;
//     treatment_repeatable = params_->treatment_repeatable;
// }

// APK_Immunology::APK_Immunology(HCPerson* idu, HCVState alter_state, IPPtr params) : Immunology(idu),
//     params_(params),
//     // hcv_state(alter_state), 
//     scheduled_actions() {
    
//     max_num_daa_treatments =  params_->max_num_daa_treatments;
//     treatment_repeatable = params_->treatment_repeatable;
// }

void APK_Immunology::deactivate(){
    purgeActions();
}

void APK_Immunology::step(){
    // APK Immunology does not have a step behavior
}

bool APK_Immunology::exposePartner(std::shared_ptr<Immunology> partner_imm, double tick) {
    Statistics* stats = Statistics::instance();
    stats->logStatusChange(LogType::EXPOSED, partner_imm->idu_, "by agent " + std::to_string(idu_->id()));

    if (! isHcvRNA(tick)) {
        return false;
    }

    if (isAcute()) {
        if (repast::Random::instance()->nextDouble() > CONTACT_RISK * params_->transmissibility * ACUTE_BOOST) {
            stats->logStatusChange(LogType::EXPOSED, idu_, "transmission failed");
            return false;
        }
    } else {
        if (repast::Random::instance()->nextDouble() > CONTACT_RISK * params_->transmissibility) {
            stats->logStatusChange(LogType::EXPOSED, idu_, "transmission failed");
            return false;
        }
    }

    bool established_new_infection = partner_imm->receiveInfectiousDose(tick); //receive_infectious_dose();
    if (established_new_infection) {
        partner_imm->idu_->setLastExposureDate(tick); //important: must follow Statistics.fire()
    }
    return established_new_infection;
}

void APK_Immunology::leaveExposed() {
    hcv_state = HCVState::INFECTIOUS_ACUTE;
    Statistics::instance()->logStatusChange(LogType::INFECTIOUS, idu_, "");
}

bool APK_Immunology::leaveAcute() {

    //returns true iff self limiting
    double prob_self_limiting = idu_->getGender() == Gender::MALE ? params_->prob_self_limiting_male : params_->prob_self_limiting_female;
            //agent.getGender() == Gender.Male? prob_self_limiting_male : prob_self_limiting_female;
    if (((!past_recovered) && prob_self_limiting > repast::Random::instance()->nextDouble()) ||
         ((past_recovered) && params_->prob_clearing > repast::Random::instance()->nextDouble()) ||
             ((past_cured) && params_->treatment_susceptibility < repast::Random::instance()->nextDouble()))    {
        hcv_state = HCVState::RECOVERED;
        Statistics::instance()->logStatusChange(LogType::RECOVERED, idu_, "");
        return true;
    } else {
        hcv_state = HCVState::CHRONIC;
        Statistics::instance()->logStatusChange(LogType::CHRONIC, idu_, "");
        return false;
    }
}

void APK_Immunology::purgeActions() {
    for (auto evt : scheduled_actions) {
        evt->cancel();
    }
    scheduled_actions.clear();
    in_treatment = false;
}

bool APK_Immunology::receiveInfectiousDose(double now) {
    if (isHcvRNA(now) || isInTreatment()) {
        return false;
    }
    purgeActions();

    //note: this long memory changes the behavior compared to original APK, even in the default (no treatment) case.
    past_recovered = past_recovered | (hcv_state == HCVState::RECOVERED);
    past_cured     = past_cured     | (hcv_state == HCVState::CURED);

    hcv_state = HCVState::EXPOSED;
    Statistics::instance()->logStatusChange(LogType::INFECTED, idu_, "");

    idu_->setLastInfectionDate(now);

    repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
    double exposed_end_time = now + repast::Random::instance()->createExponentialGenerator(1.0 / params_->mean_days_naive_to_infectious).next();

    EventPtr leave_exposed_evt = boost::make_shared<Event>(exposed_end_time, EventFuncType::LEAVE_EXPOSED,
        new MethodFunctor<APK_Immunology, void>(this, &APK_Immunology::leaveExposed));
    scheduled_actions.push_back(leave_exposed_evt);
    runner.scheduleEvent(exposed_end_time, leave_exposed_evt);

    double acute_end_time;
    if (! past_recovered) {
        acute_end_time =  now + repast::Random::instance()->createExponentialGenerator(1.0 / params_->mean_days_acute_naive).next();

                //time_now + RandomHelper.createExponential(1./mean_days_acute_naive).nextDouble();
    } else {
        acute_end_time =  now + repast::Random::instance()->createExponentialGenerator(1.0 / params_->mean_days_acute_rechallenged).next();
    }

    EventPtr leave_acute_evt = boost::make_shared<Event>(acute_end_time, EventFuncType::LEAVE_ACUTE,
        new MethodFunctor<APK_Immunology, bool>(this, &APK_Immunology::leaveAcute));
    scheduled_actions.push_back(leave_acute_evt);
    runner.scheduleEvent(acute_end_time, leave_acute_evt);

    return true;
}


bool APK_Immunology::isHcvRNA(double now) {
    return (hcv_state == HCVState::EXPOSED ||
            hcv_state == HCVState::INFECTIOUS_ACUTE ||
            hcv_state == HCVState::CHRONIC) &&
            (!isInTreatmentViralSuppression(now));
}

// bool APK_Immunology::isInfectious(double now) {
//     return (hcv_state == HCVState::INFECTIOUS_ACUTE ||
//             hcv_state == HCVState::CHRONIC)
//             && (!isInTreatmentViralSuppression(now));
// }

bool APK_Immunology::isInTreatmentViralSuppression(double tick) {
    if (!in_treatment) {
        return false;
    }
    return (treatment_start_date + params_->mean_days_residual_hcv_infectivity) < tick;
}


// Similar to isTreatable() check, but logs an HCV test event which is typically
//   called when testing a person for treatment, but not in other cases, e.g. simple logging.
bool APK_Immunology::getTestedHCV(double now){
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
void APK_Immunology::setHCVInitState(double now, HCVState state, int logging) {

	// TODO check purge actions.  APK doesn't do this but the initial HCV state
	//      is called twice for new arriving Persons so it may add additional
	//      schedule actions that are not valid after the second call.
	purgeActions();

    hcv_state = state;
    switch (state.value()) {
        case HCVState::Value::susceptible:
        {
            hcv_state = HCVState::SUSCEPTIBLE;
            if (logging > 0) {
                Statistics::instance()->logStatusChange(LogType::INFO, idu_, "new_hcv_state="+state.stringValue());
            }
            break;
        }

        case HCVState::Value::infectious_acute:
        {
            double acute_end_time = now + repast::Random::instance()->createExponentialGenerator(1.0 / params_->mean_days_acute_naive).next();

            repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
            EventPtr leave_acute_evt = boost::make_shared<Event>(acute_end_time, EventFuncType::LEAVE_ACUTE,
                new MethodFunctor<APK_Immunology, bool>(this, &APK_Immunology::leaveAcute));
            scheduled_actions.push_back(leave_acute_evt);
            runner.scheduleEvent(acute_end_time, leave_acute_evt);
            if (logging > 0) {
                Statistics::instance()->logStatusChange(LogType::INFECTIOUS, idu_, "");
                idu_->setLastExposureDate(now);
            }
            break;
        }

        case HCVState::Value::chronic:
        {
            idu_->setLastExposureDate(now);
            if (logging > 0) {
                Statistics::instance()->logStatusChange(LogType::CHRONIC, idu_, "");
                idu_->setLastExposureDate(now); //must follow fire, b/c it uses the signature
            }
            break;
        }
        case HCVState::Value::recovered:
        {
            idu_->setLastExposureDate(now);
            if (logging > 0) {
                Statistics::instance()->logStatusChange(LogType::RECOVERED, idu_, "");
                idu_->setLastExposureDate(now); //must follow fire, b/c it uses the signature
            }

            break;
        }
        default:
            throw std::invalid_argument("Unexpected state passed to setHCVInitState: " + state.stringValue());

    }
}

void APK_Immunology::leaveTreatment(bool treatment_succeeded) {
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

void APK_Immunology::startTreatment(bool adherent, double now) {
//    if(! isTreatable(now)) {
//        std::cout << "Agent cannot be treated [doubly recruited?] ..." << (*idu_) << std::endl;
//        return;
//    }
    //prevent any accidental switch to chronic during treatment
    purgeActions(); //here - to purge any residual actions, such as switch to chronic
    treatment_failed = false;
    treatment_start_date = now;
    in_treatment = true;
    Statistics::instance()->logStatusChange(LogType::STARTED_TREATMENT, idu_, "");

    repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
    double treatment_end_time = now + repast::Random::instance()->createNormalGenerator(params_->treatment_duration, 1).next();
    
    bool treatment_succeeds = (repast::Random::instance()->nextDouble() < params_->treatment_svr) && adherent;
    EventPtr treatment_end_evt = boost::make_shared<Event>(treatment_end_time, EventFuncType::END_TREATMENT,
        new EndTreatmentFunctor(treatment_succeeds, this));
    scheduled_actions.push_back(treatment_end_evt);
    runner.scheduleEvent(treatment_end_time, treatment_end_evt);
    
    num_daa_treatments++;
}


APK_Immunology::~APK_Immunology() {
}


} /* namespace crx */
