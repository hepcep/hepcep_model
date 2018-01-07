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
#include "Network.h"

namespace hepcep {


/**
 * Create Persons from the specified file, placing them in the specified map.
 */
void create_persons(const std::string& filename, std::map<unsigned int, PersonPtr>& persons);


}



#endif /* SRC_PERSONCREATOR_H_ */
