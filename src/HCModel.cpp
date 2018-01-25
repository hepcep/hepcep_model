/*
 * HCModel.cpp
 *
 *  Created on: Nov 27, 2017
 *      Author: nick
 */


#include "repast_hpc/Schedule.h"
#include "chi_sim/Parameters.h"

#include "HCModel.h"
#include "Statistics.h"
#include "PersonCreator.h"
#include "PersonDataLoader.h"
#include "ZoneLoader.h"
#include "parameters_constants.h"

namespace hepcep {

shared_ptr<ReducibleDataSet<double>> init_data_collection() {

	Statistics* stats = Statistics::instance();
	stats->reset();
	vector<shared_ptr<DataSource<double>>> data_sources = stats->createDataSources();

	DataSetBuilder<double> builder;
	for (auto ds : data_sources) {
		builder.addDataSource(ds, std::plus<double>());
	}

	return builder.createReducibleDataSet();
}

HCModel::HCModel(repast::Properties& props, unsigned int moved_data_size) : 
					AbsModelT(moved_data_size, props),
					run(std::stoi(props.getProperty(RUN))) ,
					//			file_sink(rank_, run, init_data_collection()) //,
					network(true),
					personData(),
					zoneMap(),
					zoneDistanceMap()
{

	string output_directory = Parameters::instance()->getStringParameter(OUTPUT_DIRECTORY);

	std::cout << "HepCEP Model Initialization." << std::endl;
	std::cout << "Output dir: " << output_directory << std::endl;

//    string stats_fname = output_directory + "/" + Parameters::instance()->getStringParameter(STATS_OUTPUT_FILE);
//    file_sink.open(insert_in_file_name(stats_fname, run));

	ScheduleRunner& runner = RepastProcess::instance()->getScheduleRunner();
	runner.scheduleEvent(1, 1, Schedule::FunctorPtr(new MethodFunctor<HCModel>(this, &HCModel::step)));

	std::string cnep_file = Parameters::instance()->getStringParameter(CNEP_PLUS_FILE);
	std::cout << "CNEP+ file: " << cnep_file << std::endl;
	loadPersonData(cnep_file, personData);
	std::cout << "CNEP+ profiles loaded: " << personData.size() << std::endl;

	// TODO initial person count
	int personCount = 5;

	PersonCreator personCreator;

	personCreator.create_persons(local_persons, personData, personCount);

	std::string zones_file = Parameters::instance()->getStringParameter(ZONES_FILE);
	std::cout << "Zones file: " << zones_file << std::endl;
	loadZones(zones_file, zoneMap);

	std::string zones_distance_file = Parameters::instance()->getStringParameter(ZONES_DISTANCE_FILE);
	std::cout << "Zones distance file: " << zones_distance_file << std::endl;
//	loadZonesDistances(zones_distance_file, zoneMap, zoneDistanceMap);

//    string network_file = Parameters::instance()->getStringParameter(NETWORK_FILE);
//    create_network(network_file, local_persons, network);

// check that the network loading worked
//    std::cout << network.vertexCount() << std::endl;
//    vector<EdgePtrT<HCPerson>> edges;
//    network.outEdges(local_persons.at(2), edges);
//    for (auto e : edges) {
//        std::cout << e->v1()->id() << " -> " << e->v2()->id() << std::endl;
//    }
}

HCModel::~HCModel() {}

void HCModel::step() {
	double tick = RepastProcess::instance()->getScheduleRunner().currentTick();
	for (auto entry : local_persons) {
		PersonPtr& person = entry.second;
		person->doSomething();

		Statistics::instance()->increment(4);
	}
	//    file_sink.record(tick);
	Statistics::instance()->reset();
}

} /* namespace hepcep */
