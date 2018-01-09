/*
 * ZoneLoader.cpp
 *
 *  
 */

#include "ZoneLoader.h"
#include "SVReader.h"

namespace hepcep {

const int ZIPCODE_IDX = 0;			// zip code (String)
const int DRUG_MARKET_IDX = 1;		// Drug market ID (int)


void loadZones(const string& filename, map<std::string, Zone> & zonesMap) {
    SVReader reader(filename, ',');
    vector<string> line;

	// Header
	// Zipcode	Drug_Market
	
    // skip header
    reader.next(line);

    while (reader.next(line)) {
		string zip = line[ZIPCODE_IDX];
		unsigned int drugMarket = stoul(line[DRUG_MARKET_IDX]);
		
		Zone zone(zip);
		zone.setDrugMarket(drugMarket);

		zonesMap.emplace(zip, zone);
				
    }
}

void loadZonesDistances(const string& filename, map<std::string, Zone> & zonesMap,
		map<Zone, map<Zone,double>> & zoneDistanceMap) {
    SVReader reader(filename, ',');
    vector<std::string> line;

	// Header
	// Zipcode	then the actual zip codes ...
	
    // skip header
    reader.next(line);

    while (reader.next(line)) {
		string zip = line[0];
		
		auto pos = zonesMap.find(zip);
		if (pos == zonesMap.end()) {
			std::cout << "Zip not found: " << zip << std::endl;
		} 
		else {
			Zone zone = pos->second;

			map<Zone,double> distMap();
			zoneDistanceMap.emplace(zone, distMap);

			int rowlen = line.size();

			for (int i=1; i<rowlen; i++){

			}
		}
    }
}

}
