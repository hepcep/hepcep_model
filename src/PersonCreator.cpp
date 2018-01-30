/*
 * PersonCreator.cpp
 *
 *  
 */

#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/Random.h"

#include "PersonCreator.h"

namespace hepcep {

unsigned int PersonCreator::ID_COUNTER = 1;

PersonCreator::PersonCreator() {

}

void PersonCreator::create_persons(std::map<unsigned int, PersonPtr>& persons,
		std::vector<HCPersonData> & personData, std::map<std::string,ZonePtr>& zoneMap,
		unsigned int person_count){

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

			//		persons.emplace(ID_COUNTER, std::make_shared<HCPerson>(ID_COUNTER, data));

			person->setZone(zoneMap[data.zipCode]);

			persons[ID_COUNTER] = person;

			count++;
			ID_COUNTER++;  // increment id count
		}
	}
}

PersonCreator::~PersonCreator() {
}

} // namespace hepcep
