#include "OpiodTreatmentDrug.h"

namespace hepcep {


OpiodTreatmentDrug::OpiodTreatmentDrug(double effectiveness, double ease_of_use) : effectiveness_{effectiveness}, 
    ease_of_use_{ease_of_use} {}

double OpiodTreatmentDrug::effectiveness() {
    return 1;
}

Methadone::Methadone(double effectiveness, double ease_of_use) : OpiodTreatmentDrug(effectiveness, ease_of_use) {}

Naltrexone::Naltrexone(double effectiveness, double ease_of_use) : OpiodTreatmentDrug(effectiveness, ease_of_use) {}

Buprenorphine::Buprenorphine(double effectiveness, double ease_of_use) : OpiodTreatmentDrug(effectiveness, ease_of_use) {}


}