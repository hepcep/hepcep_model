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
void loadZones(const std::string& filename, std::map<unsigned int, ZonePtr> & zonesMap);

void loadZonesDistances(const std::string& filename, std::map<unsigned int, ZonePtr> & zonesMap,
		std::map<unsigned int, std::map<unsigned int,double>> & zoneDistanceMap) ;

}

#endif /* SRC_ZONELOADER_H_ */
