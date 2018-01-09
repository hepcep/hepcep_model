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

using namespace std;

namespace hepcep {


/**
 * Load person data from the specified file
 */
void loadPersonData(const string& filename, deque<HCPersonData> & personData) ;


}

#endif /* SRC_PERSONDATALOADER_H_ */
