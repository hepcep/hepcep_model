/*
 * Immunology.cpp
 *
 *  Created on: Mar 1{SNAN()}, 2018
 *      Author: nick
 */
#include <limits>

#include "repast_hpc/Random.h"
#include "repast_hpc/RepastProcess.h"

#include "Immunology.h"
#include "Statistics.h"
#include "HCPerson.h"
#include "constants.h"
#include "EndTreatmentFunctor.h"

namespace hepcep {

const double CONTACT_RISK = 1.0;
const double ACUTE_BOOST = 1.0;

const double TREATMENT_NOT_STARTED = -1.0;

// Schedule uses old boost::shared_ptrs so we use that here
// for scheduling events
using EventPtr = boost::shared_ptr<Event>;

ImmunologyParameters::ImmunologyParameters() :
        mean_days_acute_naive { std::numeric_limits<double>::signaling_NaN() }, mean_days_acute_rechallenged {
                std::numeric_limits<double>::signaling_NaN() }, mean_days_naive_to_infectious {
                std::numeric_limits<double>::signaling_NaN() }, mean_days_residual_hcv_infectivity {
                std::numeric_limits<double>::signaling_NaN() }, prob_self_limiting_female {
                std::numeric_limits<double>::signaling_NaN() }, prob_self_limiting_male {
                std::numeric_limits<double>::signaling_NaN() }, prob_clearing { std::numeric_limits<
                double>::signaling_NaN() }, transmissibility {
                std::numeric_limits<double>::signaling_NaN() }, treatment_duration {
                std::numeric_limits<double>::signaling_NaN() }, treatment_svr { std::numeric_limits<
                double>::signaling_NaN() }, treatment_susceptibility {
                std::numeric_limits<double>::signaling_NaN() }, treatment_repeatable(false)
{}

Immunology::Immunology(HCPerson* idu, IPPtr params) : params_(params), idu_(idu), hcv_state(HCV_State::susceptible), past_cured(false),
        past_recovered(false), in_treatment(false), treatment_start_date(TREATMENT_NOT_STARTED) {

}

Immunology::Immunology(HCPerson* idu, HCV_State alter_state, IPPtr params) : params_(params), idu_(idu), hcv_state(alter_state), past_cured(false),
        past_recovered(false), in_treatment(false), treatment_start_date(TREATMENT_NOT_STARTED) {

}

bool Immunology::exposePartner(Immunology& partner_imm, double tick) {
    Statistics* stats = Statistics::instance();
    stats->logStatusChange(LogType::EXPOSED, partner_imm.idu_, "by agent " + std::to_string(idu_->id()));

    if (! isHcvRNA(tick)) {
        return false;
    }

    if (isAcute()) {
        if (repast::Random::instance()->nextDouble()
                > CONTACT_RISK * params_->transmissibility * ACUTE_BOOST) {
            stats->logStatusChange(LogType::EXPOSED, idu_, "transmission failed");
            return false;
        }
    } else {
        if (repast::Random::instance()->nextDouble() > CONTACT_RISK * params_->transmissibility) {
            stats->logStatusChange(LogType::EXPOSED, idu_, "transmission failed");
            return false;
        }
    }

    bool established_new_infection = partner_imm.receiveInfectiousDose(tick); //receive_infectious_dose();
    if (established_new_infection) {
        partner_imm.idu_->setLastExposureDate(tick); //important: must follow Statistics.fire()
    }
    return established_new_infection;
}

void Immunology::leaveExposed() {
    hcv_state = HCV_State::infectiousacute;
    Statistics::instance()->logStatusChange(LogType::INFECTIOUS, idu_, "");
}

bool Immunology::leaveAcute() {

    //returns true iff self limiting
    double prob_self_limiting = idu_->getGender() == MALE ? params_->prob_self_limiting_male : params_->prob_self_limiting_female;
            //agent.getGender() == Gender.Male? prob_self_limiting_male : prob_self_limiting_female;
    if (((!past_recovered) && prob_self_limiting > repast::Random::instance()->nextDouble()) ||
         ((past_recovered) && params_->prob_clearing > repast::Random::instance()->nextDouble()) ||
             ((past_cured) && params_->treatment_susceptibility < repast::Random::instance()->nextDouble()))    {
        hcv_state = HCV_State::recovered;
        Statistics::instance()->logStatusChange(LogType::RECOVERED, idu_, "");
        return true;
    } else {
        hcv_state = HCV_State::chronic;
        Statistics::instance()->logStatusChange(LogType::CHRONIC, idu_, "");
        return false;
    }
}

void Immunology::purgeActions() {
    for (auto evt : scheduled_actions) {
        evt->cancel();
    }
    scheduled_actions.clear();
}

bool Immunology::receiveInfectiousDose(double now) {
    if (isHcvRNA(now) || isInTreatment()) {
        return false;
    }
    purgeActions();

    //note: this long memory changes the behavior compared to original APK, even in the default (no treatment) case.
    past_recovered = past_recovered | (hcv_state == HCV_State::recovered);
    past_cured     = past_cured     | (hcv_state == HCV_State::cured);

    hcv_state = HCV_State::exposed;
    Statistics::instance()->logStatusChange(LogType::INFECTED, idu_, "");
    repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
    double exposed_end_time = now + repast::Random::instance()->createExponentialGenerator(1.0 / params_->mean_days_naive_to_infectious).next();

    EventPtr leave_exposed_evt = boost::make_shared<Event>(new MethodFunctor<Immunology, void>(this, &Immunology::leaveExposed));
    scheduled_actions.push_back(leave_exposed_evt);
    runner.scheduleEvent(exposed_end_time, leave_exposed_evt);

    double acute_end_time;
    if (! past_recovered) {
        acute_end_time =  now + repast::Random::instance()->createExponentialGenerator(1.0 / params_->mean_days_acute_naive).next();
                //time_now + RandomHelper.createExponential(1./mean_days_acute_naive).nextDouble();
    } else {
        acute_end_time =  now + repast::Random::instance()->createExponentialGenerator(1.0 / params_->mean_days_acute_rechallenged).next();
    }

    EventPtr leave_acute_evt = boost::make_shared<Event>(new MethodFunctor<Immunology, bool>(this, &Immunology::leaveAcute));
    scheduled_actions.push_back(leave_acute_evt);
    runner.scheduleEvent(acute_end_time, leave_acute_evt);

    return true;
}


bool Immunology::isAcute() {
    return hcv_state == HCV_State::exposed || hcv_state == HCV_State::infectiousacute;
}

bool Immunology::isChronic() {
    return hcv_state == HCV_State::chronic;
}

bool Immunology::isCured() {
    return hcv_state == HCV_State::cured;
}

bool Immunology::isExposed() {
    return hcv_state == HCV_State::exposed;
}

bool Immunology::isHcvABpos() { //presence of antigens
    return (hcv_state != HCV_State::susceptible) || (hcv_state == HCV_State::ABPOS)
            || (hcv_state == HCV_State::cured);
}

bool Immunology::isHcvRNA(double now) {
    return (hcv_state == HCV_State::exposed || hcv_state == HCV_State::infectiousacute
            || hcv_state == HCV_State::chronic) && (!isIn_treatment_viral_suppression(now));
}

bool Immunology::isInfectious(double now) {
    return (hcv_state == HCV_State::infectiousacute || hcv_state == HCV_State::chronic)
            && (!isIn_treatment_viral_suppression(now));
}

bool Immunology::isInTreatment() {
    return in_treatment;
}

bool Immunology::isIn_treatment_viral_suppression(double tick) {
    if (!in_treatment) {
        return false;
    }
    return (treatment_start_date + params_->mean_days_residual_hcv_infectivity) < tick;
}

bool Immunology::isNaive() {
    return hcv_state == HCV_State::susceptible;
}

bool Immunology::isResistant() {
    return hcv_state == HCV_State::recovered;
}

bool Immunology::isPostTreatment() { //i.e. completed a course of treatment
    return (!in_treatment) & (treatment_start_date != TREATMENT_NOT_STARTED);
}

bool Immunology::isTreatable(double now) {
    return (!in_treatment) && isHcvRNA(now)
            && (params_->treatment_repeatable || (!isPostTreatment()));
}

HCV_State Immunology::getHCVState() {
    return hcv_state;
}

void Immunology::setHCVInitState(double now, HCV_State state, int logging) {
    //assert state != HCV_state.ABPOS; //used only in the DrugUser container
    hcv_state = state;
    switch (state) {
        case HCV_State::susceptible:
        {
            hcv_state = HCV_State::susceptible;
            if (logging > 0) {
                Statistics::instance()->logStatusChange(LogType::INFO, idu_, "new_hcv_state="+hcv_state_to_string(state));
            }
            break;
        }

//          case cured:
//              assert false; //should not be called from here
//              treatment_start_date = RepastEssentials.GetTickCount();
//              hcv_state = HCV_state.cured;
//              if (logging > 0) {
//                  Statistics.fire_status_change(AgentMessage.started_treatment, this.agent, "", null);
//              }
//              break;
//          case exposed:
//              assert false;//should not be called
//              receive_infectious_dose();
//              if (logging > 0) {
//                  Statistics.fire_status_change(AgentMessage.exposed, this.agent, "", null);
//                  this.agent.setLastExposureDate(); //must follow fire, b/c it uses the signature
//              }
//              break;

        case HCV_State::infectiousacute:
        {
            double acute_end_time = now + repast::Random::instance()->createExponentialGenerator(1.0 / params_->mean_days_acute_naive).next();
            repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
            EventPtr leave_acute_evt = boost::make_shared<Event>(new MethodFunctor<Immunology, bool>(this, &Immunology::leaveAcute));
            scheduled_actions.push_back(leave_acute_evt);
            runner.scheduleEvent(acute_end_time, leave_acute_evt);
            if (logging > 0) {
                Statistics::instance()->logStatusChange(LogType::INFECTIOUS, idu_, "");
                idu_->setLastExposureDate(now);
            }
            break;
        }


        case HCV_State::chronic:
        {
            idu_->setLastExposureDate(now);
            if (logging > 0) {
                Statistics::instance()->logStatusChange(LogType::CHRONIC, idu_, "");
                idu_->setLastExposureDate(now); //must follow fire, b/c it uses the signature
            }
            break;
        }
        case HCV_State::recovered:
        {
            idu_->setLastExposureDate(now);
            if (logging > 0) {
                Statistics::instance()->logStatusChange(LogType::RECOVERED, idu_, "");
                idu_->setLastExposureDate(now); //must follow fire, b/c it uses the signature
            }

            break;
        }
        default:
            throw std::invalid_argument("Unexpected state passed to setHCVInitState: " + hcv_state_to_string(state));

    }
}

void Immunology::leaveTreatment(bool treatment_succeeded) {
    in_treatment = false;
    if (treatment_succeeded) {
        hcv_state = HCV_State::cured;
        Statistics::instance()->logStatusChange(LogType::CURED, idu_, "");
    } else {
        Statistics::instance()->logStatusChange(LogType::FAILED_TREATMENT, idu_, "");
        hcv_state = HCV_State::chronic; //even if entered as acute.  ignore the case where was about to self-limit
    }
}

void Immunology::startTreatment(bool adherent, double now) {
    if(! isTreatable(now)) {
        std::cout << "Agent cannot be treated [doubly recruited?] ..." << (*idu_) << std::endl;
        return;
    }
    //prevent any accidental switch to chronic during treatment
    purgeActions(); //here - to purge any residual actions, such as switch to chronic
    treatment_start_date = now;
    in_treatment = true;
    Statistics::instance()->logStatusChange(LogType::STARTED_TREATMENT, idu_, "");

    repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
    double treatment_end_time = now + repast::Random::instance()->createNormalGenerator(params_->treatment_duration, 1).next();

    bool treatment_succeeds = (repast::Random::instance()->nextDouble() < params_->treatment_svr) && adherent;
    EventPtr treatment_end_evt = boost::make_shared<Event>(new EndTreatmentFunctor(treatment_succeeds, this));
    scheduled_actions.push_back(treatment_end_evt);
    runner.scheduleEvent(treatment_end_time, treatment_end_evt);
}


Immunology::~Immunology() {
}

} /* namespace crx */
