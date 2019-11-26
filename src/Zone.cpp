/*
 * Zone.cpp
 *
 * 
 */

#include "Zone.h"

namespace hepcep {

Zone::Zone(unsigned int zipcode) : zipcode_(zipcode) {

}

Zone::Zone(unsigned int zipcode, double lat, double lon) : zipcode_(zipcode), lat_(lat), lon_(lon) {

}

Zone::~Zone() {
}

void Zone::setDrugMarket(unsigned int drugMarket){
	drugMarket_ = drugMarket;
}

} /* namespace hepcep */
