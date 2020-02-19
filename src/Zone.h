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

class Zone {

private:
	std::map<DrugName, double> distance_to_treatment;

protected:
	unsigned int zipcode_;
	unsigned int drugMarket_;
	double lat_;
	double lon_;

public:
	// TODO -- load the treatment scenarios for this zone as well
	//Zone(unsigned int zipcode);
	Zone(unsigned int zipcode, double lat, double lon, std::map<DrugName, double>& dist_map);

	virtual ~Zone();

	void setDrugMarket(unsigned int drugMarket);

	double getDistanceToTreatment(DrugName drug_name);

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
