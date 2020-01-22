#include <memory>

#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/Schedule.h"

#include "OpiodTreatment.h"
#include "OpiodTreatmentDrug.h"
#include "Zone.h"

namespace hepcep {

class NoTreatment : public OpiodTreatmentImpl {

public:
    NoTreatment() : OpiodTreatmentImpl() {}
    virtual ~NoTreatment() {}

    bool run() override {
        return false;
    }

    bool inTreatment() override {
        return false;
    }
};

class DrugTreatment : public OpiodTreatmentImpl {

private:
    std::shared_ptr<OpiodTreatmentDrug> drug_;
    std::shared_ptr<TreatmentScenario> scenario_;

public:
    DrugTreatment(std::shared_ptr<OpiodTreatmentDrug> drug, std::shared_ptr<TreatmentScenario> scenario);
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

DrugTreatment::DrugTreatment(std::shared_ptr<OpiodTreatmentDrug> drug, std::shared_ptr<TreatmentScenario> scenario) : 
    drug_{drug}, scenario_{scenario} {}


OpiodTreatment::OpiodTreatment() : treatment{new NoTreatment()} {}

OpiodTreatment::~OpiodTreatment() {
    delete treatment;
}

void OpiodTreatment::reset() {
    delete treatment;
    treatment = new NoTreatment();
}

bool OpiodTreatment::inTreatment() {
    return treatment->inTreatment();
}

void OpiodTreatment::reset(std::shared_ptr<OpiodTreatmentDrug> drug, std::shared_ptr<TreatmentScenario> scenario, 
    double duration) {

    delete treatment;
    treatment = new DrugTreatment(drug, scenario);

    repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
    double tick = runner.currentTick();
    runner.scheduleEvent(tick + duration, repast::Schedule::FunctorPtr(new repast::MethodFunctor<OpiodTreatment>(this, &OpiodTreatment::reset)));

}


}
