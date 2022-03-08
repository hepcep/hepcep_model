#ifndef SRC_IMMUNOLOGY_INTERFACE_H
#define SRC_IMMUNOLOGY_INTERFACE_H

#include <memory>
#include <vector>

#include "boost/shared_ptr.hpp"

#include "Event.h"
#include "HCVState.h"

namespace hepcep {

class HCPerson;
class AttributeWriter;

class Immunology {
    

public:

    Immunology(HCPerson* idu);
    virtual ~Immunology(){}

    // TODO the following were included in the private members of the original
    //      Immunology, however casting shared pointers between the new subclasses
    //      doesn't work with accessing private fields directly, so these are now
    //      public, but we could update the accesss in other parts of the model
    //      and make these private again.
    
    // this is a pointer and not a shared_ptr because HCPerson needs to pass itself.
    HCPerson* idu_;
    
    // HCVState hcv_state;

    /**
     * Exposes a partner, perhaps leading to infected.
     *
     * @return true if a new infection was established in partner, otherwise false.
     * 
     * "give_exposure" in APK Immunology.java
     */
    virtual bool exposePartner(std::shared_ptr<Immunology> partner_imm, double tick) = 0;

    virtual void deactivate() = 0;

    virtual void leaveExposed() = 0;

    virtual bool leaveAcute() = 0;

    virtual bool isAcute() = 0;
    virtual bool isChronic() = 0;
    virtual bool isCured() = 0;
    virtual bool isExposed() = 0;
    virtual bool isHcvABpos() = 0;
    virtual bool isHcvRNA(double now) = 0;
    virtual bool isInfectious(double now) = 0;

    virtual bool isInTreatment() = 0;

    virtual bool isNaive() = 0;
    virtual bool isResistant() = 0;
    virtual bool isPostTreatment() = 0;
    virtual bool isTreatable(double now) = 0;
    virtual HCVState getHCVState() = 0;
    virtual bool getTestedHCV(double now) = 0;

    virtual double getTreatmentStartDate() = 0;

    virtual void setHCVInitState(double now, HCVState state, int logging) = 0;

    virtual void leaveTreatment(bool treatment_succeeded) = 0;
    virtual void startTreatment(bool adherent, double now) = 0;

    virtual  void purgeActions() = 0;

    /**
     * start a NATURAL infection via exposure.
     * 1. the calling method is responsible for announcing the exposure, and updating the time of last exposure
     * 2. if one is recovered, it's possible to be "infected" a new
     * 3. if one has RNA, then no new infection would be established.
     *
     * @returns true iff new infection has been started
     *
     */
    virtual bool receiveInfectiousDose(double tick) = 0;


};

}  // namespace hepcep

#endif  // SRC_IMMUNOLOGY_INTERFACE_H