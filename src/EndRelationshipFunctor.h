/*
 * EndRelationshipFunctor.h
 *
 *  Created on: Apr 2, 2018
 *      Author: eric
 */

#ifndef SRC_ENDRELATIONSHIPFUNCTOR_H_
#define SRC_ENDRELATIONSHIPFUNCTOR_H_

#include <repast_hpc/Schedule.h>
#include <Network.h>
#include <HCPerson.h>

namespace hepcep {

class EndRelationshipFunctor : public repast::Functor {

private:
	PersonPtr sourcePerson;
	PersonPtr buddy;
	NetworkPtr<HCPerson> network;

public:
	EndRelationshipFunctor(PersonPtr source, PersonPtr other, NetworkPtr<HCPerson> net);
	virtual ~EndRelationshipFunctor();
	void operator()();
};

} /* namespace hepcep */

#endif /* SRC_ENDRELATIONSHIPFUNCTOR_H_ */
