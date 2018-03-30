/*
 * @file HCModel.cpp
 * HepCEP model.
 *
 * @author Eric Tatara
 * @author Nick Collier
 */

#include <stdio.h>

#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/Schedule.h"
#include "chi_sim/Parameters.h"

#include "HCModel.h"
#include "Statistics.h"
#include "PersonCreator.h"
#include "PersonDataLoader.h"
#include "ZoneLoader.h"
#include "parameters_constants.h"

namespace hepcep {

HCModel::HCModel(repast::Properties& props, unsigned int moved_data_size) : 
					AbsModelT(moved_data_size, props),
					run(std::stoi(props.getProperty(RUN))) ,
//					network(true),
					personData(),
					zoneMap(),
					zoneDistanceMap(),
					zonePopulation(),
					effectiveZonePopulation()
{

	string output_directory = chi_sim::Parameters::instance()->getStringParameter(OUTPUT_DIRECTORY);

	std::cout << "HepCEP Model Initialization." << std::endl;
	std::cout << "Output dir: " << output_directory << std::endl;

	network = std::make_shared<Network<HCPerson>>(true);

	string stats_fname = output_directory + "/" + chi_sim::Parameters::instance()->getStringParameter(STATS_OUTPUT_FILE);
	string events_fname = output_directory + "/" + chi_sim::Parameters::instance()->getStringParameter(EVENTS_OUTPUT_FILE);
	Statistics::init(stats_fname, events_fname);

	repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
	runner.scheduleEvent(1, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<HCModel>(this, &HCModel::step)));
	runner.scheduleEndEvent(repast::Schedule::FunctorPtr(new repast::MethodFunctor<HCModel>(this, &HCModel::atEnd)));

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

	personCreator = std::make_shared<PersonCreator>();
	burnInControl();

	personCreator->create_persons(local_persons, personData, zoneMap, personCount);

	std::cout << "Initial PWID count: " << local_persons.size() << std::endl;

	performInitialLinking();

	std::string fname("./output/net_initial.gml");
	write_network(fname, network, &writePerson, &writeEdge);
	// write t0 stats
	Statistics::instance()->recordStats(0, local_persons);

	// TODO ------- Test prints below --------
	for (auto entry : zonePopulation){
			const ZonePtr & zone = entry.first;

			std::vector<PersonPtr> & list = entry.second;

			std::cout << "Zone " << zone->getZipcode() << " size = " << list.size() << std::endl;
	}

}


HCModel::~HCModel() {}

void HCModel::atEnd() {
    Statistics::instance()->close();
}

void HCModel::step() {
	double tick = repast::RepastProcess::instance()->getScheduleRunner().currentTick();

	// Schedule each Person agent step

	// Pass in the network reference?

	for (auto entry : local_persons) {
		PersonPtr& person = entry.second;
		person->step(network);
	}

	// Collect "dead" agents and remove them from the person list

	Statistics::instance()->recordStats(tick, local_persons);
}

