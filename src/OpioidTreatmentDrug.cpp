#include "OpioidTreatmentDrug.h"

namespace hepcep {

std::vector<DrugName> DRUG_NAMES{DrugName::METHADONE, DrugName::NALTREXONE, DrugName::BUPRENORPHINE};

OpioidTreatmentDrug::OpioidTreatmentDrug(double effectiveness, double duration, 
    double journey_time_threshold) : effectiveness_{effectiveness}, duration_{duration}, 
    journey_time_threshold_(journey_time_threshold) {}

double OpioidTreatmentDrug::effectiveness() const {
    return effectiveness_;
}

double OpioidTreatmentDrug::duration() const {
    return duration_;
}

double OpioidTreatmentDrug::journeyTimeThreshold() const {
    return journey_time_threshold_;
}

Methadone::Methadone(double effectiveness, double duration, double journey_time_threshold) : 
    OpioidTreatmentDrug(effectiveness, duration, journey_time_threshold) {}

Naltrexone::Naltrexone(double effectiveness, double duration, double journey_time_threshold) : 
    OpioidTreatmentDrug(effectiveness, duration, journey_time_threshold) {}

Buprenorphine::Buprenorphine(double effectiveness, double duration, double journey_time_threshold) : 
    OpioidTreatmentDrug(effectiveness, duration, journey_time_threshold) {}

OpioidTreatmentDrugs* OpioidTreatmentDrugs::instance_ = nullptr;

OpioidTreatmentDrugs* OpioidTreatmentDrugs::instance() {
    if (!instance_) {
        throw std::domain_error("OpioidTreatmentDrugs must be with all drugs before use");
    } 

    for (DrugName name : DRUG_NAMES) {
        if (instance_->drugs.find(name) == instance_->drugs.end()) {
            delete instance_;
            throw std::domain_error("OpioidTreatmentDrugs must be with all drugs before use");
        }
    }
    return instance_;
}

void OpioidTreatmentDrugs::initDrug(DrugName name, double effectiveness, double duration, double journey_time_threshold) {
    if (!instance_) {
        instance_ = new OpioidTreatmentDrugs();
    }

    if (name == DrugName::BUPRENORPHINE) {
        instance_->drugs[name] = std::make_shared<Buprenorphine>(effectiveness, duration, journey_time_threshold);
    } else if (name == DrugName::METHADONE) {
        instance_->drugs[name] = std::make_shared<Methadone>(effectiveness, duration, journey_time_threshold);
    } else if (name == DrugName::NALTREXONE) {
        instance_->drugs[name] = std::make_shared<Naltrexone>(effectiveness, duration, journey_time_threshold);
    }
}

OpioidTreatmentDrugs::OpioidTreatmentDrugs() : drugs() {}
OpioidTreatmentDrugs::~OpioidTreatmentDrugs() {}

std::shared_ptr<OpioidTreatmentDrug> OpioidTreatmentDrugs::getDrug(DrugName name) {
    return drugs[name];
}

}