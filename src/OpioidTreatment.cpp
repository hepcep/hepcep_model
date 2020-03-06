#include <memory>

#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/Schedule.h"

#include "OpioidTreatment.h"
#include "HCPerson.h"

namespace hepcep {

OpioidTreatment::OpioidTreatment(ZonePtr zone, std::shared_ptr<OpioidTreatmentDrug> drug) : 
    treatment_prob(0), drug_(drug)
{
    double distance = zone->getDistanceToTreatment(drug_->name());
    if (distance > drug_->getTreatmentThreshold(AreaType::getAreaType(zone->getZipcode()))) {
        treatment_prob = drug_->getPFar();
    } else {
        treatment_prob = drug->getPClose();
    }
}

OpioidTreatment::~OpioidTreatment() {}

bool OpioidTreatment::treat(std::shared_ptr<HCPerson> person) {
    if (repast::Random::instance()->nextDouble() <= treatment_prob) {
        repast::DoubleUniformGenerator gen = 
            repast::Random::instance()->createUniDoubleGenerator(0, drug_->maxInjectionIntensity());
        person->setInjectionIntensityMultiplier(gen.next());
        person->setInOpioidTreatment(true);
        
        person->setCurrentOpioidTreatmentDrug(drug_->name());
        
        return true;
    }
    person->setInOpioidTreatment(false);
    person->setInjectionIntensityMultiplier(1);
    return false;
}

}
