/*
 * PersonCreator.h
 *
 *  
 */

#ifndef SRC_PERSONCREATOR_H_
#define SRC_PERSONCREATOR_H_

#include <map>
#include <string>

#include "HCPerson.h"

using namespace std;

namespace hepcep {

class PersonCreator {

public:
	static unsigned int ID_COUNTER;

	PersonCreator();
	
    virtual ~PersonCreator();
	
	/**
	 * Create Persons from the specified file, placing them in the specified map.
	 */
	void create_persons(map<unsigned int, PersonPtr>& persons,
			deque<HCPersonData> & personData,  unsigned int person_count);


};

} // namespace hepcep

#endif /* SRC_PERSONCREATOR_H_ */
