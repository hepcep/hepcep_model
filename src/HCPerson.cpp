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
#include "utilities.h"

namespace hepcep {


HCPerson::HCPerson(unsigned int id, HCPersonData& data) : AbsPersonT(id), lastExposureDate(-1.0) {

//	std::cout << "create Person " << id << std::endl;

	age = data.age;
	ageStarted = data.ageStarted;
	drug_outDegree = data.drug_outDegree;
	drug_inDegree = data.drug_inDegree;
	fractionReceptSharing = data.fractionReceptSharing;
	race = data.race;
	gender = parse_gender(data.gender);
	injectionIntensity = data.injectionIntensity;
	zipCode = data.zipCode;
	syringeSource = data.syringeSource;

	// TODO Set HCV state via Immunology as in APK Model
	hcvState = HCV_State::unknown;

	// If the HCV state is ABPOS then assign the specific infection state
	if(data.hcvState == HCV_State::ABPOS) {
		double ab_prob_chronic = chi_sim::Parameters::instance()->getDoubleParameter(AB_PROB_CHRONIC);
		double ab_prob_acute = chi_sim::Parameters::instance()->getDoubleParameter(AB_PROB_ACUTE);
		double roll = repast::Random::instance()->nextDouble() - ab_prob_chronic;

		if (roll < 0 ){
			hcvState = HCV_State::chronic;
		}
		else if (roll - ab_prob_acute < 0){
			hcvState = HCV_State::infectiousacute;
		}
		else {
			hcvState = HCV_State::recovered;
		}
	}
	// Otherwise the state is explicitly defined in the person data.
	else {
		hcvState = data.hcvState;
	}
}


HCPerson::~HCPerson() {
//	std::cout << "Destruct Person." << std::endl;
}

void HCPerson::doSomething() {
//    std::cout << id_ << ": hello " << std::endl;

    // TODO increment age
}

/*
 * attribute and social distance between the individuals (>=0)
 * specifically excludes geographic distance
 * assumes all attributes are weighted by 1.0.  allow some to contribute > 1.0
 */
double HCPerson::getDemographicDistance(PersonPtr other){
	double ret = 0.0;

	ret += (race == other->getRace()) ? 0.0: 1.0;

	// TODO why was this more complicated than a simple age difference?
//	ret += (Math.abs(APKBuilder.getDateDifference(this.birth_date, other.birth_date) / 10.0));

	ret += std::abs(age - other->getAge()) / 10.0;

	return ret / 2.0;
}

 std::ostream& operator<<(std::ostream& os, const HCPerson& person) {
     os << person.getAge() << "," << person.getGender() << "," << person.getRace() << "," << person.getZipcode() << "," << person.getSyringeSource()
             << "," << hcv_state_to_string(person.getHCVState()) << "," // << person.getHcvNeighborPrevalence()
                      << "," << person.getAgeStarted()
                      << "," << person.getDrugReceptDegree() << "," << person.getDrugGivingDegree() << "," <<
                      /*person.getNumBuddies() << */ "," << person.getInjectionIntensity() << "," << person.getFractionReceptSharing(); // << "," << person.getDatabaseLabel();

     return os;

 }

} /* namespace hepcep */
