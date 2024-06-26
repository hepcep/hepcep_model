/*
 * @file APK_Immunology.h
 *
 * Immunology class using the original APK infection mechanics.
 *
 */

#ifndef SRC_APK_IMMUNOLOGY_H_
#define SRC_APKIMMUNOLOGY_H_

#include "Immunology.h"

namespace hepcep {

struct ImmunologyParameters {

     double mean_days_acute_naive,
		 mean_days_acute_rechallenged,
         mean_days_naive_to_infectious,
		 mean_days_residual_hcv_infectivity,
         prob_self_limiting_female,
		 prob_self_limiting_male,
         prob_clearing,
		 transmissibility,
         treatment_duration,
		 treatment_svr,
         treatment_susceptibility;
     
     ImmunologyParameters();
};

using IPPtr = std::shared_ptr<ImmunologyParameters>;

class APK_Immunology : public Immunology {

private:
    friend void write_immunology(std::shared_ptr<Immunology>, AttributeWriter&, double);
    friend void read_immunology(NamedListAttribute*, std::shared_ptr<Immunology>, HCPerson*, double);

    IPPtr params_;
    
    bool isInTreatmentViralSuppression(double tick);


public:
    APK_Immunology(HCPerson* idu);
//    APK_Immunology(HCPerson* idu, IPPtr params);
//    APK_Immunology(HCPerson* idu, HCVState alter_state, IPPtr params);
    
    virtual ~APK_Immunology();

    bool exposePartner(std::shared_ptr<Immunology> partner_imm, double tick) override;
    bool receiveInfectiousDose(double tick) override;

    void deactivate() override;
    void leaveExposed() override;
    bool leaveAcute() override;

    bool isHcvRNA(double now) override;
    // bool isInfectious(double now) override;

    double get_transmissibility() override;
    double get_viral_load() override;

    bool getTestedHCV(double now) override;

    void setHCVInitState(double now, HCVState state, int logging) override;

    void leaveTreatment(bool treatment_succeeded) override;
    void startTreatment(bool adherent, double now) override;

    void purgeActions() override;

    void step() override;

    VKProfile getVKProfile() override;
    
};


} /* namespace hepcep */

#endif /* SRC_APKIMMUNOLOGY_H_ */
