/*
 * @file VK_Immunology.h
 *
 * Class for the viral kinetics immunology that includes in-host dynamic time series.
 *
 */

#ifndef SRC_VK_IMMUNOLOGY_H_
#define SRC_VKIMMUNOLOGY_H_

#include "Immunology.h"

namespace hepcep {

class VK_Immunology : public Immunology {

private:
    friend void write_immunology(std::shared_ptr<Immunology>, AttributeWriter&, double);
    friend void read_immunology(NamedListAttribute*, std::shared_ptr<Immunology>, HCPerson*, double);
    
    bool isInTreatmentViralSuppression(double tick);

    // The current time in days along the viral load curve.
    unsigned int viral_load_time; 

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
