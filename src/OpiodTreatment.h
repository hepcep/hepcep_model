#ifndef SRC_OPIODTREATMENT_H
#define SRC_OPIODTREATMENT_H

#include "OpiodTreatmentDrug.h"
#include "Zone.h"

namespace hepcep {

class OpiodTreatmentImpl {

public:
    OpiodTreatmentImpl() {}
    virtual ~OpiodTreatmentImpl() {}

    virtual bool run() = 0;
    virtual bool inTreatment() = 0;

};

class OpiodTreatment {

private:
    OpiodTreatmentImpl* treatment;


public:
    OpiodTreatment();
    virtual ~OpiodTreatment();

    /**
     * Whether or not injects based on treatment.
     */
    bool run();
    bool inTreatment();
    void reset();
    void reset(std::shared_ptr<OpiodTreatmentDrug> drug, std::shared_ptr<TreatmentScenario> scenario, 
        double duration);


};

}

#endif