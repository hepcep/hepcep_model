/*
 * PersonCreator.h
 *
 *  
 */

#ifndef SRC_ZONELOADER_H_
#define SRC_ZONELOADER_H_

#include <map>
#include <string>
#include <iostream>

#include "Zone.h"

namespace hepcep {


/**
 * Load zones data from the specified file
 */
void loadZones(const std::string& filename, std::map<std::string, ZonePtr> & zonesMap);

void loadZonesDistances(const std::string& filename, std::map<std::string, ZonePtr> & zonesMap,
		std::map<std::string, std::map<std::string,double>> & zoneDistanceMap) ;

}

#endif /* SRC_ZONELOADER_H_ */
