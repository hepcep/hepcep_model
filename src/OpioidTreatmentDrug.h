#ifndef SRC_OPIOIDTREATMENTDRUG_H
#define SRC_OPIOIDTREATMENTDRUG_H

#include <vector>
#include <map>
#include <memory>

namespace hepcep {

enum class DrugName {METHADONE, NALTREXONE, BUPRENORPHINE};

class OpioidTreatmentDrug {

private:
    double effectiveness_;
    double duration_;
    double journey_time_threshold_;

public:
    // ease_of_use is meant to capture that taking it once a month
    // is easier (and so more effective) that having to take it every day
    OpioidTreatmentDrug(double effectiveness, double duration, double journey_time_threshold);
    virtual ~OpioidTreatmentDrug() {}

    // This value becomes the injection intensity of the treated
    // person
    double effectiveness() const;
    double duration() const;
    double journeyTimeThreshold() const;
    virtual DrugName name() const = 0;
};

class Methadone : public OpioidTreatmentDrug {

public:
    
    Methadone(double effectiveness, double ease_of_use, double journey_time_threshold);
    virtual ~Methadone() {}

    DrugName name() const override {
        return DrugName::METHADONE;
    }
};

class Naltrexone : public OpioidTreatmentDrug {

public:
    
    Naltrexone(double effectiveness, double duration, double journey_time_threshold);
    virtual ~Naltrexone() {}

    DrugName name() const override {
        return DrugName::NALTREXONE;
    }
};

class Buprenorphine : public OpioidTreatmentDrug {

public:
    
    Buprenorphine(double effectiveness, double duration, double journey_time_threshold);
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
    static void initDrug(DrugName name, double effectiveness, double duration, double journey_time_threshold);

    std::shared_ptr<OpioidTreatmentDrug> getDrug(DrugName name);
};

extern std::vector<DrugName> DRUG_NAMES;


}

#endif