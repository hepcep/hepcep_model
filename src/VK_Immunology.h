/*
 * @file VK_Immunology.h
 *
 * Class for the viral kinetics immunology that includes in-host dynamic time series.
 *
 */

#ifndef SRC_VK_IMMUNOLOGY_H_
#define SRC_VK_IMMUNOLOGY_H_

#include "Immunology.h"
#include "VKProfile.h"

namespace hepcep {

class VK_Immunology : public Immunology {

private:
    friend void write_immunology(std::shared_ptr<Immunology>, AttributeWriter&, double);
    friend void read_immunology(NamedListAttribute*, std::shared_ptr<Immunology>, HCPerson*, double);
    
    bool isInTreatmentViralSuppression(double tick);

    // The intial VK profile types for any agent with an acute infection (defined in HCPersonData)
    static const std::vector<VKProfile> initial_acute_profiles;

    // The intial VK profile types for any agent with a chronic infection (defined in HCPersonData)
    static const std::vector<VKProfile> initial_chronic_profiles;

    // The VK profile types for new infections (N1, N2, N3) for Naive PWID
    static const std::vector<VKProfile> new_infection_profiles;

    // The VK profile types for new infections (R1, R2, R3) for previously cleared PWID
    static const std::vector<VKProfile> re_infection_profiles;

    // The current time in days along the viral load curve.
    unsigned int viral_load_time;

    // The current viral kinetics profile type
    VKProfile vk_profile;

    // Unique ID of the profile series (array) associated with the VKProfile
    unsigned int vk_profile_id;  
    
public:
    VK_Immunology(HCPerson* idu);
    
    virtual ~VK_Immunology();

    bool exposePartner(std::shared_ptr<Immunology> partner_imm, double tick) override;
    bool receiveInfectiousDose(double tick) override;

    void deactivate() override;
    void leaveExposed() override;
    bool leaveAcute() override;

    bool isHcvRNA(double now) override;
    // bool isInfectious(double now) override;

    bool getTestedHCV(double now) override;

    void setHCVInitState(double now, HCVState state, int logging) override;

    void leaveTreatment(bool treatment_succeeded) override;
    void startTreatment(bool adherent, double now) override;

    void purgeActions() override;

    void step() override;

    void reset_viral_load_time();
    double get_viral_load();
    
};


} /* namespace hepcep */

#endif /* SRC_VK_IMMUNOLOGY_H_ */
