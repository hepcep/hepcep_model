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
	unsigned int zipcode_;
	unsigned int drugMarket_;
	double lat_;
	double lon_;

public:
	Zone(unsigned int zipcode);
	Zone(unsigned int zipcode, double lat, double lon);

	virtual ~Zone();

	void setDrugMarket(unsigned int drugMarket);

	unsigned int getDrugMarket() const {
		return drugMarket_;
	}

	unsigned int getZipcode() const {
		return zipcode_;
	}

	double getLat() const {
		return lat_;
	}

	double getLon() const {
		return lon_;
	}

};

using ZonePtr = std::shared_ptr<Zone>;

} /* namespace hepcep */


#endif /* SRC_ZONE_H_ */
