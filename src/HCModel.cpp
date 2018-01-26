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
					zoneDistanceMap(),
					zonePopulation(),
					effectiveZonePopulation()
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

	int personCount = Parameters::instance()->getIntParameter(INITIAL_PWID_COUNT);

	PersonCreator personCreator;

	personCreator.create_persons(local_persons, personData, personCount);

	std::cout << "Initial PWID count: " << local_persons.size() << std::endl;

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

	// TODO Handle Repast context.add() actions re: projectons etc.
	//      Network add agents

	performInitialLinking();
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

void HCModel::performInitialLinking(){

	double total_edges = network.edgeCount();
	double total_recept_edge_target = 0;
	double total_give_edge_target = 0;

	for (auto iter = network.verticesBegin(); iter != network.verticesEnd(); ++iter){
		PersonPtr person = (*iter);

		total_recept_edge_target += person->getDrugReceptDegree();
		total_give_edge_target += person->getDrugGivingDegree();
	}

	int iteration = 0;

	//stopping criterion, in terms of actual vs. required number of edges
	double DENSITY_TARGET = 0.95;

	//maximal number of iterations when forming the network, to prevent too much work
	int    MAXITER = 30;
	while ((total_edges/total_recept_edge_target < DENSITY_TARGET) &&
				(total_edges/total_give_edge_target < DENSITY_TARGET) &&
				(iteration < MAXITER)) {

		std::cout << "Total edges: " << total_edges << ". target in: " << total_recept_edge_target
				<< ". target out: " << total_give_edge_target << std::endl;

		zoneCensus();
		performLinking();
		performLinking();

		total_edges = network.edgeCount();
		iteration ++;
	}
	std::cout << "Total edges: " << total_edges << ". target in: " << total_recept_edge_target
				<< ". target out: " << total_give_edge_target << std::endl;

	if (iteration == MAXITER) {
		std::cout << "Initial linking reached the maximum number of iterations (" << MAXITER << ")" << std::endl;
	}

}

void HCModel::performLinking(){
//	//System.out.printf("%.1f: Linking ... ", RepastEssentials.GetTickCount());
//	TreeMap <Double, Object[]> actions = new TreeMap <Double, Object[]> ();
//	for(ZoneAgent zone1 : effective_zone_population.keySet()) {
//		if(effective_zone_population.get(zone1).size() == 0) {
//			continue;
//		}
//		for(ZoneAgent zone2 : effective_zone_population.keySet()) {
//			Double rate = interaction_rate(zone1, zone2);
//			if (rate == 0.0) {
//				continue;
//			}
//			Exponential exp_gen = RandomHelper.createExponential(rate);
//			for (double t = 0; ;) {
//				t += exp_gen.nextDouble();
//				if (t > linking_time_window) {
//					break;
//				}
//				Object[] zones = new Object[2];
//				zones[0] = zone1;
//				zones[1] = zone2;
//				actions.put(t, zones);
//			}
//		}
//		if(actions.size() > 1E6) {
//			System.out.println("Warning: too many linking actions might exhaust heap memory.  Reduce linking_time_window.");
//		}
//	}
//	//System.out.print("building ... ");
//	int num_new_links = 0;
//	for(Object[] pair : actions.values()) {
//		boolean new_link = link_zones((ZoneAgent)pair[0], (ZoneAgent)pair[1], excess_serosorting);
//		num_new_links += new_link? 1: 0;
//	}
//	//System.out.println("Done. New links:" + num_new_links);

}

double HCModel::interactionRate(ZonePtr zone1, ZonePtr zone2){
//	if(zone1 == null || zone2 == null) {
//		return 0.0;
//	}
//	Double dis = zone_zone_distance.get(zone1).get(zone2);
//	if(dis == null) {
//		dis = getDistance(zone1,zone2);
//		zone_zone_distance.get(zone1).put(zone2, dis);
//	}
//	LinkedList<IDU> near_pop    = effective_zone_population.get(zone1);
//	LinkedList<IDU> distant_pop = effective_zone_population.get(zone2);
//	if(near_pop == null || distant_pop == null) {
//		return 0.0;
//	}
//	//zone-zone interaction rate based on distance and population
//	int pop1 = near_pop.size();
//	int pop2 = distant_pop.size();
//	double ret = 0;
//	if (dis > interaction_home_cutoff) {
//		if (zone1.getDrug_market() == zone2.getDrug_market()){
//			ret = (interaction_rate_at_drug_sites*pop1*pop2) + (interaction_rate_exzone*pop1*pop2)/Math.pow(dis, 2);
//		} else {
//			ret = (interaction_rate_exzone*pop1*pop2)/Math.pow(dis, 2);
//		}
//	} else {
//		ret = (interaction_rate_at_drug_sites*pop1*pop2) + (interaction_rate_constant*pop1*pop2);
//	}
//	return ret;

	return 0;
}

void HCModel::zoneCensus(){
	std::cout << "Census... " ;

	zonePopulation.clear();
	effectiveZonePopulation.clear();

	totalIDUPopulation  = 0;

	for (auto entry : local_persons) {
		PersonPtr& person = entry.second;
		totalIDUPopulation += 1;

		ZonePtr zone = person->getZone();

		std::vector<PersonPtr> myEffAgents = effectiveZonePopulation[zone];

		effectiveZonePopulation[zone] = myEffAgents;

		//if (person->canAcceptInOrOutConnection()){
		//	myEffAgents.push_back(person);
		//}

		myEffAgents.push_back(person);

	}

//	for(Object obj : context) {
//		if(obj instanceof IDU) {
//			IDU agent = (IDU) obj;
//			total_IDU_population += 1;
//			assert agent.isActive();
//			ZoneAgent zone = agent.getZone();
//			LinkedList <IDU> my_eff_agents = effective_zone_population.get(zone);
//			if (my_eff_agents == null) {
//				my_eff_agents = new LinkedList<IDU> ();
//				effective_zone_population.put(zone, my_eff_agents);
//			}
//			if(agent.can_accept_in_or_out_connection()) {
//				my_eff_agents.add(agent);
//			}
//			ArrayList <IDU> my_agents = zone_population.get(zone);
//			if (my_agents == null) {
//				my_agents = new ArrayList<IDU> ();
//				zone_population.put(zone, my_agents);
//			}
//			my_agents.add(agent);
//		}
//	}

	std::cout << "done." << std::endl;

}

} /* namespace hepcep */
