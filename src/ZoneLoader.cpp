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


void loadZones(const std::string& filename, std::map<std::string, ZonePtr> & zonesMap) {
    SVReader reader(filename, ',');
    std::vector<std::string> line;

	// Header
	// Zipcode	Drug_Market
	
    // skip header
    reader.next(line);

    while (reader.next(line)) {

    	std::string zip = line[ZIPCODE_IDX];
    	unsigned int drugMarket = std::stoul(line[DRUG_MARKET_IDX]);

    	auto zone = std::make_shared<Zone>(zip);

    	zone->setDrugMarket(drugMarket);

    	std::cout << "A_" << zip << "_B  :" << drugMarket << std::endl;

    	zonesMap[zip] = zone;

    }
}

void loadZonesDistances(const std::string& filename, std::map<std::string, ZonePtr> & zonesMap,
		std::map<ZonePtr, std::map<ZonePtr,double>> & zoneDistanceMap) {
    SVReader reader(filename, ',');
    std::vector<std::string> line;

	// Header
	// Zipcode	then the actual zip codes ...
	
    // skip header
    reader.next(line);

    while (reader.next(line)) {
    std::string zip = line[0];
		
		auto pos = zonesMap.find(zip);
		if (pos == zonesMap.end()) {
			std::cout << "Zip not found: " << zip << std::endl;
		} 
		else {
			ZonePtr zone = pos->second;

			std::map<ZonePtr, double> distMap();
//			zoneDistanceMap.emplace(zone, distMap);

//			int rowlen = line.size();

//			for (int i=1; i<rowlen; i++){
//
//			}
		}
    }
}

}
