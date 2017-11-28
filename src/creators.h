/*
 * creators.h
 *
 *  Created on: Nov 27, 2017
 *      Author: nick
 */

#ifndef SRC_CREATORS_H_
#define SRC_CREATORS_H_

#include <map>
#include <string>

#include "HCPerson.h"
#include "Network.h"

namespace hepcep {


/**
 * Create Persons from the specified file, placing them in the specified map.
 */
void create_persons(const std::string& filename, std::map<unsigned int, PersonPtr>& persons);

void create_network(const std::string& filename, std::map<unsigned int, PersonPtr>& persons, Network<HCPerson>& network);


}



#endif /* SRC_CREATORS_H_ */
