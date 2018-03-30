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
#include "Statistics.h"

namespace hepcep {


HCPerson::HCPerson(unsigned int id, HCPersonData& data) : AbsPersonT(id),
		gender(Gender::FEMALE), race(Race::OTHER),
		syringeSource(HarmReduction::NON_HARM_REDUCTION),
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

void HCPerson::step(NetworkPtr<HCPerson> network) {
//    std::cout << id_ << ": hello " << std::endl;

	if (! active) {
		return;
	}

	double n = repast::Random::instance()->nextDouble();
	double num_sharing_episodes = round(n	* injectionIntensity *
			fractionReceptSharing);

	for (int episode=0; episode<num_sharing_episodes; ++episode) {
		receive_equipment_or_drugs(network);
	}

  age += 1.0 / 365.0;
}

/*
 * attribute and social distance between the individuals (>=0)
 * specifically excludes geographic distance
 * assumes all attributes are weighted by 1.0.  allow some to contribute > 1.0
 */
double HCPerson::getDemographicDistance(PersonPtr other) const {
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
  Statistics::instance()->logStatusChange(LogType::DEACTIVATED, this, "");

  // TODO remove the PersonPtr from the local_persons lisr in the model
//	context.remove(this);

	immunology->deactivate();

	// TODO schedule
	//	if(my_status != null) {
//		ISchedule schedule = RunEnvironment.getInstance().getCurrentSchedule();
//		schedule.removeAction(my_status);
//	}
}

void HCPerson::receive_equipment_or_drugs(NetworkPtr<HCPerson> network) {

	// be exposed to the blood of a friend
	std::vector<EdgePtrT<HCPerson>> vec;

	// TODO not working!
	network->inEdges(this, vec);

	// Get a random in-edge
	int n = vec.size();

	if (n > 0){
		double roll = repast::Random::instance()->nextDouble();
		int idx = std::round(roll * (n-1));

		EdgePtrT<HCPerson> edge = vec[idx];
		PersonPtr donor = edge->v1();

		if (donor != NULL) {
			double tick = repast::RepastProcess::instance()->getScheduleRunner().currentTick();
			donor->immunology->exposePartner(this->immunology, tick);
		}
	}
}

void HCPerson::reportStatus() {
	Statistics::instance()->logStatusChange(LogType::STATUS, this, "");
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

unsigned int HCPerson::getDrugReceptDegree() const {
	return drug_inDegree;
}

unsigned int HCPerson::getDrugGivingDegree() const {
	return drug_outDegree;
}

ZonePtr HCPerson::getZone() const {
	return myZone;
}

void HCPerson::setZone(ZonePtr zone){
	myZone = zone;
}

std::string HCPerson::getZipcode() const {
	return zipCode;
}

double HCPerson::getAge() const {
	return age;
}

void HCPerson::setAge(double newAge) {
	age = newAge;
}

double HCPerson::getAgeStarted() const {
	return ageStarted;
}

Race HCPerson::getRace() const {
	return race;
}

Gender HCPerson::getGender() const {
	return gender;
}

HarmReduction HCPerson::getSyringeSource() const {
	return syringeSource;
}

HCVState HCPerson::getHCVState() const {
	return immunology->getHCVState();
}

double HCPerson::getInjectionIntensity() const {
	return injectionIntensity;
}

double HCPerson::getFractionReceptSharing() const {
	return fractionReceptSharing;
}

void HCPerson::setLastExposureDate(double tick){
    lastExposureDate = tick;
}

double HCPerson::getLastExposureDate() const {
    return lastExposureDate;
}

bool HCPerson::isHcvABpos() const {
	return immunology->isHcvABpos();
}

bool HCPerson::isHcvRNA() const{
	double tick = repast::RepastProcess::instance()->getScheduleRunner().currentTick();
	return immunology->isHcvRNA(tick);
}

bool HCPerson::isCured() const {
	return immunology->isCured();
}

bool HCPerson::isInTreatment() const {
	return immunology->isInTreatment();
}

bool HCPerson::isInHarmReduction() const {
	return (syringeSource == HarmReduction::HARM_REDUCTION);
}

bool HCPerson::isPostTreatment() const {
	return immunology->isPostTreatment();
}

bool HCPerson::isTreatable() const {
	double tick = repast::RepastProcess::instance()->getScheduleRunner().currentTick();
	return immunology->isTreatable(tick);
}


} /* namespace hepcep */
