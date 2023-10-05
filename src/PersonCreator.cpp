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



PersonCreator::PersonCreator(unsigned int starting_id) : burnInMode(false), burnInDays(0),
	probInfectedNewArriving(chi_sim::Parameters::instance()->getDoubleParameter(PROB_INFECTED_NEW_ARRIVING)),
	id_counter(starting_id) {
}

/**
 * @brief Create persons by random sampling from person data.
 * 
 * @param persons 
 * @param personData 
 * @param zoneMap 
 * @param network 
 * @param person_count 
 * @param earlyCareerOnly 
 */
void PersonCreator::create_persons(std::map<unsigned int, PersonPtr>& persons,
		std::vector<HCPersonData> & personData, std::map<unsigned int,ZonePtr>& zoneMap,
		NetworkPtr<HCPerson> network, unsigned int person_count, bool earlyCareerOnly){

	unsigned int count = 1;
    
	repast::IntUniformGenerator generator = repast::Random::instance()->createUniIntGenerator (0, personData.size() - 1);
    
	while (count <= person_count) {
		// Select a random CNEP+ profile from which to create the Person instance

		HCPersonData data;
		// Sample from entire CNEP+
		if (!earlyCareerOnly){
			int i = (int)generator.next();
			data = personData[i];
		}

		// TODO this seems computationally wasteful..
		// TODO need a 100% way to find a young PWID as this can still fail.
		else{  // Sample only from early career persons
			int remaining_trials;
			for (remaining_trials = 250; remaining_trials > 0; --remaining_trials) {
				int i = (int)generator.next();
				HCPersonData candidate = personData[i];
				if (candidate.early_career == true){
					data = candidate; 
					break;
				}
			}
		}

        create_person_from_data(persons, data, zoneMap, network, earlyCareerOnly);
		count++;
	}
}

/**
 * @brief Create persons using the complete HCPersonData from the ERGM input file. 
 * 
 * @param persons 
 * @param personData 
 * @param zoneMap 
 * @param network 
 */
void PersonCreator::create_persons_from_ergm_data(std::map<unsigned int, PersonPtr>& persons,
		std::vector<HCPersonData> & personData, std::map<unsigned int,ZonePtr>& zoneMap,
		NetworkPtr<HCPerson> network){

	for (auto data : personData){
		create_person_from_data(persons, data, zoneMap, network, false);
	}
}

/**
 * @brief Create a single person from a single HCPersonData.
 * 
 * @param persons 
 * @param data 
 * @param zoneMap 
 * @param network 
 */
void PersonCreator::create_person_from_data(std::map<unsigned int, PersonPtr>& persons,
		HCPersonData & data, std::map<unsigned int,ZonePtr>& zoneMap,
		NetworkPtr<HCPerson> network, bool earlyCareerOnly){

	double tick = repast::RepastProcess::instance()->getScheduleRunner().currentTick();
    
	// TODO use a vector of zone ids insteaf of zone pointers
	
	auto person = std::make_shared<HCPerson>(id_counter, data);
	auto zone = zoneMap[data.zipCode];

	// std::cout << "Create Person: " << person->id() << ", zip: " << person->getZipcode() << std::endl;
	
	// NOTE: person can have a valid 5-digit int zip code, but a corresponding zone may not exist.
	if (zone == nullptr){
		// std::cout << "WARNING: Person " << person->id() << ", zip: " << person->getZipcode() <<
		//  " does not exist in Zones data." << std::endl;
	}
	else {
		// std::cout << "\t Zone zip: " << zone->getZipcode() << std::endl;
	}
	person->setZone(zone);

	if (earlyCareerOnly){
		// Force early career PWIDs to be young.  Subtract some time from the
		//   person's age based on their time since starting injecting.
		double daysSinceInitiated = 365.0*(person->getAge() - person->getAgeStarted());

		double age = person->getAge();
		double newAge = age - (daysSinceInitiated - 50.0) / 365.0;
		person->setAge(newAge);

		// TODO make sure setting the initial HCV state doesnt create duplicate
		//     schedule entries in the immunology.  APK follows the same pattern.
		double roll = repast::Random::instance()->nextDouble();
		
		if(probInfectedNewArriving < roll) { //typically for new IDU
			person->setHcvInitialState(HCVState::SUSCEPTIBLE, tick);
		} 
		else {
			person->setHcvInitialState(HCVState::INFECTIOUS_ACUTE, tick);
		}
	}

	double elapsedCareerDays = 365.0*(person->getAge() - person->getAgeStarted());
	double residualBurninDays = 0;
	if (burnInMode) {
		residualBurninDays = max(0., burnInDays - tick);
	}

	// Check if the person can be activated based on lifespan and burn in period.
	// person.activate() will iteratively attempt to find a lifespan that meets this
	// crieteria, but can fail.
	if(! person->activate(residualBurninDays, elapsedCareerDays)) {
		std::cout << "!!!! Failed to activate person: " << person->id() << std::endl;
		return;
	}
	// std::cout << "Activate person: " << person->id() << std::endl;

	Statistics::instance()->logStatusChange(LogType::ACTIVATED, person, "");

	persons[id_counter] = person;
	network->addVertex(person);

	++id_counter;  // increment id count

	// Log agent properties on creation
	Statistics::instance()->logPerson(person);	
}

void PersonCreator::setBurnInPeriod(bool burnInMode, double burnInPeriod){
	this->burnInMode = burnInMode;
	this->burnInDays = burnInPeriod;
}

PersonCreator::~PersonCreator() {
}

} // namespace hepcep
