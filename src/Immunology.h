/*
 * Immunology.h
 *
 *  Created on: Mar 1, 2018
 *      Author: nick
 */

#ifndef SRC_IMMUNOLOGY_H_
#define SRC_IMMUNOLOGY_H_

#include <memory>
#include <vector>

#include "boost/shared_ptr.hpp"

#include "HCV_State.h"
#include "Event.h"

namespace hepcep {

class HCPerson;

struct ImmunologyParameters {

     double mean_days_acute_naive, mean_days_acute_rechallenged,
     mean_days_naive_to_infectious, mean_days_residual_hcv_infectivity,
     prob_self_limiting_female, prob_self_limiting_male,
     prob_clearing, transmissibility,
     treatment_duration, treatment_svr,
     treatment_susceptibility;

     bool treatment_repeatable;

     ImmunologyParameters();
};

using IPPtr = std::shared_ptr<ImmunologyParameters>;

class Immunology {

private:
    IPPtr params_;

    // this is a pointer and not a shared_ptr because
    // HCPerson needs to pass itself.
    HCPerson* idu_;
    HCV_State hcv_state;
    std::vector<boost::shared_ptr<Event>> scheduled_actions;

    bool past_cured, past_recovered, in_treatment;
    double treatment_start_date;

    bool isIn_treatment_viral_suppression(double tick);

    /**
     * start a NATURAL infection via exposure.
     * 1. the calling method is responsible for announcing the exposure, and updating the time of last exposure
     * 2. if one is recovered, it's possible to be "infected" a new
     * 3. if one has RNA, then no new infection would be established.
     *
     * @returns true iff new infection has been started
     *
     */
    bool receiveInfectiousDose(double tick);
    void purgeActions();

public:
    Immunology(HCPerson* idu, IPPtr params);
    Immunology(HCPerson* idu, HCV_State alter_state, IPPtr params);
    virtual ~Immunology();

    /**
     * Exposes a partner, perhaps leading to infected.
     *
     * @return true if a new infection was established in partner, otherwise false.
     */
    bool exposePartner(Immunology& partner_imm, double tick);

    void leaveExposed();

    bool leaveAcute();

    bool isAcute();
    bool isChronic();
    bool isCured();
    bool isExposed();
    bool isHcvABpos();
    bool isHcvRNA(double now);
    bool isInfectious(double now);

    bool isInTreatment();

    bool isNaive();
    bool isResistant();
    bool isPostTreatment();
    bool isTreatable(double now);
    HCV_State getHCVState();

    /**
     *
     * for initializing agents
     * this function overrides the normal course of an infection
     * it should not be used for natural exposures
     *
     * censored_acute = in the middle of an acute infection
     */
    void setHCVInitState(double now, HCV_State state, int logging);

    void leaveTreatment(bool treatment_succeeded);

    /**
     * Initiates treatment.
     */
    void startTreatment(bool adherent, double now);
};


} /* namespace hepcep */

#endif /* SRC_IMMUNOLOGY_H_ */
