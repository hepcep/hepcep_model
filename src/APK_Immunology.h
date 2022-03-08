/*
 * Immunology.h
 *
 *  Created on: Mar 1, 2018
 *      Author: nick
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

     bool treatment_repeatable;

     // Max number of times that can be re-enrolled in DAA treatment 
     unsigned int max_num_daa_treatments;  
     
     ImmunologyParameters();
};

using IPPtr = std::shared_ptr<ImmunologyParameters>;

class APK_Immunology : public Immunology {

private:
    friend void write_immunology(std::shared_ptr<Immunology>, AttributeWriter&, double);
    friend void read_immunology(NamedListAttribute*, std::shared_ptr<Immunology>, HCPerson*, double);

    IPPtr params_;

    HCVState hcv_state;
    std::vector<boost::shared_ptr<Event>> scheduled_actions;

    bool past_cured, past_recovered, in_treatment;
    double treatment_start_date;
    bool treatment_failed;  // indicates a prior treatment attempt has failed

    unsigned int num_daa_treatments = 0;   // Number of times person has enrolled in DAA treatment
    
    bool isInTreatmentViralSuppression(double tick);


public:
    APK_Immunology(HCPerson* idu);
    APK_Immunology(HCPerson* idu, IPPtr params);
    APK_Immunology(HCPerson* idu, HCVState alter_state, IPPtr params);
    virtual ~APK_Immunology();

    bool exposePartner(std::shared_ptr<Immunology> partner_imm, double tick) override;
    bool receiveInfectiousDose(double tick) override;

    void deactivate() override;

    void leaveExposed() override;

    bool leaveAcute() override;

    bool isAcute() override;
    bool isChronic() override;
    bool isCured() override;
    bool isExposed() override;
    bool isHcvABpos() override;
    bool isHcvRNA(double now) override;
    bool isInfectious(double now) override;

    bool isInTreatment() override;

    bool isNaive() override;
    bool isResistant() override;
    bool isPostTreatment() override;
    bool isTreatable(double now) override;
    HCVState getHCVState() override;
    bool getTestedHCV(double now) override;

    double getTreatmentStartDate() override;

    void setHCVInitState(double now, HCVState state, int logging) override;

    void leaveTreatment(bool treatment_succeeded) override;
    void startTreatment(bool adherent, double now) override;

    void purgeActions() override;

    
};


} /* namespace hepcep */

#endif /* SRC_APKIMMUNOLOGY_H_ */
