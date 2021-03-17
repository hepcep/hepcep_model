#include "OpioidTreatmentDrug.h"

namespace hepcep {

std::vector<DrugName> DRUG_NAMES{DrugName::METHADONE, DrugName::NALTREXONE, DrugName::BUPRENORPHINE};

OpioidTreatmentDrug::OpioidTreatmentDrug(const DrugParams& params) : max_injection_intensity_{params.max_injection_intensity}, duration_{params.duration}, 
    thresholds{0, 0}, p_close_{params.p_close}, p_far_{params.p_far} 
{
        thresholds[AreaType::Value::City] = params.urban_threshold;
        thresholds[AreaType::Value::Suburban] = params.non_urban_threshold;
        max_thresholds[AreaType::Value::City] = params.urban_max_threshold;
        max_thresholds[AreaType::Value::Suburban] = params.non_urban_max_threshold;
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

double OpioidTreatmentDrug::getMaxTreatmentThreshold(AreaType area_type) const {
    return max_thresholds[area_type.value()];
}

double OpioidTreatmentDrug::getPClose() const {
    return p_close_;
}

double OpioidTreatmentDrug::getPFar() const {
    return p_far_;
}

Methadone::Methadone(const DrugParams& params) : 
    OpioidTreatmentDrug(params) {}

Naltrexone::Naltrexone(const DrugParams& params) : 
    OpioidTreatmentDrug(params) {}

Buprenorphine::Buprenorphine(const DrugParams& params) : 
    OpioidTreatmentDrug(params) {}

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

void OpioidTreatmentDrugs::initDrug(const DrugParams& params) {
    if (!instance_) {
        instance_ = new OpioidTreatmentDrugs();
    }

    if (params.name == DrugName::BUPRENORPHINE) {
        instance_->drugs[params.name] = std::make_shared<Buprenorphine>(params);
    } else if (params.name == DrugName::METHADONE) {
        instance_->drugs[params.name] = std::make_shared<Methadone>(params);
    } else if (params.name == DrugName::NALTREXONE) {
        instance_->drugs[params.name] = std::make_shared<Naltrexone>(params);
    }
}

OpioidTreatmentDrugs::OpioidTreatmentDrugs() : drugs() {}
OpioidTreatmentDrugs::~OpioidTreatmentDrugs() {}

std::shared_ptr<OpioidTreatmentDrug> OpioidTreatmentDrugs::getDrug(DrugName name) {
    return drugs[name];
}

}