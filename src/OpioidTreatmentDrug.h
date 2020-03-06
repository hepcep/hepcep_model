#ifndef SRC_OPIOIDTREATMENTDRUG_H
#define SRC_OPIOIDTREATMENTDRUG_H

#include <vector>
#include <map>
#include <memory>

#include "AreaType.h"

namespace hepcep {

enum class DrugName {METHADONE, NALTREXONE, BUPRENORPHINE};

struct DrugParams {
    DrugName name;
    double max_injection_intensity, duration;
    double urban_threshold, non_urban_threshold;
    double p_close, p_far;
};

class OpioidTreatmentDrug {

private:
    double max_injection_intensity_;
    double duration_;
    double thresholds[2];
    double p_close_, p_far_;

public:
    // ease_of_use is meant to capture that taking it once a month
    // is easier (and so more effective) that having to take it every day
    OpioidTreatmentDrug(const DrugParams& params);
    virtual ~OpioidTreatmentDrug() {}

    // This value becomes the injection intensity of the treated
    // person
    double maxInjectionIntensity() const;
    double duration() const;
    double getTreatmentThreshold(AreaType area_type) const;
    double getPClose() const;
    double getPFar() const;
    virtual DrugName name() const = 0;
};

class Methadone : public OpioidTreatmentDrug {

public:
    
    Methadone(const DrugParams& params);
    virtual ~Methadone() {}

    DrugName name() const override {
        return DrugName::METHADONE;
    }
};

class Naltrexone : public OpioidTreatmentDrug {

public:
    
    Naltrexone(const DrugParams& params);
    virtual ~Naltrexone() {}

    DrugName name() const override {
        return DrugName::NALTREXONE;
    }
};

class Buprenorphine : public OpioidTreatmentDrug {

public:
    
    Buprenorphine(const DrugParams& params);
    virtual ~Buprenorphine() {}

    DrugName name() const override {
        return DrugName::BUPRENORPHINE;
    }
};

class OpioidTreatmentDrugs {

private:
    static OpioidTreatmentDrugs* instance_;
    OpioidTreatmentDrugs();
    ~OpioidTreatmentDrugs();
    std::map<DrugName, std::shared_ptr<OpioidTreatmentDrug>> drugs;
    
public:
    static OpioidTreatmentDrugs* instance();
    static void initDrug(const DrugParams& params);

    std::shared_ptr<OpioidTreatmentDrug> getDrug(DrugName name);
};

extern std::vector<DrugName> DRUG_NAMES;


}

#endif