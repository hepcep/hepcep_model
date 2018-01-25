/*
 * HCPerson.cpp
 *
 *  Created on: Nov 27, 2017
 *      Author: nick
 */

#include "repast_hpc/Random.h"
#include "chi_sim/Parameters.h"

#include "parameters_constants.h"
#include "HCPerson.h"

namespace hepcep {


HCPerson::HCPerson(unsigned int id, HCPersonData& data) : AbsPersonT(id) {
	age = data.age;
	ageStarted = data.ageStarted;
	drug_outDegree = data.drug_outDegree;
	drug_inDegree = data.drug_inDegree;
	fractionReceptSharing = data.fractionReceptSharing;
	race = data.race;
	gender = data.gender;
	injectionIntensity = data.injectionIntensity;
	zipCode = data.zipCode;
	syringeSource = data.syringeSource;

	// TODO Set HCV state via Immunology as in APK Model
	hcvState = HCV_State::unknown;

	if(data.hcvState == HCV_State::ABPOS) {
		double ab_prob_chronic = chi_sim::Parameters::instance()->getDoubleParameter(AB_PROB_CHRONIC);
		double ab_prob_acute = chi_sim::Parameters::instance()->getDoubleParameter(AB_PROB_ACUTE);
		double roll = repast::Random::instance()->nextDouble() - ab_prob_chronic;

		if (roll < 0 ){
			hcvState = HCV_State::chronic;
		} else if (roll - ab_prob_acute < 0){
			hcvState = HCV_State::infectiousacute;
		} else {
			hcvState = HCV_State::recovered;
		}
	} else {
		hcvState = HCV_State::susceptible;
	}

}

HCPerson::~HCPerson() {
}

void HCPerson::doSomething() {
//    std::cout << id_ << ": hello " << std::endl;

    // TODO increment age
}

} /* namespace hepcep */
