/*
 * @file ZoneLoader.cpp
 * Loader for zone zip code list and zone-zone distances map.
 *
 * @author Eric Tatara
 */

#include "ZoneLoader.h"
#include "SVReader.h"

namespace hepcep {

const int ZIPCODE_IDX = 0;			// zip code (String)
const int DRUG_MARKET_IDX = 1;		// Drug market ID (int)

/**
 * Creates a set of Zone instances and maps them to their zip code strings.
 */
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

		zonesMap[zip] = zone;
	}
}

/**
 * Creates a map of zipcode to zipcode distances from the provided filename.
 */
void loadZonesDistances(const std::string& filename, std::map<std::string, ZonePtr> & zonesMap,
		std::map<std::string, std::map<std::string,double>> & zoneDistanceMap) {

	SVReader reader(filename, ',');
	std::vector<std::string> line;

	// Header
	// Zipcode	then the actual zip codes ...
	std::vector<std::string> header;

	reader.next(header);

	// Each row represents the source zip to which the column target zips are mapped.
	while (reader.next(line)) {
		std::string sourceZip = line[0];  // First zip is the source zip

		// This map holds the distance to every other zip column.
		std::map<std::string, double> distMap;

		int rowlen = line.size();
		for (int i=1; i<rowlen; i++){            // start i=1
			std::string targetZip = header[i];
			distMap[targetZip] = std::stod(line[i]);
		}
		zoneDistanceMap[sourceZip] = distMap;
	}
}

}
