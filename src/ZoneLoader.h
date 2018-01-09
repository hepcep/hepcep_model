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

using namespace std;

namespace hepcep {


/**
 * Load zones data from the specified file
 */
void loadZones(const string& filename, map<std::string, Zone> & zonesMap); 

void loadZonesDistances(const string& filename, map<std::string, Zone> & zonesMap,
		map<Zone, map<Zone,double>> & zoneDistanceMap) ;

}

#endif /* SRC_ZONELOADER_H_ */
