/*
 * PersonCreator.h
 *
 *  
 */

#ifndef SRC_PERSONDATALOADER_H_
#define SRC_PERSONDATALOADER_H_

#include <map>
#include <string>
#include <vector>
#include <unordered_map>

#include "HCPerson.h"

namespace hepcep {

/**
 * Load person data from the specified file
 */
void loadPersonData(const std::string& filename, std::vector<HCPersonData> & personData, const std::string& pwid_data_type);

void load_pwid_edge_data(const std::string& filename, std::unordered_map<unsigned int, std::vector<int>> & edge_data);

}

#endif /* SRC_PERSONDATALOADER_H_ */
