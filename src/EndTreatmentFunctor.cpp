/*
 * EndTreamentFunctor.cpp
 *
 *  Created on: Mar 2, 2018
 *      Author: nick
 */

#include "EndTreatmentFunctor.h"

#include "Immunology.h"

namespace hepcep {

EndTreatmentFunctor::EndTreatmentFunctor(bool treatment_success, Immunology* imm) : success(treatment_success), immunology(imm) {
}

EndTreatmentFunctor::~EndTreatmentFunctor() {
}

void EndTreatmentFunctor::operator()() {

	if (immunology){
    immunology->leaveTreatment(success);
	}
	else{
		std::cout << "NULL immunology ptr" << std::endl;
	}
}

} /* namespace hepcep */
