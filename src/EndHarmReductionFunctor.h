/*
 * EndHarmReductionFunctor.h
 *
 * Used to schedule the end of DAA-related harm reduction behvaior.
 *
 */

#ifndef SRC_ENDHARMREDUCTIONFUNCTOR_H_
#define SRC_ENDHARMREDUCTIONFUNCTOR_H_

#include "repast_hpc/Schedule.h"
#include <HCPerson.h>

namespace hepcep {

class HCPerson;

class EndHarmReductionFunctor : public repast::Functor {

private:
    HCPerson* person;
    double inject_intensity;

public:
    EndHarmReductionFunctor(HCPerson* person, double intensity);
    virtual ~EndHarmReductionFunctor();

    void operator()();
   
};

} /* namespace hepcep */

#endif /* SRC_ENDHARMREDUCTIONFUNCTOR_H_ */
