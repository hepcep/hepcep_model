/*
 * Event.h
 *
 *  Created on: Mar 2, 2018
 *      Author: nick
 */

#ifndef SRC_EVENT_H_
#define SRC_EVENT_H_

#include <memory>

#include "repast_hpc/Schedule.h"

namespace hepcep {

class Event : public repast::Functor {

private:
    repast::Functor* func_;
    bool canceled;

public:
    Event(repast::Functor* func);
    virtual ~Event();

    void cancel();
    void operator()();
};

template<typename ClassType, typename MethodRetType>
class MethodFunctor: public repast::Functor {
private:
    MethodRetType (ClassType::*fptr)();
    ClassType *obj;
public:
    MethodFunctor(ClassType *_obj, MethodRetType(ClassType::*_fptr)()) :
        fptr(_fptr), obj(_obj) {
    }
    ;
    ~MethodFunctor() {
    }
    ;
    void operator()() {
        (obj->*fptr)();
    }
};



} /* namespace hepcep */

#endif /* SRC_EVENT_H_ */
