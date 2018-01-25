/*
 * PersonCreator.cpp
 *
 *  
 */

#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/Random.h"

#include "PersonCreator.h"

namespace hepcep {

unsigned int PersonCreator::ID_COUNTER = 0;

PersonCreator::PersonCreator() {

}

void PersonCreator::create_persons(std::map<unsigned int, PersonPtr>& persons,
		std::vector<HCPersonData> & personData, unsigned int person_count){

	// not strictly necessary in initial version, as world size is always 1
	// but kept here to illustrate how to only create "local persons"
	int my_rank = repast::RepastProcess::instance()->rank();
	int world_size = repast::RepastProcess::instance()->worldSize();

	unsigned int count = 0;
	int p_rank = 0;

	repast::IntUniformGenerator generator = repast::Random::instance()->createUniIntGenerator (0, personData.size());

	while (count <= person_count) {
		// Select a random CNEP+ profile from which to create the Person instance
		int i = (int)generator.next();
		HCPersonData data = personData[i];

		// TODO use constant rank 0 for now
//     int p_rank = std::stoi(line[RANK_IDX]);

		if (p_rank == my_rank || world_size == 1) {
			persons.emplace(ID_COUNTER, std::make_shared<HCPerson>(ID_COUNTER, data));
		}
		count++;
		ID_COUNTER++;  // increment id count
	}
}

PersonCreator::~PersonCreator() {
}

} // namespace hepcep
