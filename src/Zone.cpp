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

Zone::~Zone() {
}

unsigned int Zone::getDrugMarket(){
	return drugMarket;
}

void Zone::setDrugMarket(unsigned int drugMarket){
	this->drugMarket = drugMarket;
}

} /* namespace hepcep */
