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
            double at = runner.currentTick() + treatment_->duration();
            runner.scheduleEvent(at, boost::make_shared<OpioidContinueTreatmentEvent>(person_, treatment_));
            
            // TODO log continue treatment info?
        }
        else {
            Statistics::instance()->logStatusChange(LogType::STOPPED_OPIOID_TREATMENT, person_, treatment_->drugLabel());
        }
    }
}

}