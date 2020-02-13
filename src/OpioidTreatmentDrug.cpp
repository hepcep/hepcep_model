#include "OpioidTreatmentDrug.h"

namespace hepcep {


OpioidTreatmentDrug::OpioidTreatmentDrug(double effectiveness, double ease_of_use) : effectiveness_{effectiveness}, 
    ease_of_use_{ease_of_use} {}

double OpioidTreatmentDrug::effectiveness() {
    return 1;
}

Methadone::Methadone(double effectiveness, double ease_of_use) : OpioidTreatmentDrug(effectiveness, ease_of_use) {}

Naltrexone::Naltrexone(double effectiveness, double ease_of_use) : OpioidTreatmentDrug(effectiveness, ease_of_use) {}

Buprenorphine::Buprenorphine(double effectiveness, double ease_of_use) : OpioidTreatmentDrug(effectiveness, ease_of_use) {}


}