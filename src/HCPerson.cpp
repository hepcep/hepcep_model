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

#include "Distributions.h"
#include "parameters_constants.h"
#include "HCPerson.h"
#include "Network.h"
#include "Statistics.h"

namespace hepcep {

HCPerson::HCPerson(unsigned int id, HCPersonData& data, std::shared_ptr<Immunology> imm) :  AbsPersonT(id),
		gender(Gender::FEMALE), race(Race::OTHER),
		syringeSource(HarmReduction::NON_HARM_REDUCTION),
		lastExposureDate(-1.0),
		lastInfectionDate(-1.0),
		deactivateAt(-1.0) , immunology(imm) 
{
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
}


HCPerson::HCPerson(unsigned int id, HCPersonData& data) : AbsPersonT(id),
		gender(Gender::FEMALE), race(Race::OTHER),
		syringeSource(HarmReduction::NON_HARM_REDUCTION),
		lastExposureDate(-1.0),
		lastInfectionDate(-1.0),
		deactivateAt(-1.0) {


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
    immunology->purgeActions();
}

void HCPerson::step(NetworkPtr<HCPerson> network) {
//    std::cout << id_ << ": step " << std::endl;

	double n = repast::Random::instance()->nextDouble();
	double num_sharing_episodes = round(n	* injectionIntensity *
			fractionReceptSharing);

	for (int episode=0; episode<num_sharing_episodes; ++episode) {
		receive_equipment_or_drugs(network);
	}

  age += 1.0 / 365.0;
}

double HCPerson::getDeactivateAt() const {
	return deactivateAt;
}

/*
 * attribute and social distance between the individuals (>=0)
 * specifically excludes geographic distance
 * assumes all attributes are weighted by 1.0.  allow some to contribute > 1.0
 */
double HCPerson::getDemographicDistance(PersonPtr other) const {
	double ret = 0.0;

	// race adjustment
	ret += (race == other->getRace()) ? 0.0: 1.0;

	// age adjustment
	ret += std::abs(age - other->getAge()) / 10.0;

	return ret / 2.0;
}

/*
 * let the agent know that it's about go enter the simulation
 * this method should be called before adding the IDU to context
 * life_extension is used to adjust for possible burn-in time or time already in
 * the drug career
 */
bool HCPerson::activate(double residualBurninDays, double elapsedCareerDays,
		double statusReportFrequency) {

	active = true;

	if(! scheduleEnd(residualBurninDays, elapsedCareerDays)) {
		return false;
	}

	// TODO event recording
//	if(status_report_frequency > 0) {
//		ISchedule schedule = RunEnvironment.getInstance().getCurrentSchedule();
//		ScheduleParameters sched_params = ScheduleParameters.createRepeating(RepastEssentials.GetTickCount()+0.0001, status_report_frequency);
//		my_status = schedule.schedule(sched_params, this, "report_status");
//	}
	return true;
}

void HCPerson::deactivate(){
    Statistics::instance()->logStatusChange(LogType::DEACTIVATED, this, "");
    active = false;
    immunology->deactivate();

	// TODO schedule this may not be needed depending on how we implement reporting
	//	if(my_status != null) {
//		ISchedule schedule = RunEnvironment.getInstance().getCurrentSchedule();
//		schedule.removeAction(my_status);
//	}
}

void HCPerson::receive_equipment_or_drugs(NetworkPtr<HCPerson> network) {

	// be exposed to the blood of a friend
	std::vector<EdgePtrT<HCPerson>> vec;

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

/*
 * called at initialization to plan the death of this agent
 *
 */
bool HCPerson:: scheduleEnd(double residualBurninDays, double elapsedCareerDays) {

	//distributions for days until end
	double residualLife = 0;
	double timeToCessation = 0;
	double residualTimeInApk = 0;

	double probCessation = chi_sim::Parameters::instance()->getDoubleParameter(PROB_CESSATION);

	for(int trial=0; trial<100; ++trial) {
		//anticipate lifetime from birth, accounting for burnin period
		
        residualLife = residualBurninDays + Distributions::instance()->getLifespanRandom();
		        
        residualLife -= getAge() * 365.0;

		double roll = repast::Random::instance()->nextDouble();
		if(roll < probCessation) {
			timeToCessation = residualBurninDays - elapsedCareerDays + Distributions::instance()->getCessationTimeRandom();

			residualTimeInApk = min(timeToCessation, residualLife);
		}
		else {
			residualTimeInApk = residualLife;
		}
		if (residualTimeInApk > residualBurninDays) {
			break;
		}
	}
	if (residualTimeInApk <= residualBurninDays) {
		return false;
	}


	// TODO decide if the scheduled action needs an equivalent HPC object
//	my_end = schedule.schedule(death_sched_params, this, "deactivate");

	// Schedule death deactivate method
	double tick = repast::RepastProcess::instance()->getScheduleRunner().currentTick();
	residualTimeInApk += tick;
	deactivateAt = residualTimeInApk;
	repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
	runner.scheduleEvent(residualTimeInApk, repast::Schedule::FunctorPtr(
			new repast::MethodFunctor<HCPerson>(this, &HCPerson::deactivate)));

	return true;
}

void HCPerson::startTreatment() {
	double treatmentNonadherence = chi_sim::Parameters::instance()->getDoubleParameter(TREATMENT_NONADHERENCE);
	double roll = repast::Random::instance()->nextDouble();

	bool adherent = (roll > treatmentNonadherence);
	double tick = repast::RepastProcess::instance()->getScheduleRunner().currentTick();
	immunology->startTreatment(adherent,tick);

//	std::cout << "Treatment started: " << this->id() << std::endl;
}

void HCPerson::endRelationship(PersonPtr buddy, NetworkPtr<HCPerson> network){

	// TODO Using id() for now but check if we should use the Person pointer instead.
	EdgePtrT<HCPerson> edge = network->removeEdge(this->id(), buddy->id());

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

unsigned int HCPerson::getZipcode() const {
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

void HCPerson::setLastInfectionDate(double tick){
    lastInfectionDate = tick;
}

double HCPerson::getLastInfectionDate() const {
    return lastInfectionDate;
}

bool HCPerson::isHcvABpos() const {
	return immunology->isHcvABpos();
}

bool HCPerson::isHcvRNA() const{
	double tick = repast::RepastProcess::instance()->getScheduleRunner().currentTick();
	return immunology->isHcvRNA(tick);
}

bool HCPerson::getTestedHCV() const {
	double tick = repast::RepastProcess::instance()->getScheduleRunner().currentTick();
  return immunology->getTestedHCV(tick);
}

// Returns true if the last infection date tick is now (today)
bool HCPerson::isInfectedToday() const{
	double tick = repast::RepastProcess::instance()->getScheduleRunner().currentTick();
	return tick == lastInfectionDate;
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

bool HCPerson::isActive() const{
	return active;
}

void HCPerson::setHcvInitialState(HCVState hcvState, double tick){
	immunology->setHCVInitState(tick,hcvState,0);
}



} /* namespace hepcep */
