#ifndef SRC_OPIOIDTREATMENT_H
#define SRC_OPIOIDTREATMENT_H

#include "OpioidTreatmentDrug.h"
#include "Zone.h"

namespace hepcep {

class OpioidTreatmentImpl {

public:
    OpioidTreatmentImpl() {}
    virtual ~OpioidTreatmentImpl() {}

    virtual bool run() = 0;
    virtual bool inTreatment() = 0;

};

class OpioidTreatment {

private:
    OpioidTreatmentImpl* treatment;


public:
    OpioidTreatment();
    virtual ~OpioidTreatment();

    /**
     * Whether or not injects based on treatment.
     */
    bool run();
    bool inTreatment();
    void reset();
    void reset(std::shared_ptr<OpioidTreatmentDrug> drug, std::shared_ptr<TreatmentScenario> scenario, 
        double duration);


};

}

#endif