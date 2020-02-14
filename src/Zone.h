/*
 * Zone.h
 *
 *  
 */

#ifndef SRC_ZONE_H_
#define SRC_ZONE_H_

#include <memory>
#include <string>
#include <vector>
#include <map>

#include "OpioidTreatmentDrug.h"

namespace hepcep {

enum class DistanceMetric {WALKING, DRIVING};

class Zone {

private:
	std::map<DrugName, std::map<DistanceMetric, double>> journey_times;

protected:
	unsigned int zipcode_;
	unsigned int drugMarket_;
	double lat_;
	double lon_;

public:
	// TODO -- load the treatment scenarios for this zone as well
	Zone(unsigned int zipcode);
	Zone(unsigned int zipcode, double lat, double lon);

	virtual ~Zone();

	void setDrugMarket(unsigned int drugMarket);

	// TODO update with real data
	double getJourneyTime(DrugName drug_name, DistanceMetric metric);

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

	bool isUrban() const {
		// TODO update based on zipcode
		return false;
	}

};

using ZonePtr = std::shared_ptr<Zone>;

} /* namespace hepcep */


#endif /* SRC_ZONE_H_ */
