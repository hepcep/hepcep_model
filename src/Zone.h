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

#include "OpiodTreatmentDrug.h"

namespace hepcep {

enum class DistanceMetric {SHORTEST_DISTANCE, ON_FOOT, BY_CAR};

struct TreatmentScenario {
	// TODO for now just the probability of continued treatment.
	// If it remains just this double then abandon the struct.
	double prob_continued_treatment;
};

// enums are vector indices
using TreatmentScenarioMap = std::vector<std::vector<std::shared_ptr<TreatmentScenario>>>;

class Zone {

private:
	TreatmentScenarioMap treatment_map;

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

	std::shared_ptr<TreatmentScenario> getTreatmentScenario(DrugName drug_name, DistanceMetric metric);

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
