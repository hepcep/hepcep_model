/*
 * Zone.cpp
 *
 * 
 */

#include "Zone.h"

namespace hepcep {

//Zone::Zone(unsigned int zipcode) : zipcode_(zipcode) {}

Zone::Zone(unsigned int zipcode, double lat, double lon, std::map<DrugName, double>& dist_map) : 
	distance_to_treatment(dist_map), zipcode_(zipcode), lat_(lat), lon_(lon)
{

}

Zone::~Zone() {
}

void Zone::setDrugMarket(unsigned int drugMarket){
	drugMarket_ = drugMarket;
}

// TODO update when we have the real data
double Zone::getDistanceToTreatment(DrugName drug_name) {
	return distance_to_treatment[drug_name];
}


} /* namespace hepcep */
