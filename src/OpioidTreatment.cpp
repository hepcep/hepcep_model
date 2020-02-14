#include <memory>

#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/Schedule.h"

#include "OpioidTreatment.h"
#include "OpioidTreatmentDrug.h"
#include "Zone.h"

namespace hepcep {

class NoTreatment : public OpioidTreatmentImpl {

public:
    NoTreatment() : OpioidTreatmentImpl() {}
    virtual ~NoTreatment() {}

    bool run() override {
        return false;
    }

    bool inTreatment() override {
        return false;
    }
};

class DrugTreatment : public OpioidTreatmentImpl {

private:
    std::shared_ptr<OpioidTreatmentDrug> drug_;
    std::shared_ptr<TreatmentScenario> scenario_;

public:
    DrugTreatment(std::shared_ptr<OpioidTreatmentDrug> drug, std::shared_ptr<TreatmentScenario> scenario);
    virtual ~DrugTreatment() {}

    bool run() override;

    bool inTreatment() override {
        return true;
    }
};

bool DrugTreatment::run() {
    // TODO some combination of drug and scenario effectiveness
    return false;
}

DrugTreatment::DrugTreatment(std::shared_ptr<OpioidTreatmentDrug> drug, std::shared_ptr<TreatmentScenario> scenario) : 
    drug_{drug}, scenario_{scenario} {}


OpioidTreatment::OpioidTreatment() : treatment{new NoTreatment()} {}

OpioidTreatment::~OpioidTreatment() {
    delete treatment;
}

void OpioidTreatment::reset() {
    delete treatment;
    treatment = new NoTreatment();
}

bool OpioidTreatment::inTreatment() const {
    return treatment->inTreatment();
}

void OpioidTreatment::reset(std::shared_ptr<OpioidTreatmentDrug> drug, std::shared_ptr<TreatmentScenario> scenario, 
    double duration) {

    delete treatment;
    treatment = new DrugTreatment(drug, scenario);

    repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
    double tick = runner.currentTick();
    runner.scheduleEvent(tick + duration, repast::Schedule::FunctorPtr(new repast::MethodFunctor<OpioidTreatment>(this, &OpioidTreatment::reset)));

}


}
