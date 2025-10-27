/*
 * Event.h
 *
 *  Created on: Mar 2, 2018
 *      Author: nick
 */

#ifndef SRC_EVENT_H_
#define SRC_EVENT_H_

#include <memory>
#include "boost/shared_ptr.hpp"

#include "repast_hpc/Schedule.h"
#include "network_utils.h"


namespace hepcep {

// used by serialization mechanism to recreate the functors for the events
enum class EventFuncType{LEAVE_EXPOSED, LEAVE_ACUTE, END_TREATMENT, END_HARM_REDUCTION};

class Event : public repast::Functor {

private:
    friend void write_event(int idx, boost::shared_ptr<Event>, std::stringstream&);
    double scheduled_for_;
    EventFuncType ef_type_;
    repast::Functor* func_;
    bool canceled;

public:
    Event(double scheduled_for, EventFuncType ef_type, repast::Functor* func);
    virtual ~Event();

    void cancel();
    double scheduled_for() const;
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
