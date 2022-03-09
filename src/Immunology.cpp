/**
 * @file Immunology.cpp
 * 
 * Immunology abstract base class.  Implementing subclasses should
 * define the infection mechanics.
 * 
 */

#include "Immunology.h"
#include "Statistics.h"

namespace hepcep {

// Constructor
Immunology::Immunology(HCPerson* idu) : hcv_state(HCVState::SUSCEPTIBLE),
    in_treatment(false),
    past_cured(false),
    past_recovered(false),
     treatment_start_date(TREATMENT_NOT_STARTED),  
    idu_(idu){

}

// void Immunology::leaveExposed() {
//     hcv_state = HCVState::INFECTIOUS_ACUTE;
//     Statistics::instance()->logStatusChange(LogType::INFECTIOUS, idu_, "");
// }

HCVState Immunology::getHCVState() {
    return hcv_state;
}

bool Immunology::isAcute() {
    return hcv_state == HCVState::EXPOSED || hcv_state == HCVState::INFECTIOUS_ACUTE;
}

bool Immunology::isChronic() {
    return hcv_state == HCVState::CHRONIC;;
}

bool Immunology::isCured() {
    return hcv_state == HCVState::CURED;
}

bool Immunology::isExposed() {
    return hcv_state == HCVState::EXPOSED;
}

bool Immunology::isHcvABpos() { //presence of antigens
    return (hcv_state != HCVState::SUSCEPTIBLE) || (hcv_state == HCVState::ABPOS)
            || (hcv_state == HCVState::CURED);
}

bool Immunology::isNaive() {
    return hcv_state == HCVState::SUSCEPTIBLE;
}

bool Immunology::isResistant() {
    return hcv_state == HCVState::RECOVERED;
}

bool Immunology::isInTreatment() {
    return in_treatment;
}

double Immunology::getTreatmentStartDate() {
    return treatment_start_date;
}

bool Immunology::isPostTreatment() { //i.e. completed a course of treatment
    return (!in_treatment) & (treatment_start_date != TREATMENT_NOT_STARTED);
}

/**
 * Determines if the individual is available for treatment.  Conditions include:
 *   - can't currently already be in treatment
 *   - must be currently infected
 *   - if treatment prohibition is enabled, then cannot have already been
 *     in treatment....
 *   - ...unless previous treatment failed due to SVR or adherence
 */
bool Immunology::isTreatable(double now) {
	if (in_treatment){
		return false;      // if not currently being treated
	}
    
    // TODO - We don't need the treatment_repeatable boolean since we can just set 
    //        max_num_daa_treatments to a large number, or = 1 for no re-treatments
    if (num_daa_treatments >= max_num_daa_treatments){
        return false;      // Don't treat if re-treatment max has been exceeded
    }

	if (!(treatment_repeatable)){          // if not repeatable then check conditions.
        if (isPostTreatment() || treatment_failed){   // Has already been in one treatment)
		                                              // or Treatment failure (not re-infect))
  			return false;
        }
	}
    return isHcvRNA(now);    // if currently infected
}

}