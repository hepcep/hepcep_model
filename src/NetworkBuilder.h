/*
 * PersonCreator.h
 *
 *  
 */

#ifndef SRC_NETWORKBUILDER_H_
#define SRC_NETWORKBUILDER_H_

#include <map>
#include <string>

#include "HCPerson.h"
#include "Network.h"

namespace hepcep {

class NetworkBuilder {

public:

	NetworkBuilder();
	
    virtual ~NetworkBuilder();
	
	/**
	 * Create Persons from the specified file, placing them in the specified map.
	 */
//	void create_persons(std::map<unsigned int, PersonPtr>& persons, 
//			std::deque<HCPersonData> & personData,  unsigned int person_count);


};

} // namespace hepcep

#endif /* SRC_NETWORKBUILDER_H_ */
