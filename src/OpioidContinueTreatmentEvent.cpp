#include "OpioidContinueTreatmentEvent.h"

#include "repast_hpc/RepastProcess.h"

namespace hepcep {

OpioidContinueTreatmentEvent::OpioidContinueTreatmentEvent(std::shared_ptr<HCPerson> person,
     std::shared_ptr<OpioidTreatment> treatment) : person_(person), treatment_(treatment) {}

OpioidContinueTreatmentEvent::~OpioidContinueTreatmentEvent() {}

void OpioidContinueTreatmentEvent::operator()() {
    if (person_->isActive()) {
        bool success = treatment_->treat(person_);
        if (success) {
            repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
            // -0.0001 so goes on / off before regularly scheduled actions
            double at = runner.currentTick() + treatment_->duration() - 0.0001;
            runner.scheduleEvent(at, boost::make_shared<OpioidContinueTreatmentEvent>(person_, treatment_));
        }
    }
}

}