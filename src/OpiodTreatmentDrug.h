#ifndef SRC_OPIODTREATMENTDRUG_H
#define SRC_OPIODTREATMENTDRUG_H

namespace hepcep {

enum class DrugName {METHADONE, NALTREXONE, BUPRENORPHINE};

class OpiodTreatmentDrug {

private:
    double effectiveness_, ease_of_use_;

public:
    // ease_of_use is meant to capture that taking it once a month
    // is easier (and so more effective) that having to take it every day
    OpiodTreatmentDrug(double effectiveness, double ease_of_use);
    virtual ~OpiodTreatmentDrug() {}

    // TODO: Some combination of effectiveness and easy of use
    // that applies to all drugs, or make virtual if difference is
    // not parameterizable.
    double effectiveness();
    virtual DrugName name() const = 0;
};

class Methadone : public OpiodTreatmentDrug {

public:
    
    Methadone(double effectiveness, double ease_of_use);
    virtual ~Methadone() {}

    DrugName name() const override {
        return DrugName::METHADONE;
    }
};

class Naltrexone : public OpiodTreatmentDrug {

public:
    
    Naltrexone(double effectiveness, double ease_of_use);
    virtual ~Naltrexone() {}

    DrugName name() const override {
        return DrugName::NALTREXONE;
    }
};

class Buprenorphine : public OpiodTreatmentDrug {

public:
    
    Buprenorphine(double effectiveness, double ease_of_use);
    virtual ~Buprenorphine() {}

    DrugName name() const override {
        return DrugName::BUPRENORPHINE;
    }
};


}

#endif