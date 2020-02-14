/*
 * Zone.cpp
 *
 * 
 */

#include "Zone.h"

namespace hepcep {

Zone::Zone(unsigned int zipcode) : journey_times(), zipcode_(zipcode) {
	// TODO replace with  the real data
	for (DrugName name : DRUG_NAMES) {
		if (name == DrugName::METHADONE) {
			std::map<DistanceMetric, double> m = { {DistanceMetric::DRIVING, 30}, 
												   {DistanceMetric::WALKING, 30}
												 };
			journey_times.emplace(name, m);
		} else {
			std::map<DistanceMetric, double> m = { {DistanceMetric::DRIVING, 60}, 
												   {DistanceMetric::WALKING, 60}
												 };
			journey_times.emplace(name, m);
		}
	}
}

Zone::Zone(unsigned int zipcode, double lat, double lon) : journey_times(), zipcode_(zipcode), lat_(lat), lon_(lon) {

}

Zone::~Zone() {
}

void Zone::setDrugMarket(unsigned int drugMarket){
	drugMarket_ = drugMarket;
}

// TODO update when we have the real data
double Zone::getJourneyTime(DrugName drug_name, DistanceMetric metric) {
	return journey_times[drug_name][metric];
}


} /* namespace hepcep */
