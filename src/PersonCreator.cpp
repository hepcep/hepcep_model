/*
 * PersonCreator.cpp
 *
 *  
 */

#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/Random.h"

#include "chi_sim/Parameters.h"

#include "PersonCreator.h"
#include "parameters_constants.h"
#include "Statistics.h"

namespace hepcep {

unsigned int PersonCreator::ID_COUNTER = 1;

PersonCreator::PersonCreator() {
	statusReportFrequency = chi_sim::Parameters::instance()->getDoubleParameter(STATUS_REPORT_FREQUENCY);
	probInfectedNewArriving = chi_sim::Parameters::instance()->getDoubleParameter(PROB_INFECTED_NEW_ARRIVING);
}

// TODO change to return a vector that we can emplace into a map in HCModel
void PersonCreator::create_persons(std::map<unsigned int, PersonPtr>& persons,
		std::vector<HCPersonData> & personData, std::map<std::string,ZonePtr>& zoneMap,
		unsigned int person_count, bool earlyCareerOnly){

	unsigned int count = 1;

	repast::IntUniformGenerator generator = repast::Random::instance()->createUniIntGenerator (0, personData.size() - 1);

	double tick = repast::RepastProcess::instance()->getScheduleRunner().currentTick();

	while (count <= person_count) {
		// Select a random CNEP+ profile from which to create the Person instance

		HCPersonData data;
		// Sample from entire CNEP+
		if (!earlyCareerOnly){
			int i = (int)generator.next();
			data = personData[i];
		}

		// TODO this seems computationally wasteful..
		else{  // Sample only from early career persons
			int remaining_trials;
			for (remaining_trials = 50; remaining_trials > 0; --remaining_trials) {
				int i = (int)generator.next();
				HCPersonData candidate = personData[i];
				if (candidate.early_career == true){
					data = candidate;
					break;
				}
			}
		}

		// If the zone is undefined, skip this person data
		//   TODO should the data be pruned from the list to avoid repeat checks?
		if (zoneMap.find(data.zipCode) == zoneMap.end()){

		}
		else {
			auto person = std::make_shared<HCPerson>(ID_COUNTER, data);

			person->setZone(zoneMap[data.zipCode]);

			if (earlyCareerOnly){
				// Force early career PWIDs to be young.  Subtract some time from the
				//   person's age based on their time since starting injecting.
				double daysSinceInitiated = 365.0*(person->getAge() - person->getAgeStarted());

				// TODO How to handle birthdate?
				// APK adds days to the birth date which makes the person younger, so
				//    HepCEP can just decrease the age accordingly.
//				idu.setBirthDate(idu.getBirthDate().plusDays(days_since_initiated - 50));

				double age = person->getAge();
				double newAge = age - (daysSinceInitiated - 50.0) / 365.0;
				person->setAge(newAge);


				// TODO make sure setting the initial HCV state doesnt create duplicate
				//     schedule entries in the immunology.  APK follows the same pattern.
				double roll = repast::Random::instance()->nextDouble();
				if(probInfectedNewArriving < roll) { //typically for new IDU
					person->setHcvInitialState(HCVState::SUSCEPTIBLE, tick);
				} else {
					person->setHcvInitialState(HCVState::INFECTIOUS_ACUTE, tick);
				}
			}

			// TODO how to handle entry date?
//			idu.setEntryDate(APKBuilder.getSimulationDate());

			double elapsedCareerDays = 365.0*(person->getAge() - person->getAgeStarted());
			double residualBurninDays = 0;
			if (burnInMode) {
				residualBurninDays = max(0., burnInDays - tick);
			}

			// Dont include person in simulation if...
			if(! person->activate(residualBurninDays, elapsedCareerDays, statusReportFrequency)) {

				// TODO additional hepcep actions here?
				continue;
			}

			Statistics::instance()->logStatusChange(LogType::ACTIVATED, person, "");

			persons[ID_COUNTER] = person;

	//		std::cout << "new per age: " << person->getAge() << std::endl;

			count++;
			ID_COUNTER++;  // increment id count
		}
	}
}

void PersonCreator::setBurnInPeriod(bool burnInMode, double burnInPeriod){
	this->burnInMode = burnInMode;
	this->burnInDays = burnInPeriod;
}

PersonCreator::~PersonCreator() {
}

} // namespace hepcep
