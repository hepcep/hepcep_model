/*
 * Zone.h
 *
 *  
 */

#ifndef SRC_ZONE_H_
#define SRC_ZONE_H_

#include <string>

using namespace std;

namespace hepcep {

class Zone {

protected:
	string zipcode;
	unsigned int drugMarket;
	
public:
    Zone(string zipcode);
	
    virtual ~Zone();

	unsigned int getDrugMarket();
	void setDrugMarket(unsigned int drugMarket);
   
};

} /* namespace hepcep */

#endif /* SRC_ZONE_H_ */
