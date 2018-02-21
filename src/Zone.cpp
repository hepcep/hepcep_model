/*
 * Zone.cpp
 *
 * 
 */

#include "Zone.h"

namespace hepcep {

Zone::Zone(std::string zipcode){
	this->zipcode = zipcode;
}

Zone::Zone(std::string zipcode, double lat, double lon){
	this->zipcode = zipcode;
	this->lat = lat;
	this->lon = lon;
}

Zone::~Zone() {
}

void Zone::setDrugMarket(unsigned int drugMarket){
	this->drugMarket = drugMarket;
}

} /* namespace hepcep */
