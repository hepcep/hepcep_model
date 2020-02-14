#include <memory>

#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/Schedule.h"

#include "OpioidTreatment.h"
#include "HCPerson.h"

namespace hepcep {

OpioidTreatment::OpioidTreatment(ZonePtr zone, std::shared_ptr<OpioidTreatmentDrug> drug) : 
    treatment_prob(0), drug_(drug)
{
  
    DistanceMetric metric = zone->isUrban() ? DistanceMetric::WALKING : DistanceMetric::DRIVING;
    // TODO given that zone determines urban or not, update journey time not to take DistanceMetric
    double journey_time = zone->getJourneyTime(drug_->name(), metric);

    // TODO need real thresholds and probabilities
    if (journey_time > drug_->journeyTimeThreshold()) {
        treatment_prob = 0.3;
    } else{
        treatment_prob = 0.7;
    }
}

OpioidTreatment::~OpioidTreatment() {}

bool OpioidTreatment::treat(std::shared_ptr<HCPerson> person) {
    if (repast::Random::instance()->nextDouble() <= treatment_prob) {
        repast::DoubleUniformGenerator gen = 
            repast::Random::instance()->createUniDoubleGenerator(0, drug_->effectiveness());
        person->setInjectionIntensityMultiplier(gen.next());
        person->setInOpioidTreatment(true);
        return true;
    }

    person->setInOpioidTreatment(false);
    person->setInjectionIntensityMultiplier(1);
    return false;
}

}
