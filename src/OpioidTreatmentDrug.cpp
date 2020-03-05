#include "OpioidTreatmentDrug.h"

namespace hepcep {

std::vector<DrugName> DRUG_NAMES{DrugName::METHADONE, DrugName::NALTREXONE, DrugName::BUPRENORPHINE};

OpioidTreatmentDrug::OpioidTreatmentDrug(double max_injection_intensity, double duration, 
    double urban_threshold, double non_urban_threshold) : max_injection_intensity_{max_injection_intensity}, duration_{duration}, thresholds{0, 0} {
        thresholds[AreaType::Value::City] = urban_threshold;
        thresholds[AreaType::Value::Suburban] = non_urban_threshold;
}

double OpioidTreatmentDrug::maxInjectionIntensity() const {
    return max_injection_intensity_;
}

double OpioidTreatmentDrug::duration() const {
    return duration_;
}

double OpioidTreatmentDrug::getTreatmentThreshold(AreaType area_type) const {
    return thresholds[area_type.value()];
}

Methadone::Methadone(double max_injection_intensity, double duration, double urban_threshold, double non_urban_threshold) : 
    OpioidTreatmentDrug(max_injection_intensity, duration, urban_threshold, non_urban_threshold) {}

Naltrexone::Naltrexone(double max_injection_intensity, double duration, double urban_threshold, double non_urban_threshold) : 
    OpioidTreatmentDrug(max_injection_intensity, duration, urban_threshold, non_urban_threshold) {}

Buprenorphine::Buprenorphine(double max_injection_intensity, double duration, double urban_threshold, double non_urban_threshold) : 
    OpioidTreatmentDrug(max_injection_intensity, duration, urban_threshold, non_urban_threshold) {}

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

void OpioidTreatmentDrugs::initDrug(DrugName name, double max_injection_intensity, double duration, double urban_threshold, double non_urban_threshold) {
    if (!instance_) {
        instance_ = new OpioidTreatmentDrugs();
    }

    if (name == DrugName::BUPRENORPHINE) {
        instance_->drugs[name] = std::make_shared<Buprenorphine>(max_injection_intensity, duration, urban_threshold, non_urban_threshold);
    } else if (name == DrugName::METHADONE) {
        instance_->drugs[name] = std::make_shared<Methadone>(max_injection_intensity, duration, urban_threshold, non_urban_threshold);
    } else if (name == DrugName::NALTREXONE) {
        instance_->drugs[name] = std::make_shared<Naltrexone>(max_injection_intensity, duration, urban_threshold, non_urban_threshold);
    }
}

OpioidTreatmentDrugs::OpioidTreatmentDrugs() : drugs() {}
OpioidTreatmentDrugs::~OpioidTreatmentDrugs() {}

std::shared_ptr<OpioidTreatmentDrug> OpioidTreatmentDrugs::getDrug(DrugName name) {
    return drugs[name];
}

}