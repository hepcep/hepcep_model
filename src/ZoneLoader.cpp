/*
 * @file ZoneLoader.cpp
 * Loader for zone zip code list and zone-zone distances map.
 *
 * @author Eric Tatara
 */
#include <exception>

#include "ZoneLoader.h"
#include "SVReader.h"

namespace hepcep {

const int ZIPCODE_IDX = 0;			// zip code (String)
const int DRUG_MARKET_IDX = 1;		// Drug market ID (int)
const int LAT_INDEX = 2;				// Latitude
const int LON_INDEX = 3;				// Longitude

const int REAL_BUP_IDX = 1;
const int REAL_METH_IDX = 2;
const int REAL_NAL_IDX = 3;
const int S1_BUP_IDX = 4;
const int S1_METH_IDX = 5;
const int S1_NAL_IDX = 6;
const int S2_BUP_IDX = 7;
const int S2_METH_IDX = 8;
const int S2_NAL_IDX = 9;
const int S3_BUP_IDX = 10;
const int S3_METH_IDX = 11;
const int S3_NAL_IDX = 12;

struct MinDistance {
	double meth, bup, nal;
};


void loadOpioidTreatmentDistances(const std::string& treatment_dist_file, 
	const std::string& opioid_treatment_access_scenario, std::map<unsigned int, MinDistance>& map) 
{
	SVReader reader(treatment_dist_file, ',');
	std::vector<std::string> line;
	
	size_t idxs[] = {0, 0, 0};
	if (opioid_treatment_access_scenario == "REAL") {
		idxs[0] = REAL_METH_IDX;
		idxs[1] = REAL_BUP_IDX;
		idxs[2] = REAL_NAL_IDX;
	} else if (opioid_treatment_access_scenario == "SCENARIO_1") {
		idxs[0] = S1_METH_IDX;
		idxs[1] = S1_BUP_IDX;
		idxs[2] = S1_NAL_IDX;
	} else if (opioid_treatment_access_scenario == "SCENARIO_2") {
		idxs[0] = S2_METH_IDX;
		idxs[1] = S2_BUP_IDX;
		idxs[2] = S2_NAL_IDX;
    } else if (opioid_treatment_access_scenario == "SCENARIO_3") {
		idxs[0] = S3_METH_IDX;
		idxs[1] = S3_BUP_IDX;
		idxs[2] = S3_NAL_IDX;
	} else {
		throw std::invalid_argument("Invalid opioid_treatment_access_scenario: " + opioid_treatment_access_scenario);
	}
	
	
	// skip header
	reader.next(line);
	while (reader.next(line)) {
		unsigned int zip = std::stol(line[ZIPCODE_IDX]);
		MinDistance md;
		md.meth = std::stod(line[idxs[0]]);
		md.bup = std::stod(line[idxs[1]]);
		md.nal = std::stod(line[idxs[2]]);
		map.emplace(zip, md);
	}
}

/**
 * Creates a set of Zone instances and maps them to their zip code strings.
 */
void loadZones(const std::string& zones_file, const std::string& treatment_dist_file, 
	const std::string& opioid_treatment_access_scenario, std::map<unsigned int, ZonePtr> & zonesMap) {
	
	std::map<unsigned int, MinDistance> dist_map;
	loadOpioidTreatmentDistances(treatment_dist_file, opioid_treatment_access_scenario, dist_map);

	SVReader reader(zones_file, ',');
	std::vector<std::string> line;

	// Header
	// Zipcode	Drug_Market

	// skip header
	reader.next(line);

	while (reader.next(line)) {

		unsigned int zip = std::stoul(line[ZIPCODE_IDX]);
		unsigned int drugMarket = std::stoul(line[DRUG_MARKET_IDX]);
		double lat = std::stod(line[LAT_INDEX]);
		double lon = std::stod(line[LON_INDEX]);

		auto md = dist_map.at(zip);
		std::map<DrugName, double> zone_dist_map {
			{DrugName::METHADONE, md.meth},
			{DrugName::BUPRENORPHINE, md.bup},
			{DrugName::NALTREXONE, md.nal}
		};

		auto zone = std::make_shared<Zone>(zip,lat,lon, zone_dist_map);

		zone->setDrugMarket(drugMarket);

		zonesMap[zip] = zone;
	}
}

/**
 * Creates a map of zipcode to zipcode distances from the provided filename.
 */
void loadZonesDistances(const std::string& filename, std::map<unsigned int, ZonePtr> & zonesMap,
		std::unordered_map<unsigned int, std::unordered_map<unsigned int,double>> & zoneDistanceMap) {

	SVReader reader(filename, ',');
	std::vector<std::string> line;

	// Header
	// Zipcode	then the actual zip codes ...
	std::vector<std::string> header;

	reader.next(header);

	// Each row represents the source zip to which the column target zips are mapped.
	while (reader.next(line)) {
		unsigned int sourceZip = std::stoul(line[0]);  // First zip is the source zip

		// This map holds the distance to every other zip column.
		std::unordered_map<unsigned int, double> distMap;

		int rowlen = line.size();
		for (int i=1; i<rowlen; i++){            // start i=1
			unsigned int targetZip = std::stoul(header[i]);
			distMap[targetZip] = std::stod(line[i]);
		}
		zoneDistanceMap[sourceZip] = distMap;
	}
}

}
