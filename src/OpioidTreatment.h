#ifndef SRC_OPIOIDTREATMENT_H
#define SRC_OPIOIDTREATMENT_H

#include <memory>

#include "OpioidTreatmentDrug.h"
#include "Zone.h"


namespace hepcep {

class HCPerson;

class OpioidTreatment {

private:
    double treatment_prob;
    std::shared_ptr<OpioidTreatmentDrug> drug_;

public:

    OpioidTreatment(ZonePtr zone, std::shared_ptr<OpioidTreatmentDrug> drug);
    virtual ~OpioidTreatment();

    bool treat(std::shared_ptr<HCPerson> person);
    double duration() const {
        return drug_->duration();
    }
    
    std::string drugLabel() const {
        return drug_->label();
    }
};

}

#endif