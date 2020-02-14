#ifndef SRC_OPIOIDCONTINUETREATMENTEVENT_H
#define SRC_OPIOIDCONTINUETREATMENTEVENT_H

#include "repast_hpc/Schedule.h"

#include "HCPerson.h"
#include "OpioidTreatment.h"

namespace hepcep {

class OpioidContinueTreatmentEvent : public repast::Functor {

private:
    std::shared_ptr<HCPerson> person_;
    std::shared_ptr<OpioidTreatment> treatment_;

public:
    OpioidContinueTreatmentEvent(std::shared_ptr<HCPerson> person, std::shared_ptr<OpioidTreatment> treatment);
    virtual ~OpioidContinueTreatmentEvent();

    void operator()();
};

}

#endif