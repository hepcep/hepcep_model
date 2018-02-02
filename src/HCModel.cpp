/*
 * @file HCModel.cpp
 * HepCEP model.
 *
 * @author Eric Tatara
 * @author Nick Collier
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

	string output_directory = chi_sim::Parameters::instance()->getStringParameter(OUTPUT_DIRECTORY);

	std::cout << "HepCEP Model Initialization." << std::endl;
	std::cout << "Output dir: " << output_directory << std::endl;

//    string stats_fname = output_directory + "/" + Parameters::instance()->getStringParameter(STATS_OUTPUT_FILE);
//    file_sink.open(insert_in_file_name(stats_fname, run));

	repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
	runner.scheduleEvent(1, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<HCModel>(this, &HCModel::step)));

	std::string cnep_file = chi_sim::Parameters::instance()->getStringParameter(CNEP_PLUS_FILE);
	std::cout << "CNEP+ file: " << cnep_file << std::endl;
	loadPersonData(cnep_file, personData);
	std::cout << "CNEP+ profiles loaded: " << personData.size() << std::endl;

	std::string zones_file = chi_sim::Parameters::instance()->getStringParameter(ZONES_FILE);
	std::cout << "Zones file: " << zones_file << std::endl;
	loadZones(zones_file, zoneMap);
	std::cout << "Initial zoneMap size = " << zoneMap.size() << std::endl;

	std::string zones_distance_file = chi_sim::Parameters::instance()->getStringParameter(ZONES_DISTANCE_FILE);
	std::cout << "Zones distance file: " << zones_distance_file << std::endl;
	loadZonesDistances(zones_distance_file, zoneMap, zoneDistanceMap);

	int personCount = chi_sim::Parameters::instance()->getIntParameter(INITIAL_PWID_COUNT);

	PersonCreator personCreator;

	// TODO make PersonCreator sharedPtr if reused
	personCreator.create_persons(local_persons, personData, zoneMap, personCount);

	std::cout << "Initial PWID count: " << local_persons.size() << std::endl;

	performInitialLinking();

	std::string fname("./output/net_initial.gml");
	write_network(fname, network, &writePerson, &writeEdge);

	// TODO ------- Test prints below --------
	for (auto entry : zonePopulation){
			const ZonePtr & zone = entry.first;

			std::vector<PersonPtr> & list = entry.second;

			std::cout << "Zone " << zone->getZipcode() << " size = " << list.size() << std::endl;
		}
}


HCModel::~HCModel() {}

void HCModel::step() {
//	double tick = repast::RepastProcess::instance()->getScheduleRunner().currentTick();
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

	// APK iterates over network nodes but we can just iterate over the population
//	for (auto iter = network.verticesBegin(); iter != network.verticesEnd(); ++iter){
//			PersonPtr person = (*iter);

	for (auto entry : local_persons) {
		PersonPtr & person = entry.second;

		total_recept_edge_target += person->getDrugReceptDegree();
		total_give_edge_target += person->getDrugGivingDegree();

		network.addVertex(person);
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

	for (auto entry1 : effectiveZonePopulation){
		const ZonePtr & zone1 = entry1.first;

		// Skip if zone population is zero
		if (entry1.second.size() == 0){
			continue;
		}

		for (auto entry2 : effectiveZonePopulation){
			const ZonePtr & zone2 = entry2.first;

			double rate = interactionRate(zone1, zone2);
			if (rate == 0.0) {
				continue;
			}

			repast::ExponentialGenerator generator =
					repast::Random::instance()->createExponentialGenerator (rate);

			double linkingTimeWindow = chi_sim::Parameters::instance()->getDoubleParameter(LINKING_TIME_WINDOW);
			double t = 0;

			while (t < linkingTimeWindow){

				linkZones(zone1, zone2);

				t += generator.next();
			}
		}
	}
}

void HCModel::linkZones(const ZonePtr& zone1, const ZonePtr& zone2){

//	std::cout << "Linking zones " << zone1->getZipcode() << " + " <<
//			zone2->getZipcode() << std::endl;

	int s1 = effectiveZonePopulation[zone1].size();
	int s2 = effectiveZonePopulation[zone2].size();
	if (s1 == 0 || s2 == 0) {
		return;
	}

	double d1 = repast::Random::instance()->nextDouble();
	double d2 = repast::Random::instance()->nextDouble();

	int a1_idx = d1 * (s1-1);
	int a2_idx = d2 * (s2-1);

	if(zone1 == zone2 && a1_idx == a2_idx) {
		return;
	}

	PersonPtr & person1 = effectiveZonePopulation[zone1][a1_idx];
	PersonPtr & person2 = effectiveZonePopulation[zone2][a2_idx];

	tryConnect(person1,person2);
}

/**
 * Attempt a bi-directional network connection pair between person1 & person2
 */
