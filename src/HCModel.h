/*
 * HCModel.h
 *
 *  Created on: Nov 27, 2017
 *      Author: nick
 */

#ifndef SRC_HCMODEL_H_
#define SRC_HCMODEL_H_

#include "chi_sim/AbstractModel.h"

#include "HCPerson.h"
#include "Zone.h"
#include "FileSink.h"
#include "Network.h"

using namespace chi_sim;
using namespace repast;
using namespace std;

namespace hepcep {

using AbsModelT = chi_sim::AbstractModel<HCPerson, HCPlace, int>;

class HCModel: public AbsModelT {

private:
    int run;
//    FileSink<double> file_sink;
    Network<HCPerson> network;

protected:

	// PersonData queue used for generating new HCPerson instances.
	deque<HCPersonData> personData;
	
	map<std::string,Zone> zoneMap;
	
	map<Zone, map<Zone,double>> zoneDistanceMap;

    // not used in initial version
    void nextActSelected(PersonPtr& person, chi_sim::NextPlace<HCPlace>& next_act) override {
    }

    /**
     * Create a person from the specified parameters.
     *
     * @param index the index of the start of the block of data for this Person
     */
    // not used in initial version
    PersonPtr createPerson(unsigned int p_id, int index, int* data) override {
        return std::shared_ptr<HCPerson>();
    }

    /**
     * Update the state of the specified person with the data in data.
     *
     * @param index the index of the start of the block of data for this Person
     */
    // not used in initial version
    virtual void updatePerson(PersonPtr& person, int index, int* data) override {
    }

public:
    HCModel(repast::Properties& props, unsigned int moved_data_size);
    virtual ~HCModel();

    void step();

};

} /* namespace hepcep */

#endif /* SRC_HCMODEL_H_ */
