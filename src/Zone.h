/*
 * Zone.h
 *
 *  
 */

#ifndef SRC_ZONE_H_
#define SRC_ZONE_H_

#include <memory>
#include <string>

namespace hepcep {

class Zone {

protected:
	std::string zipcode;
	unsigned int drugMarket;

public:
	Zone(std::string zipcode);

	virtual ~Zone();

	unsigned int getDrugMarket();
	void setDrugMarket(unsigned int drugMarket);

	std::string getZipcode(){
		return zipcode;
	}

};

using ZonePtr = std::shared_ptr<Zone>;

} /* namespace hepcep */


#endif /* SRC_ZONE_H_ */
