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
	double lat;
	double lon;

public:
	Zone(std::string zipcode);
	Zone(std::string zipcode, double lat, double lon);

	virtual ~Zone();

	void setDrugMarket(unsigned int drugMarket);

	unsigned int getDrugMarket() const {
		return drugMarket;
	}

	std::string getZipcode() const {
		return zipcode;
	}

	double getLat() const {
		return lat;
	}

	double getLon() const {
		return lon;
	}

};

using ZonePtr = std::shared_ptr<Zone>;

} /* namespace hepcep */


#endif /* SRC_ZONE_H_ */
