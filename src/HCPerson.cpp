/*
 * HCPerson.cpp
 *
 *  Created on: Nov 27, 2017
 *      Author: nick
 */

#include <math.h>

#include "repast_hpc/Random.h"
#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/Schedule.h"
#include "chi_sim/Parameters.h"

#include "parameters_constants.h"
#include "HCPerson.h"
#include "Network.h"

namespace hepcep {


HCPerson::HCPerson(unsigned int id, HCPersonData& data) : AbsPersonT(id),
		gender(Gender::FEMALE), race(Race::OTHER),
		syringeSource(HarmReduction::HARM_REDUCTION),
		lastExposureDate(-1.0) {

//	std::cout << "create Person " << id << std::endl;


	immunology = std::make_shared<Immunology>(this);

	age = data.age;
	ageStarted = data.ageStarted;
	drug_outDegree = data.drug_outDegree;
	drug_inDegree = data.drug_inDegree;
	fractionReceptSharing = data.fractionReceptSharing;
	race = Race::valueOf(data.race);
	gender = Gender::valueOf(data.gender);
	injectionIntensity = data.injectionIntensity;
	zipCode = data.zipCode;
	syringeSource = HarmReduction::valueOf(data.syringeSource);

	HCVState hcvState = HCVState::UNKNOWN;

	// If the HCV state is ABPOS then assign the specific infection state
	if(data.hcvState == HCVState::ABPOS) {
		double ab_prob_chronic = chi_sim::Parameters::instance()->getDoubleParameter(AB_PROB_CHRONIC);
		double ab_prob_acute = chi_sim::Parameters::instance()->getDoubleParameter(AB_PROB_ACUTE);
		double roll = repast::Random::instance()->nextDouble() - ab_prob_chronic;

		if (roll < 0 ){
			hcvState = HCVState::CHRONIC;
		}
		else if (roll - ab_prob_acute < 0){
			hcvState = HCVState::INFECTIOUS_ACUTE;
		}
		else {
			hcvState = HCVState::RECOVERED;
		}
	}
	// Otherwise the state is explicitly defined in the person data.
	else {
		hcvState = data.hcvState;
	}

	double tick = repast::RepastProcess::instance()->getScheduleRunner().currentTick();
	immunology->setHCVInitState(tick,hcvState,0);
}


HCPerson::~HCPerson() {
//	std::cout << "Destruct Person." << std::endl;
}

void HCPerson::step() {
//    std::cout << id_ << ": hello " << std::endl;

	if (! active) {
		return;
	}

	double n = repast::Random::instance()->nextDouble();
	double num_sharing_episodes = round(n	* injectionIntensity *
			fractionReceptSharing);

	for (int episode=0; episode<num_sharing_episodes; ++episode) {
		receive_equipment_or_drugs();
	}

  age += 1.0 / 365.0;
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

/*
 * let the agent know that it's about go enter the simulation
 * this method should be called before adding the IDU to context
 * life_extension is used to adjust for possible burn-in time or time already in the drug career
 */
bool HCPerson::activate(double residual_burnin_days, double elapsed_career_days,
		double status_report_frequency) {

	active = true;

	// TODO Scheduling
//	if(! schedule_end(residual_burnin_days, elapsed_career_days)) {
//		return false;
//	}
//
//	if(status_report_frequency > 0) {
//		ISchedule schedule = RunEnvironment.getInstance().getCurrentSchedule();
//		ScheduleParameters sched_params = ScheduleParameters.createRepeating(RepastEssentials.GetTickCount()+0.0001, status_report_frequency);
//		my_status = schedule.schedule(sched_params, this, "report_status");
//	}
	return true;
}

void HCPerson::deactivate(){
	// TODO deactive
//	Statistics.fire_status_change(AgentMessage.deactivated, this, "", null);
//	context.remove(this);
	immunology->deactivate();
//	if(my_status != null) {
//		ISchedule schedule = RunEnvironment.getInstance().getCurrentSchedule();
//		schedule.removeAction(my_status);
//	}
}

void HCPerson::receive_equipment_or_drugs() {

	// be exposed to the blood of a friend
	std::vector<EdgePtrT<HCPerson>> vec;

	// TODO not working!
//	network->inEdges(this, vec);

	// Get a random in-edge
	int n = vec.size();
	double roll = repast::Random::instance()->nextDouble();
	int idx = std::round(roll * (n-1));

//	PersonPtr donor = vec[idx]->v1();
//
//	if (donor != NULL) {
//		double tick = repast::RepastProcess::instance()->getScheduleRunner().currentTick();
//		donor->immunology->exposePartner(this->immunology, tick);
//	}
}


void HCPerson::startTreatment() {
	// TODO start treatment
//		bool adherent = RandomHelper.nextDouble() > treatment_nonadherence;
//		immunology->startTreatment(adherent);
	}

std::ostream& operator<<(std::ostream& os, const HCPerson& person) {
	os << person.getAge() << "," << person.getGender().stringValue() << "," << person.getRace().stringValue() << "," << person.getZipcode() << ","
			<< person.getSyringeSource().stringValue()
			<< "," << person.getHCVState().stringValue() << "," // << person.getHcvNeighborPrevalence()
			<< "," << person.getAgeStarted()
			<< "," << person.getDrugReceptDegree() << "," << person.getDrugGivingDegree() << "," <<
			/*person.getNumBuddies() << */ "," << person.getInjectionIntensity() << "," << person.getFractionReceptSharing(); // << "," << person.getDatabaseLabel();

     return os;
 }

void HCPerson::setNetwork(NetworkPtr<HCPerson> aNet){
	network = aNet;
}

} /* namespace hepcep */
