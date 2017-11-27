/*
 * HCModel.cpp
 *
 *  Created on: Nov 27, 2017
 *      Author: nick
 */


#include "repast_hpc/Schedule.h"
#include "chi_sim/Parameters.h"

#include "HCModel.h"
#include "creators.h"
#include "parameters_constants.h"

using namespace chi_sim;
using namespace repast;
using namespace std;

namespace hepcep {

HCModel::HCModel(repast::Properties& props, unsigned int moved_data_size) : AbsModelT(moved_data_size, props) {
    ScheduleRunner& runner = RepastProcess::instance()->getScheduleRunner();
    runner.scheduleEvent(1, 1, Schedule::FunctorPtr(new MethodFunctor<HCModel>(this, &HCModel::step)));

    string persons_file = Parameters::instance()->getStringParameter(PERSONS_FILE);
    create_persons(persons_file, local_persons);
}

HCModel::~HCModel() {}

void HCModel::step() {
    for (auto entry : local_persons) {
        PersonPtr& person = entry.second;
        person->doSomething();
    }

}

} /* namespace hepcep */