void HCModel::tryConnect(const PersonPtr& person1, const PersonPtr& person2){

	// Check conditions for adding a directed edge from person1 -> person2
	if (network.inEdgeCount(person2) >= person2->getDrugReceptDegree()) {
		return;
	}
	if (network.outEdgeCount(person1) >= person1->getDrugGivingDegree()) {
		return;
	}
	double homophily = chi_sim::Parameters::instance()->getDoubleParameter(HOMOPHILY_STRENGTH);
	double roll = repast::Random::instance()->nextDouble();
	if (person1->getDemographicDistance(person2) * homophily > roll) {
		return;
	}

	double dist = zoneDistanceMap[person1->getZipcode()][person2->getZipcode()];

	network.addEdge(person1, person2)->putAttribute("distance", dist);


	// TODO schedule edge time
//	double out_tie_end_time = RunEnvironment.getInstance().getCurrentSchedule().getTickCount()
//					                		 + tie_endurance_distribution.nextDouble();
//	ScheduleParameters out_tie_end_params = ScheduleParameters.createOneTime(out_tie_end_time);
//	ISchedule schedule = RunEnvironment.getInstance().getCurrentSchedule();
//	schedule.schedule(out_tie_end_params, this, "end_relationship", obj);

	// Check conditions for adding a directed edge from person2 -> person1
	if (network.inEdgeCount(person1) >= person1->getDrugReceptDegree()) {
		return;
	}
	if (network.outEdgeCount(person2) >= person2->getDrugGivingDegree()) {
		return;
	}
	network.addEdge(person2, person1)->putAttribute("distance", dist);

	// TODO schedule edge time
//	schedule.schedule(out_tie_end_params, obj, "end_relationship", this); //ends at the same time

}

double HCModel::interactionRate(const ZonePtr& zone1, const ZonePtr& zone2){
	double rate = 0;

	double distance = zoneDistanceMap[zone1->getZipcode()][zone2->getZipcode()];

	int pop1 = effectiveZonePopulation[zone1].size();
	int pop2 = effectiveZonePopulation[zone2].size();

	// TODO store as class fields to avoid lookup cost.
	double interactionHomeCutoff = chi_sim::Parameters::instance()->getDoubleParameter(INTERACTION_HOME_CUTOFF);
	double interactionRateDrugSites = chi_sim::Parameters::instance()->getDoubleParameter(INTERACTION_RATE_DRUG_SITES);
	double interactionRateExzone = chi_sim::Parameters::instance()->getDoubleParameter(INTERACTION_RATE_EXZONE);
	double interactionRateConst = chi_sim::Parameters::instance()->getDoubleParameter(INTERACTION_RATE_CONST);


	if (distance > interactionHomeCutoff){
		if (zone1->getDrugMarket() ==  zone2->getDrugMarket()){
			rate = (interactionRateDrugSites * pop1 * pop2) +
					(interactionRateExzone * pop1 * pop2) / std::pow(distance, 2);
		}
		else{
			rate = (interactionRateExzone * pop1 * pop2)/std::pow(distance, 2);
		}
	}
	else{
		rate = (interactionRateDrugSites * pop1 * pop2) + (interactionRateConst * pop1 * pop2);
	}
	return rate;
}

/*
 * Determine how many IDUs in each zones are available to form new connections
 * - the census is stored in zone_population (all) and effective_zone_population
 * 		(only those that can form new connections)
 * - typically occurs once a day (linking_time_window)
 */
void HCModel::zoneCensus(){
	std::cout << "Census... " ;

	zonePopulation.clear();
	effectiveZonePopulation.clear();

	totalIDUPopulation  = 0;

	for (auto entry : local_persons) {
		PersonPtr & person = entry.second;
		totalIDUPopulation += 1;

		std::string zipcode = person->getZipcode();

//		std::cout << "Person " << person << " zip: " << zipcode << std::endl;

		if (zoneMap.find(zipcode) == zoneMap.end()){
			// TODO handle zone undefined

			std::cout << "Error: zone not found: " << zipcode << std::endl;
		}

		ZonePtr & zone = zoneMap[zipcode];

		std::vector<PersonPtr> & myEffAgents = effectiveZonePopulation[zone];

		// Effective agents are not incarcerated status and have available in or out
		// degree connections
		// TODO check against incarceration state

		unsigned int inCount = network.inEdgeCount(person);
		unsigned int outCount = network.outEdgeCount(person);

		if (inCount < person->getDrugReceptDegree() ||
				outCount < person->getDrugGivingDegree()){
			myEffAgents.push_back(person);
		}

		std::vector<PersonPtr> & myAgents = zonePopulation[zone];

		myAgents.push_back(person);
	}
}

void writePerson(HCPerson* person, AttributeWriter& write) {
	write("age", person->getAge());
}

void writeEdge(Edge<HCPerson>* edge, AttributeWriter& write) {
	write("distance", edge->getAttribute("distance", 0));
}

} /* namespace hepcep */
