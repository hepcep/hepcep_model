/*
 * Zone.cpp
 *
 * 
 */

#include "Zone.h"

namespace hepcep {

Zone::Zone(unsigned int zipcode) : zipcode_(zipcode), treatment_map() {

}

Zone::Zone(unsigned int zipcode, double lat, double lon) : zipcode_(zipcode), lat_(lat), lon_(lon), treatment_map() {

}

Zone::~Zone() {
}

void Zone::setDrugMarket(unsigned int drugMarket){
	drugMarket_ = drugMarket;
}


std::shared_ptr<TreatmentScenario> Zone::getTreatmentScenario(DrugName drug_name, DistanceMetric metric) {
	return treatment_map[static_cast<int>(drug_name)][static_cast<int>(metric)];
}


} /* namespace hepcep */