void HCModel::performInitialLinking(){

	double total_edges = network->edgeCount();
	double total_recept_edge_target = 0;
	double total_give_edge_target = 0;

	// APK iterates over network nodes but we can just iterate over the population
//	for (auto iter = network.verticesBegin(); iter != network.verticesEnd(); ++iter){
//			PersonPtr person = (*iter);

	for (auto entry : local_persons) {
		PersonPtr & person = entry.second;

		total_recept_edge_target += person->getDrugReceptDegree();
		total_give_edge_target += person->getDrugGivingDegree();

		network->addVertex(person);
	}

	int iteration = 0;

	//stopping criterion, in terms of actual vs. required number of edges
	double DENSITY_TARGET = 0.99;

	//maximal number of iterations when forming the network, to prevent too much work
	int    MAXITER = 30;
	while ((total_edges/total_recept_edge_target < DENSITY_TARGET) &&
				(total_edges/total_give_edge_target < DENSITY_TARGET) &&
				(iteration < MAXITER)) {

		std::cout << "> Total edges: " << total_edges << ". target in: " << total_recept_edge_target
				<< ". target out: " << total_give_edge_target << std::endl;

		zoneCensus();
		performLinking();
		performLinking();

		total_edges = network->edgeCount();
		iteration ++;
	}
	std::cout << " Final Total edges: " << total_edges << ". target in: " << total_recept_edge_target
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

			// Skip if zone population is zero
			if (entry2.second.size() == 0){
				continue;
			}

			double rate = interactionRate(zone1, zone2);

//			if (zone1 == zone2){
//				std::cout << "eq " << rate;
//			}
//			else{
//				std::cout << "" << rate;
//			}

			if (rate == 0.0) {
				continue;
			}

			repast::ExponentialGenerator generator =
					repast::Random::instance()->createExponentialGenerator (rate);

			double linkingTimeWindow = chi_sim::Parameters::instance()->getDoubleParameter(LINKING_TIME_WINDOW);
			double t = 0;

			int count = 0;
			t += generator.next();
			while (t <= linkingTimeWindow){

				linkZones(zone1, zone2);

				t += generator.next();
				count += 1;
			}

//			std::cout << "\t" << count << std::endl;
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

	int a1_idx = std::round(d1 * (s1-1));
	int a2_idx = std::round(d2 * (s2-1));

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
	if (network->inEdgeCount(person2) >= person2->getDrugReceptDegree()) {
		return;
	}
	if (network->outEdgeCount(person1) >= person1->getDrugGivingDegree()) {
		return;
	}
	double homophily = chi_sim::Parameters::instance()->getDoubleParameter(HOMOPHILY_STRENGTH);
	double roll = repast::Random::instance()->nextDouble();
	if (person1->getDemographicDistance(person2) * homophily > roll) {
		return;
	}

	double dist = zoneDistanceMap[person1->getZipcode()][person2->getZipcode()];

	network->addEdge(person1, person2)->putAttribute("distance", dist);


	// TODO schedule edge time
//	double out_tie_end_time = RunEnvironment.getInstance().getCurrentSchedule().getTickCount()
//					                		 + tie_endurance_distribution.nextDouble();
//	ScheduleParameters out_tie_end_params = ScheduleParameters.createOneTime(out_tie_end_time);
//	ISchedule schedule = RunEnvironment.getInstance().getCurrentSchedule();
//	schedule.schedule(out_tie_end_params, this, "end_relationship", obj);

	// Check conditions for adding a directed edge from person2 -> person1
	if (network->inEdgeCount(person1) >= person1->getDrugReceptDegree()) {
		return;
	}
	if (network->outEdgeCount(person2) >= person2->getDrugGivingDegree()) {
		return;
	}
	network->addEdge(person2, person1)->putAttribute("distance", dist);

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

	zonePopulation.clear();
	effectiveZonePopulation.clear();

	totalIDUPopulation  = 0;

	for (auto entry : local_persons) {
		PersonPtr & person = entry.second;
		totalIDUPopulation += 1;

		std::string zipcode = person->getZipcode();

		if (zoneMap.find(zipcode) == zoneMap.end()){
			// TODO handle zone undefined

			std::cout << "Error: zone not found: " << zipcode << std::endl;
		}

		ZonePtr & zone = zoneMap[zipcode];

		std::vector<PersonPtr> & myEffAgents = effectiveZonePopulation[zone];

		// Effective agents are not incarcerated status and have available in or out
		// degree connections
		// TODO check against incarceration state

		unsigned int inCount = network->inEdgeCount(person);
		unsigned int outCount = network->outEdgeCount(person);

		if (inCount < person->getDrugReceptDegree() ||
				outCount < person->getDrugGivingDegree()){
			myEffAgents.push_back(person);
		}

		std::vector<PersonPtr> & myAgents = zonePopulation[zone];

		myAgents.push_back(person);
	}
}

/*
 * activate the burn-in mode
 * - should be called before the agents are created
 */
void HCModel::burnInControl() {

	double burnInDays = chi_sim::Parameters::instance()->getDoubleParameter(BURN_IN_DAYS);

	if(burnInDays <= 0) {
		return;
	}

	Statistics::instance()->setBurninMode(true);
	personCreator->setBurnInPeriod(true, burnInDays);

	// TODO Schedule
//	main_schedule.schedule(ScheduleParameters.createOneTime(RepastEssentials.GetTickCount() + burnInDays), this, "burnInEnd", burnInDays);
}

void HCModel::burnInEnd(double burnInDays) {

	Statistics::instance()->setBurninMode(false);
	personCreator->setBurnInPeriod(false, -1);

	for (auto entry : local_persons) {
			PersonPtr & person = entry.second;

			// Reduce the person's age based on the burn in period.
			double age = person->getAge();
			person->setAge(age - burnInDays);

			Statistics::instance()->logStatusChange(LogType::ACTIVATED, person, "");
	}

	std::cout << "\n**** Finished burn-in. Duration: " << burnInDays << " ****" << std::endl;
}

void writePerson(HCPerson* person, AttributeWriter& write) {
	write("age", person->getAge());
	write("age_started", person->getAgeStarted());
	write("race", "\"" + person->getRace().stringValue() +"\"");
	write("gender", "\"" + person->getGender().stringValue() +"\"");
	write("syringe_source", "\"" + person->getSyringeSource().stringValue() +"\"");
	write("zipcode", "\"" + person->getZipcode() +"\"");

	write("hcv", "\"" + person->getHCVState().stringValue() +"\"");

	write("drug_in_deg", person->getDrugReceptDegree());
	write("drug_out_deg", person->getDrugGivingDegree());
	write("inject_intens", person->getInjectionIntensity());
	write("frac_recept", person->getFractionReceptSharing());

	write("lat", person->getZone()->getLat() + 0.1 * (repast::Random::instance()->nextDouble() - 0.5));
	write("lon", person->getZone()->getLon() + 0.1 * (repast::Random::instance()->nextDouble() - 0.5));

}

void writeEdge(Edge<HCPerson>* edge, AttributeWriter& write) {
	write("distance", edge->getAttribute("distance", 0));
}

} /* namespace hepcep */
