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

namespace hepcep {

unsigned int PersonCreator::ID_COUNTER = 1;

PersonCreator::PersonCreator() {

}

void PersonCreator::create_persons(std::map<unsigned int, PersonPtr>& persons,
		std::vector<HCPersonData> & personData, std::map<std::string,ZonePtr>& zoneMap,
		unsigned int person_count){

	double status_report_frequency = chi_sim::Parameters::instance()->getDoubleParameter(STATUS_REPORT_FREQUENCY);

	unsigned int count = 1;

	repast::IntUniformGenerator generator = repast::Random::instance()->createUniIntGenerator (0, personData.size());

	while (count <= person_count) {
		// Select a random CNEP+ profile from which to create the Person instance
		int i = (int)generator.next();
		HCPersonData data = personData[i];

		// If the zone is undefined, skip this person data
		//   TODO should the data be pruned from the list to avoid repeat checks?
		if (zoneMap.find(data.zipCode) == zoneMap.end()){

		}
		else {
			auto person = std::make_shared<HCPerson>(ID_COUNTER, data);

			person->setZone(zoneMap[data.zipCode]);

			// TODO remaining code from APK IDUBuilder.add_new_IDUS()
//			idu.setEntryDate(APKBuilder.getSimulationDate());
			double elapsed_career_days = 365.0*(person->getAge() - person->getAgeStarted());
			double residual_burnin_days = 0;
			if (burnInMode) {
//				residual_burnin_days = Math.max(0, burn_in_days - RepastEssentials.GetTickCount());
			}

			// Dont include person in simulation if...
			if(! person->activate(residual_burnin_days, elapsed_career_days, status_report_frequency)) {

				// TODO additional hepcep actions here?
				continue;
			}


			persons[ID_COUNTER] = person;

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
