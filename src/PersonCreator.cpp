/*
 * PersonCreator.cpp
 *
 *  
 */

#include "repast_hpc/RepastProcess.h"

#include "PersonCreator.h"

namespace hepcep {

unsigned int PersonCreator::ID_COUNTER = 0;

PersonCreator::PersonCreator() {
	
}

void PersonCreator::create_persons(map<unsigned int, PersonPtr>& persons,
		deque<HCPersonData> & personData, unsigned int person_count){

    // not strictly necessary in initial version, as world size is always 1
    // but kept here to illustrate how to only create "local persons"
    int my_rank = repast::RepastProcess::instance()->rank();
    int world_size = repast::RepastProcess::instance()->worldSize();

	unsigned int count = 0;
	int p_rank = 0;
	
	
	while (count <= person_count) {
		HCPersonData data = personData.front();
		personData.pop_front();
				
		
	// TODO use constant rank 0 for now
//        int p_rank = std::stoi(line[RANK_IDX]);
        
		if (p_rank == my_rank || world_size == 1) {
//           persons.emplace(id, std::make_shared<HCPerson>(id));
		   
		   persons.emplace(ID_COUNTER, std::make_shared<HCPerson>(ID_COUNTER, data));
        }
	count++;
	ID_COUNTER++;  // increment id count
    }
}

PersonCreator::~PersonCreator() {
}

} // namespace hepcep
