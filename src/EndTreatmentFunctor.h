/*
 * EndTreamentFunctor.h
 *
 *  Created on: Mar 2, 2018
 *      Author: nick
 */

#ifndef SRC_ENDTREATMENTFUNCTOR_H_
#define SRC_ENDTREATMENTFUNCTOR_H_

#include "repast_hpc/Schedule.h"

namespace hepcep {

class Immunology;

class EndTreatmentFunctor : public repast::Functor {
private:
    bool success;
    Immunology* immunology;

public:
    EndTreatmentFunctor(bool treatment_success, Immunology* imm);
    virtual ~EndTreatmentFunctor();

    void operator()();
    bool isSuccess() const {
        return success;
    }

};

} /* namespace hepcep */

#endif /* SRC_ENDTREATMENTFUNCTOR_H_ */
