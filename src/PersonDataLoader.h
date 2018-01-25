/*
 * PersonCreator.h
 *
 *  
 */

#ifndef SRC_PERSONDATALOADER_H_
#define SRC_PERSONDATALOADER_H_

#include <map>
#include <string>

#include "HCPerson.h"

namespace hepcep {

/**
 * Load person data from the specified file
 */
void loadPersonData(const std::string& filename, std::vector<HCPersonData> & personData) ;

}

#endif /* SRC_PERSONDATALOADER_H_ */
