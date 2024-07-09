/*
 * @file HCModel.cpp
 * HepCEP model.
 *
 * @author Eric Tatara
 * @author Nick Collier
 */

#include <stdio.h>
#include <cmath>
#include <iomanip>
#include <chrono>

#include "boost/range/algorithm.hpp"
#include "boost/tokenizer.hpp"
#include "boost/algorithm/string.hpp"


#include "repast_hpc/RepastProcess.h"
#include "repast_hpc/Random.h"
#include "repast_hpc/Schedule.h"
#include "chi_sim/Parameters.h"
#include "chi_sim/file_utils.h"

#include "Distributions.h"
#include "EndRelationshipFunctor.h"
#include "HCModel.h"
#include "Statistics.h"
#include "PersonCreator.h"
#include "PersonDataLoader.h"
#include "ZoneLoader.h"
#include "parameters_constants.h"
#include "serialize.h"
#include "OpioidTreatment.h"
#include "OpioidContinueTreatmentEvent.h"
#include "ViralKinetics.h"
#include "VKProfile.h"

namespace hepcep {

class WriteNet : public repast::Functor {
    private:
        std::string fname_;
        NetworkPtr<HCPerson> network_;
        double at_;
    
    public:
        WriteNet(const std::string& fname, double at, NetworkPtr<HCPerson> network);
        virtual ~WriteNet() {}
        void operator()();
};

WriteNet::WriteNet(const std::string& fname, double at, NetworkPtr<HCPerson> network) : fname_(fname),
    network_(network),  at_(at) {}

void WriteNet::operator()() {
    write_network(fname_, at_, network_, &write_person, &write_edge);
}

void init_stats(const std::string& output_directory, int run_number, std::map<unsigned int,ZonePtr> zone_map) {
    // Initialize statistics collection
    string stats_fname = output_directory + "/" + chi_sim::Parameters::instance()->getStringParameter(STATS_OUTPUT_FILE);
    string events_fname = output_directory + "/" + chi_sim::Parameters::instance()->getStringParameter(EVENTS_OUTPUT_FILE);
    string personsFilename = output_directory + "/" + chi_sim::Parameters::instance()->getStringParameter(PERSONS_OUTPUT_FILE);
    string needle_sharing_filename = output_directory + "/" + chi_sim::Parameters::instance()->getStringParameter(NEEDLESHARING_OUTPUT_FILE);

    string filter_string = chi_sim::Parameters::instance()->getStringParameter(EVENT_FILTERS);
    boost::trim(filter_string);
    std::shared_ptr<Filter<LogType>> filter;

    if (filter_string == "NONE") {
        filter = std::make_shared<NeverPassFilter<LogType>>();
        std::cout << "Event logging is off" << std::endl;
    } else if (filter_string == "ALL") {
        filter = std::make_shared<AlwaysPassFilter<LogType>>();
        std::cout << "Event logging is on - logging all events" << std::endl;
    } else {
        std::shared_ptr<ContainsFilter<LogType>> cf = std::make_shared<ContainsFilter<LogType>>();
        boost::char_separator<char> sep(",");
        boost::tokenizer<boost::char_separator<char>> tok(filter_string, sep);
        std::cout << "Event logging is on - logging ";
        for (auto item : tok) {
            boost::trim(item);
            std::cout << item << " ";
            cf->addItem(LogType::valueOf(item));
        }
        std::cout << std::endl;
        filter = cf;
    }
    

    Statistics::init(stats_fname, events_fname, personsFilename, needle_sharing_filename, 
        zone_map, filter, run_number);
}

void init_opioid_treatment_drugs() {
    chi_sim::Parameters* params = chi_sim::Parameters::instance();
    OpioidTreatmentDrugs::initDrug({DrugName::METHADONE, 
        params->getDoubleParameter(METHADONE_MAX_INJECTION_INTENSITY),params->getDoubleParameter(METHADONE_DURATION), 
        params->getDoubleParameter(METHADONE_URBAN_THRESHOLD), params->getDoubleParameter(METHADONE_NON_URBAN_THRESHOLD), 
        params->getDoubleParameter(OT_URBAN_MAX_THRESHOLD), params->getDoubleParameter(OT_NON_URBAN_MAX_THRESHOLD),
        params->getDoubleParameter(METHADONE_P_CLOSE), params->getDoubleParameter(METHADONE_P_FAR)});
    OpioidTreatmentDrugs::initDrug({DrugName::BUPRENORPHINE, 
        params->getDoubleParameter(BUPRENORPHINE_MAX_INJECTION_INTENSITY), params->getDoubleParameter(BUPRENORPHINE_DURATION), 
        params->getDoubleParameter(BUPRENORPHINE_URBAN_THRESHOLD),params->getDoubleParameter(BUPRENORPHINE_NON_URBAN_THRESHOLD), 
        params->getDoubleParameter(OT_URBAN_MAX_THRESHOLD), params->getDoubleParameter(OT_NON_URBAN_MAX_THRESHOLD),
        params->getDoubleParameter(BUPRENORPHINE_P_CLOSE),params->getDoubleParameter(BUPRENORPHINE_P_FAR)});
    OpioidTreatmentDrugs::initDrug({DrugName::NALTREXONE, 
        params->getDoubleParameter(NALTREXONE_MAX_INJECTION_INTENSITY),params->getDoubleParameter(NALTREXONE_DURATION),
        params->getDoubleParameter(NALTREXONE_URBAN_THRESHOLD), params->getDoubleParameter(NALTREXONE_NON_URBAN_THRESHOLD),
        params->getDoubleParameter(OT_URBAN_MAX_THRESHOLD), params->getDoubleParameter(OT_NON_URBAN_MAX_THRESHOLD),
        params->getDoubleParameter(NALTREXONE_P_CLOSE), params->getDoubleParameter(NALTREXONE_P_FAR)});
}

HCModel::HCModel(repast::Properties& props, MPI_Datatype mpi_person_type) :
                    AbsModelT(props, mpi_person_type),
                    run(std::stoi(props.getProperty(RUN))) ,
                    personData(),
                    zoneMap(),
                    zoneDistanceMap(),
                    effectiveZonePopulation(),
                    treatmentEnrollmentProb(),
                    treatmentEnrollmentResidual(),
                    opioidTreatmentEnrollmentProb(),
                    opioidTreatmentEnrollmentResidual()
{

    init_opioid_treatment_drugs();

    // TODO put all the data init in a separate method
    double seed = chi_sim::Parameters::instance()->getIntParameter("random.seed");
    double rnum = chi_sim::Parameters::instance()->getIntParameter("run.number");
    
    // Sample the Repast random instance lots of times to check for proper seeding.
    double d = 0;
    for(int i=0; i<1E5; i++){
        d = repast::Random::instance()->nextDouble();
    }
    
    std::cout << "HepCEP Model Initialization... Run # " << rnum << ", Random seed: " << seed << ", rand = " << d << std::endl;
    
    // Initialize statistical distributions used in the model.
    double attritionRate = chi_sim::Parameters::instance()->getDoubleParameter(ATTRITION_RATE);
    double meanEdgeLifetime = chi_sim::Parameters::instance()->getDoubleParameter(MEAN_EDGE_LIFETIME);
    double meanCareerDuration = chi_sim::Parameters::instance()->getDoubleParameter(MEAN_CAREER_DURATION);

    Distributions::init(attritionRate, meanEdgeLifetime, meanCareerDuration);

    // Initialize model variables (used later) from model.props
    netInflow = chi_sim::Parameters::instance()->getDoubleParameter(NET_INFLOW);
    interactionHomeCutoff = chi_sim::Parameters::instance()->getDoubleParameter(INTERACTION_HOME_CUTOFF);
    interactionRateDrugSites = chi_sim::Parameters::instance()->getDoubleParameter(INTERACTION_RATE_DRUG_SITES);
    interactionRateExzone = chi_sim::Parameters::instance()->getDoubleParameter(INTERACTION_RATE_EXZONE);
    interactionRateConst = chi_sim::Parameters::instance()->getDoubleParameter(INTERACTION_RATE_CONST);
    linkingTimeWindow = chi_sim::Parameters::instance()->getDoubleParameter(LINKING_TIME_WINDOW);
    homophily = chi_sim::Parameters::instance()->getDoubleParameter(HOMOPHILY_STRENGTH);
    
    burnInDays = chi_sim::Parameters::instance()->getDoubleParameter(BURN_IN_DAYS);

    double treatment_enrollment_probability_unbiased = chi_sim::Parameters::instance()->getDoubleParameter(TREATMENT_ENROLLMENT_PROBABILITY_UNBIASED);
    double treatment_enrollment_probability_HRP = chi_sim::Parameters::instance()->getDoubleParameter(TREATMENT_ENROLLMENT_PROBABILITY_HRP);
    double treatment_enrollment_probability_fullnetwork = chi_sim::Parameters::instance()->getDoubleParameter(TREATMENT_ENROLLMENT_PROBABILITY_FULLNETWORK);
    double treatment_enrollment_probability_inpartner = chi_sim::Parameters::instance()->getDoubleParameter(TREATMENT_ENROLLMENT_PROBABILITY_INPARTNER);
    double treatment_enrollment_probability_outpartner = chi_sim::Parameters::instance()->getDoubleParameter(TREATMENT_ENROLLMENT_PROBABILITY_OUTPARTNER);
    
    treatmentEnrollmentProb[EnrollmentMethod::UNBIASED] = treatment_enrollment_probability_unbiased;
    treatmentEnrollmentProb[EnrollmentMethod::HRP] = treatment_enrollment_probability_HRP;
    treatmentEnrollmentProb[EnrollmentMethod::FULLNETWORK] = treatment_enrollment_probability_fullnetwork;
    treatmentEnrollmentProb[EnrollmentMethod::INPARTNER] = treatment_enrollment_probability_inpartner;
    treatmentEnrollmentProb[EnrollmentMethod::OUTPARTNER] = treatment_enrollment_probability_outpartner;

    treatmentEnrollmentResidual[EnrollmentMethod::UNBIASED] = 0.;
    treatmentEnrollmentResidual[EnrollmentMethod::HRP] = 0.;
    treatmentEnrollmentResidual[EnrollmentMethod::FULLNETWORK] = 0.;
    treatmentEnrollmentResidual[EnrollmentMethod::INPARTNER] = 0.;
    treatmentEnrollmentResidual[EnrollmentMethod::OUTPARTNER] = 0.;
    
    double opioid_treatment_enrollment_probability_methadone = chi_sim::Parameters::instance()->getDoubleParameter(OPIOID_TREATMENT_ENROLLMENT_PROBABILITY_METHADONE);
    double opioid_treatment_enrollment_probability_naltrexone = chi_sim::Parameters::instance()->getDoubleParameter(OPIOID_TREATMENT_ENROLLMENT_PROBABILITY_NALTREXONE);
    double opioid_treatment_enrollment_probability_buprenorphine = chi_sim::Parameters::instance()->getDoubleParameter(OPIOID_TREATMENT_ENROLLMENT_PROBABILITY_BUPRENORPHINE);
    
    opioidTreatmentEnrollmentProb[DrugName::METHADONE] = opioid_treatment_enrollment_probability_methadone;
    opioidTreatmentEnrollmentProb[DrugName::NALTREXONE] = opioid_treatment_enrollment_probability_naltrexone;
    opioidTreatmentEnrollmentProb[DrugName::BUPRENORPHINE] = opioid_treatment_enrollment_probability_buprenorphine;
    
    opioidTreatmentEnrollmentResidual[DrugName::METHADONE] = 0.;
    opioidTreatmentEnrollmentResidual[DrugName::NALTREXONE] = 0.;
    opioidTreatmentEnrollmentResidual[DrugName::BUPRENORPHINE] = 0.;

    // Output directory in the instance for writing output files
    string output_directory = chi_sim::Parameters::instance()->getStringParameter(OUTPUT_DIRECTORY);
    
    // Root data directory for input files in the emews project
    string data_dir = chi_sim::Parameters::instance()->getStringParameter(DATA_DIR);

//	std::cout << "Output dir: " << output_directory << std::endl;

    // Write all updated props, including any on the UPF line, to an instance file
    std::string props_file = chi_sim::unique_file_name(props.getProperty(OUTPUT_DIRECTORY) + "/model.props");
    FileOut fo(props_file);
    for (auto iter = props.keys_begin(); iter != props.keys_end(); ++iter) {
        fo << (*iter) << " = " << props.getProperty(*iter) << "\n";
    }
    fo.close();

    // TODO put all the data loading into a separate method

    // Load zones (zip codes)
    std::string zones_file = data_dir + "/" + chi_sim::Parameters::instance()->getStringParameter(ZONES_FILE);
    std::string dist_file = data_dir + "/" + chi_sim::Parameters::instance()->getStringParameter(OPIOID_TREATMENT_ZONE_DISTANCE_FILE);
    std::string access_scenario = chi_sim::Parameters::instance()->getStringParameter(OPIOID_TREATMENT_ACCESS_SCENARIO);
    loadZones(zones_file, dist_file, access_scenario, zoneMap);
    
    // Load zone-zone (zipcode) distances file
    std::string zones_distance_file = data_dir + "/" + chi_sim::Parameters::instance()->getStringParameter(ZONES_DISTANCE_FILE);
    loadZonesDistances(zones_distance_file, zoneMap, zoneDistanceMap);

    // personData and personCreator are used to create initial persons and arriving new persons
    //   so we need it regardless of how the initial population is created.
    std::string pwid_file = data_dir + "/" + chi_sim::Parameters::instance()->getStringParameter(PWID_INPUT_FILE);
    std::string pwid_file_type = chi_sim::Parameters::instance()->getStringParameter(PWID_DATA_INPUT_TYPE);
    loadPersonData(pwid_file, personData, pwid_file_type);

    init_stats(output_directory, run, zoneMap);

    // Load viral kinetics data
    ViralKinetics::init(data_dir);
   
    // starting tick: tick at which to start scheduled events. If the model 
    // is resumed from a serialized state then we want to start at the time
    // it was serialized + 1
    double start_at = 1;
    
    // TODO Determine if we want to get the resume (deserialized) workking properly.
    // bool resume =  chi_sim::Parameters::instance()->getBooleanParameter(RESUME_FROM_SAVED);
    // if (resume) {
    //     burnInDays = 0;
    //     std::string fname = chi_sim::Parameters::instance()->getStringParameter(RESUME_FROM_SAVED_FILE);
    //     double serialized_at;
    //     network = read_network<HCPerson>(fname, &read_person, &read_edge, zoneMap, &serialized_at);
    //     unsigned int max_id = 0;
    //     for (auto iter = network->verticesBegin(); iter != network->verticesEnd(); ++iter) {
    //         unsigned int id = (*iter)->id();
    //         if (id > max_id) {
    //             max_id = id;
    //         }
    //         local_persons.emplace(id, (*iter));
    //     }
    //     // The start time for regular actions is at the next integer from the serialization time
    //     start_at = floor(serialized_at) + 1; //.000000000001;
        
    //     // Perform an initial zone census
    //     zoneCensus();
        
    //     personCreator = std::make_shared<PersonCreator>(max_id + 1);
    //     std::cout << "Resuming from " << fname << ", starting at: " << std::setprecision(20) << std::fixed << start_at << std::endl;
    // } else {
    
    personCreator = std::make_shared<PersonCreator>(1);
    network = std::make_shared<Network<HCPerson>>(true);
    int personCount = chi_sim::Parameters::instance()->getIntParameter(INITIAL_PWID_COUNT);

    // Burn-in needs to be set after person creator but before generating persons
    burnInControl(); 

    // If the PWID input file type is ERGM, initialize the network from the input efge 
    if (pwid_file_type == "ERGM"){
        std::cout << "Creating Persons from ERGM."<< std::endl;
        personCreator->create_persons_from_ergm_data(local_persons, personData, zoneMap, network);
    
        std::string pwid_edge_file = data_dir + "/" + chi_sim::Parameters::instance()->getStringParameter(PWID_NETWORK_EDGES_INPUT_FILE);
        load_pwid_edge_data(pwid_edge_file, pwid_edge_data);

        construct_network_from_ergm_data();
    }
    else{  // Using CNEP+ data, the network is created programmatically.
        std::cout << "Creating Persons from CNEP+."<< std::endl;
        personCreator->create_persons(local_persons, personData, zoneMap, network, personCount, false);
    
        performInitialLinking();
    }

    // Check for missing or bad person data
    // TODO implement a more robust data integrety check.
    int missing_zip_count = 0;
    for (auto entry : local_persons) {
        PersonPtr& p = entry.second;
        
        if (p->getZone() == nullptr){
            missing_zip_count++;
        }
    }
    if (missing_zip_count > 0){
        std::cout << "WARNING: total persons with missing zip codes: " << missing_zip_count << std::endl;
    }


    std::cout << "Initial network number of edges: " << network->edgeCount() <<std::endl;
    std::cout << "Initializing Schedule..."<< std::endl;
    
    // Schedule model events
    // Model step
    repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
    runner.scheduleEvent(start_at, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<HCModel>(this, &HCModel::step)));

    // Model end
    runner.scheduleEndEvent(repast::Schedule::FunctorPtr(new repast::MethodFunctor<HCModel>(this, &HCModel::atEnd)));

    // TODO Schedule reorg - we can instead just call each model function in the
    //      step method to ensure the correct order.

    // Zone census schedule
    runner.scheduleEvent(start_at + 0.1, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<HCModel>(this, &HCModel::zoneCensus)));

    // Dynamic network linking schedule
    double linkingTimeWindow = chi_sim::Parameters::instance()->getDoubleParameter(LINKING_TIME_WINDOW);
    runner.scheduleEvent(start_at + 0.2, linkingTimeWindow, repast::Schedule::FunctorPtr(new repast::MethodFunctor<HCModel>(this, &HCModel::performLinking)));

    // DAA Treatment schedule
    treatmentEnrollPerPY = chi_sim::Parameters::instance()->getDoubleParameter(TREATMENT_ENROLLMENT_PER_PY);
    reduced_treatmentEnrollPerPY = chi_sim::Parameters::instance()->getDoubleParameter(REDUCED_TREATMENT_ENROLLMENT_PER_PY);
    double treatmentStartDelay = chi_sim::Parameters::instance()->getDoubleParameter(TREATMENT_ENROLLMENT_START_DELAY);
    double enrollmentStart = burnInDays + treatmentStartDelay;

    // The DAA treatment  strategy may attempt either a total daily enrollment of infected PWID, or a total screening of all PWID
    std::string treatment_enrollment_strategy = chi_sim::Parameters::instance()->getStringParameter(TREATMENT_ENROLLMENT_STRATEGY);
    std::cout << "DAA enrollment strategy: "<< treatment_enrollment_strategy << std::endl;
    if (treatmentEnrollPerPY > 0){
        if (treatment_enrollment_strategy == "INFECTED_ONLY") {
            runner.scheduleEvent(start_at + enrollmentStart + 0.3, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<HCModel>(this, &HCModel::daa_treatment)));
        }
        else { // ALL_PWID
            runner.scheduleEvent(start_at + enrollmentStart + 0.3, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<HCModel>(this, &HCModel::daa_treatment_all_PWID)));
        }
    }
    
    // Opioid Treatment schedule
    opioidTreatmentEnrollPerPY = chi_sim::Parameters::instance()->getDoubleParameter(OPIOID_TREATMENT_ENROLLMENT_PER_PY);
    double opioidTreatmentStartDelay = chi_sim::Parameters::instance()->getDoubleParameter(OPIOID_TREATMENT_ENROLLMENT_START_DELAY);
    double opioidEnrollmentStart = burnInDays + opioidTreatmentStartDelay;

    if (opioidTreatmentEnrollPerPY > 0){
        runner.scheduleEvent(start_at + opioidEnrollmentStart + 0.4, 1, repast::Schedule::FunctorPtr(new repast::MethodFunctor<HCModel>(this, &HCModel::opioid_treatment)));
    }

    // Log the initial network topology
    // double logNetwork = chi_sim::Parameters::instance()->getBooleanParameter(LOG_INITIAL_NETWORK);
    if (chi_sim::Parameters::instance()->contains(LOG_NETWORK_AT)) {
        std::string log_net_at = chi_sim::Parameters::instance()->getStringParameter(LOG_NETWORK_AT);
        boost::char_separator<char> sep(",");
        boost::tokenizer<boost::char_separator<char>> tok(log_net_at, sep);
        std::cout << "Logging Network at: " << log_net_at << "." << std::endl;
        for (auto item : tok) {
            boost::trim(item);
            if (item == "END" || item == "end") {
                double at =  chi_sim::Parameters::instance()->getDoubleParameter("stop.at");
                std::string fname(output_directory + "/net_" + item + ".gml");
                runner.scheduleEndEvent(boost::make_shared<WriteNet>(fname, at, network));
            } else {
                double at = std::stod(item);
                if (at == 0) {
                    std::string fname(output_directory + "/net_initial.gml");
                    write_network(fname, 0, network, &write_person, &write_edge);
                } else {
                    std::string fname(output_directory + "/net_" + item + ".gml");
                    runner.scheduleEvent(at + .1, boost::make_shared<WriteNet>(fname, at, network));
                }
            }
        }
    }

    // write t0 stats
    Statistics::instance()->recordStats(0, run, local_persons);

    // double viral_load = ViralKinetics::instance() -> get_viral_load(VKProfile::ACUTE_INFECTION_CLEARANCE, 4, 21);
    // std::cout << "TEST VIRAL LOAD ACUTE_INFECTION_CLEARANCE, 4, 21: " << viral_load << std::endl;
    
    // Random stream sanity check
    d = repast::Random::instance()->nextDouble();
    std::cout << "HepCEP Model Initialization Complete. Run # " << rnum << ", Random seed: " << seed << ", rand = " << d << std::endl;
}

HCModel::~HCModel() {}

void HCModel::atEnd() {
    Statistics::instance()->close();
}
/**
 * @brief The main model step method.
 * 
 */
void HCModel::step() {
    
    double tick = repast::RepastProcess::instance()->getScheduleRunner().currentTick();

//	std::cout << "t = " << tick << std::endl;

    std::vector<PersonPtr> inActivePersons;

    for (auto entry : local_persons) {
        PersonPtr& person = entry.second;

        if (person->isActive()){
            person->step(network);
        }
        else {
            inActivePersons.push_back(person);
        }
    }

    // Remove inactive persons
    for (PersonPtr person : inActivePersons){
        local_persons.erase(person->id());
        network->removeVertex(person);

        // TODO check if any scheduled Functor is holding onto a PersonPtr
    }

    generateArrivingPersons();

    // Record stats MUST always be last since it resets some values used above.
    Statistics::instance()->recordStats(tick, run, local_persons);
    
}

/**
 * @brief Generate new PWID agents to maintain the constant population size.
 * 
 */
void HCModel::generateArrivingPersons(){
    int totalLost = Statistics::instance()->getDailyLosses();

    double meanArrival = totalLost + netInflow/365.0;

    if (meanArrival <= 0) return;  // Poisson mean must be > 0

    PoissonGen arrival_gen(repast::Random::instance()->engine(),
            boost::random::poisson_distribution<>(meanArrival));

    repast::DefaultNumberGenerator<PoissonGen> gen(arrival_gen);

    int newCount = gen.next();

    personCreator->create_persons(local_persons, personData, zoneMap, network, newCount, true);
}

/**
 * @brief Construct the PWID netork from ERGM input data.
 * 
 */
void HCModel::construct_network_from_ergm_data(){

    // Create a mapping between the ERGM vertex ID and persn pointer
    // so that we have a fast lookup for creating initial edges.
    std::unordered_map<unsigned int, PersonPtr> ergm_vertex_person_map;

    for (auto entry : local_persons) {
        PersonPtr & person = entry.second;
        ergm_vertex_person_map[person->get_ergm_vertex_name()] = person;
    }

    // Create edges between persons using the ERGM vertex ID
    for (auto const& kv : pwid_edge_data){
        // Each key-value pair is the source PWID ERGM ID -> list of target PWID ERGM IDs
        int source_person_vertex_id = kv.first;
        std::vector<int> target_person_vertex_ids = kv.second;  

        // Get the actual source person pointer
        PersonPtr & source_person = ergm_vertex_person_map[source_person_vertex_id];

        // Connect each actual source person to the target persons
        for (auto target_vertex : target_person_vertex_ids){
            PersonPtr & target_person = ergm_vertex_person_map[target_vertex];

            // Link the people (with 100% certainly)
            connect(source_person,target_person);
        }
    }

    // Record the initial in/out degree for each person and save to the HCPersonData.
    // Used to later check sampled HCPersonData in/out degree targets for generaring
    // new PWID during the simulation.
    for (auto& data : personData){
        PersonPtr & person = ergm_vertex_person_map[data.ergm_vertex_name];

        data.drug_inDegree = network->inEdgeCount(person);
        data.drug_outDegree = network->outEdgeCount(person);

        // std::cout << data.ergm_vertex_name << "," << data.age << "," 
        //     << data.drug_inDegree << "," << data.drug_outDegree << std::endl;

    }

    // Set each initial person in/out targets based on the person's initial network structure.
     for (auto entry : local_persons) {
        PersonPtr & person = entry.second;
        person->setDrugReceptDegree(network->inEdgeCount(person));
        person->setDrugGivingDegree(network->outEdgeCount(person));
     }

    // Testing
    // for (HCPersonData data : personData){
    // std::cout << data.ergm_vertex_name << "," << data.age << "," 
    //     << data.drug_inDegree << "," << data.drug_outDegree << std::endl;
    // }
}

/**
 * @brief Create the PWID network programmatically based on PWID in/out degree targets.
 * 
 */
void HCModel::performInitialLinking(){

    double total_edges = network->edgeCount();
    double total_recept_edge_target = 0;
    double total_give_edge_target = 0;

    // Add each person to the network as a vertex.
    for (auto entry : local_persons) {
        PersonPtr & person = entry.second;

        total_recept_edge_target += person->getDrugReceptDegree();
        total_give_edge_target += person->getDrugGivingDegree();
    }

    int iteration = 0;

    //stopping criterion, in terms of actual vs. required number of edges
    double DENSITY_TARGET = 0.99;

    //maximal number of iterations when forming the network, to prevent too much work
    int    MAXITER = 30;
    while ((total_edges/total_recept_edge_target < DENSITY_TARGET) &&
                (total_edges/total_give_edge_target < DENSITY_TARGET) &&
                (iteration < MAXITER)) {

		// std::cout << "> Total edges: " << total_edges << ". target in: " << total_recept_edge_target
		// 		<< ". target out: " << total_give_edge_target << std::endl;

        zoneCensus();
        performLinking();
        performLinking();

        total_edges = network->edgeCount();
        iteration ++;
    }
	std::cout << " Final Total edges: " << total_edges << ". target in: " << total_recept_edge_target
				<< ". target out: " << total_give_edge_target << std::endl;

	// if (iteration == MAXITER) {
	// 	std::cout << "Initial linking reached the maximum number of iterations (" << MAXITER << ")" << std::endl;
	// }

}

/**
 * @brief Programmaticaly link PWID in the network if they need more peers.
 * 
 */
void HCModel::performLinking(){

    // Effective zone populations are the people in each zip code (zone) that
    //   need more network connections based on their predefined in and out 
    //   degrees in the PWID data.

    // Loop over every zone (zip code)
    for (auto entry1 : effectiveZonePopulation){
        unsigned int zipcode_1 = entry1.first;
        std::vector<PersonPtr> pop_1 = entry1.second;
        int s1 = pop_1.size();
        const ZonePtr & zone1 = zoneMap[zipcode_1];

        // Skip if zone population is zero
        if (s1 == 0){
            continue;
        }

        // Loop over every zone (zip code) again
        for (auto entry2 : effectiveZonePopulation){
            unsigned int zipcode_2 = entry2.first;
            std::vector<PersonPtr> pop_2 = entry2.second;
            int s2 = pop_2.size();
            const ZonePtr & zone2 = zoneMap[zipcode_2];

            // Skip if zone population is zero
            if (s2 == 0){
                continue;
            }
            
            double rate = 0;
            double distance = zoneDistanceMap[zipcode_1][zipcode_2];

            if (distance > interactionHomeCutoff){
                if (zone1->getDrugMarket() ==  zone2->getDrugMarket()){
                    rate = (interactionRateDrugSites * s1 * s2) +
                            (interactionRateExzone * s1 * s2) / std::pow(distance, 2);
                }
                else{
                    rate = (interactionRateExzone * s1 * s2)/std::pow(distance, 2);
                }
            }
            else{
                rate = (interactionRateDrugSites * s1 * s2) + (interactionRateConst * s1 * s2);
            }
     
            if (rate == 0.0) {
                continue;
            }

            repast::ExponentialGenerator generator =
                    repast::Random::instance()->createExponentialGenerator (rate);
                    
            double t = 0;

            t += generator.next();
            
            while (t <= linkingTimeWindow){                
                double d1 = repast::Random::instance()->nextDouble();
                double d2 = repast::Random::instance()->nextDouble();

                int a1_idx = std::round(d1 * (s1-1));
                int a2_idx = std::round(d2 * (s2-1));

                if(zipcode_1 == zipcode_2 && a1_idx == a2_idx) {
                    t += generator.next();  
                    continue;
                }

                PersonPtr & person1 = pop_1[a1_idx];
                PersonPtr & person2 = pop_2[a2_idx];

                tryConnect(person1,person2);
                                
                t += generator.next();   
            }
        }
    }
}

/**
 * @brief Attempt a bi-directional network connection pair between person1 & person2
 * 
 * @param person1 
 * @param person2 
 * 
 * TODO move to HCPerson ? 
 */
void HCModel::tryConnect(const PersonPtr& person1, const PersonPtr& person2){
    
    // Check conditions for adding a directed edge from person1 -> person2
    if (network->inEdgeCount(person2) >= person2->getDrugReceptDegree()) {
        return;
    }
    if (network->outEdgeCount(person1) >= person1->getDrugGivingDegree()) {
        return;
    }

    double roll = repast::Random::instance()->nextDouble();
    if (person1->getDemographicDistance(person2) * homophily > roll) {
        return;
    }

    connect(person1,person2);

    // Check conditions for adding a directed edge from person2 -> person1
    if (network->inEdgeCount(person1) >= person1->getDrugReceptDegree()) {
        return;
    }
    if (network->outEdgeCount(person2) >= person2->getDrugGivingDegree()) {
        return;
    }

    connect(person2,person1);
}

/**
 * @brief Add a directed PWID network connection from person1 to person2.
 * 
 * @param person1 
 * @param person2 
 */
void HCModel::connect(const PersonPtr& person1, const PersonPtr& person2){
    double tick = repast::RepastProcess::instance()->getScheduleRunner().currentTick();

    double dist = zoneDistanceMap[person1->getZipcode()][person2->getZipcode()];

    EdgePtrT<HCPerson> edge = network->addEdge(person1, person2);
    edge->putAttribute("distance", dist);

    // Schedule the p1 -> p2 edge removal in the future
    double edgeLifespan = Distributions::instance()->getNetworkLifespanRandom();
        
    double endTime = tick + edgeLifespan;
    edge->putAttribute("ends_at", endTime);

    repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
    EndRelationshipFunctor* endRelationshipEvent1 = new EndRelationshipFunctor(person1,person2,network);
    runner.scheduleEvent(endTime, repast::Schedule::FunctorPtr(endRelationshipEvent1));

}

/**
 * @brief Determine how many IDUs in each zones are available to form new network connections.
 * 
 */
void HCModel::zoneCensus(){
    effectiveZonePopulation.clear();

    totalIDUPopulation  = 0;

    for (auto entry : local_persons) {
        PersonPtr & person = entry.second;
        totalIDUPopulation += 1;

        unsigned int zipcode = person->getZipcode();

        // If the person has a zone/zip that's not in the zones map, then don't count the person for linking
        // NOTE: This should not normally occur but we check to avoid crash.
        if (zoneMap[zipcode] == nullptr){
            // std::cout << "WARNING: Zone Census - zipcode not found: " << zipcode << std::endl;
            continue;
        }

        // Record "effective" agents that have available in or out degree connections
        std::vector<PersonPtr> & myEffAgents = effectiveZonePopulation[zipcode];

        unsigned int inCount = network->inEdgeCount(person);
        unsigned int outCount = network->outEdgeCount(person);

        if (inCount < person->getDrugReceptDegree() || outCount < person->getDrugGivingDegree()){
            myEffAgents.push_back(person);
        }
    }
}

/*
 * activate the burn-in mode
 * - should be called before the agents are created
 */
void HCModel::burnInControl() {
    if(burnInDays <= 0) {
        return;
    }

    Statistics::instance()->setBurninMode(true);
    personCreator->setBurnInPeriod(true, burnInDays);

    double tick = repast::RepastProcess::instance()->getScheduleRunner().currentTick();
//	std::cout << "Scheduling burnin end for " << tick << " + " <<  burnInDays << std::endl;
    repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
    runner.scheduleEvent(tick + burnInDays,
            repast::Schedule::FunctorPtr(new repast::MethodFunctor<HCModel>(this, &HCModel::burnInEnd)));

}

void HCModel::burnInEnd() {

    Statistics::instance()->setBurninMode(false);
    personCreator->setBurnInPeriod(false, -1);

    for (auto entry : local_persons) {
            PersonPtr & person = entry.second;

            // Reduce the person's age based on the burn in period.
            double age = person->getAge();
            person->setAge(age - (burnInDays / 365.0));

            Statistics::instance()->logStatusChange(LogType::ACTIVATED, person, "");
    }

    std::cout << "**** Finished burn-in. Duration: " << burnInDays << " ****" << std::endl;
}

/**
 * @brief DAA Treatment Enrollment.
 *
 *  DAA enrollment using a daily enrollment target for enrollment of HCV+ PWID, as described in the 2022 Plos One.
 * 
 */
void HCModel::daa_treatment(){
    double reduceTreatmentTime = chi_sim::Parameters::instance()->getDoubleParameter(TREATMENT_ENROLLMENT_REDUCE_AT);
    double tick = repast::RepastProcess::instance()->getScheduleRunner().currentTick();
    
     // Default treatment enrollment is just the specified level
    double enrollment_per_py = treatmentEnrollPerPY;

    // Used the reduced treatment level if set
    if (reduceTreatmentTime !=0 && tick >= reduceTreatmentTime){
        enrollment_per_py = reduced_treatmentEnrollPerPY;
    }

    if (enrollment_per_py == 0){  // Can just return if treatment level set to zero
        return;
    }
    
    double treatmentMeanDaily = totalIDUPopulation * enrollment_per_py / 365.0;

    PoissonGen treat_gen(repast::Random::instance()->engine(), boost::random::poisson_distribution<>(treatmentMeanDaily));
    repast::DefaultNumberGenerator<PoissonGen> gen(treat_gen);

    double todaysTotalEnrollment = gen.next();

    if (todaysTotalEnrollment <= 0) {
        return; //do nothing.  occurs when we previously over-enrolled
    }

    std::vector<PersonPtr> candidates;
    for (auto entry : local_persons) {
            PersonPtr & person = entry.second;

            if (person->isTreatable()){
                candidates.push_back(person);
            }          
    }

    if (candidates.size() == 0){
        return;
    }

    for (EnrollmentMethod mthd : EnrollmentMethod::values()){
        double enrollmentTarget = todaysTotalEnrollment *
                treatmentEnrollmentProb[mthd]; // + treatmentEnrollmentResidual[mthd];

//        treatmentEnrollmentResidual[mthd] = enrollmentTarget;  //update.  might be fractional increase

        if(enrollmentTarget < 1) {
            continue;
        }

        // Use Repast random to ensure repeatability
//		repast::shuffleList(candidates);

        // TODO maybe figure out how to use random_shuffle with the Repast generator

        std::random_shuffle(candidates.begin(), candidates.end(), myrandom);

        // Person IDs to be enrolled
        std::vector<PersonPtr> enrolled;

      
        treatment_selection_infected_only(mthd, candidates, enrolled, enrollmentTarget);
      
        //carried over from day to the next day.  this can give below 0
//        treatmentEnrollmentResidual[mthd] = (enrollmentTarget - enrolled.size());

        for (PersonPtr person: enrolled){
            person->startTreatment();
        }
    }
}

/**
 * @brief DAA Treatment Screening
 *
 * DAA screening and treatment enrollment that attempts to screen a specified number of all PWID per day.
 * 
 */
void HCModel::daa_treatment_all_PWID(){
    double reduceTreatmentTime = chi_sim::Parameters::instance()->getDoubleParameter(TREATMENT_ENROLLMENT_REDUCE_AT);
    double tick = repast::RepastProcess::instance()->getScheduleRunner().currentTick();

    // Days interval for HCV screening per person (cannot be screened more than once per interval)
    int screening_interval_days = chi_sim::Parameters::instance()->getIntParameter(HCV_SCREENING_INTERVAL);

    // Default treatment enrollment is just the specified level
    double enrollment_per_py = treatmentEnrollPerPY;

    // Used the reduced treatment level if set
    if (reduceTreatmentTime !=0 && tick >= reduceTreatmentTime){
        enrollment_per_py = reduced_treatmentEnrollPerPY;
    }

    if (enrollment_per_py == 0){  // Can just return if treatment level set to zero
        return;
    }
    
    // The mean number of PWID to screen per day
    double screening_mean_daily = totalIDUPopulation * enrollment_per_py / 365.0;

    PoissonGen treat_gen(repast::Random::instance()->engine(), boost::random::poisson_distribution<>(screening_mean_daily));
    repast::DefaultNumberGenerator<PoissonGen> gen(treat_gen);

    // The number of PWID to screen today.
    double number_to_screen = gen.next();

    if (number_to_screen <= 0) {
        return; 
    }

    // Persons who can be screened today (potentially everyone)
    std::vector<PersonPtr> screen_candidates;
    for (auto entry : local_persons) {
            PersonPtr & person = entry.second;

            // TODO Finish this
            // Consider the PWID's syring sharing behavior for screening purposes.
            // Only need to screen at-risk PWID, which are those PWID with syring-sharing
            //      network connections, and that have a non-zero daily injection intensity, and fraction recept sharing?
            int num_out = network->outEdgeCount(person);
            int num_in = network->inEdgeCount(person);

            bool no_edges = false;
            if (num_out == 0 && num_in == 0){
                no_edges = true;
            }
            double injection_rate = person->getInjectionIntensity();

            // If person has no network edges OR no injection frequency, dont screen.
            if (no_edges || injection_rate == 0){
                continue;
            }

            // Don't consider PWID already in DAA treatment or who have been tested already within the screening interval.
            if (!person->isInTreatment() && person->is_eligible_for_hcv_screeening(tick, screening_interval_days)){
                screen_candidates.push_back(person);
            }
    }

    if (screen_candidates.empty()){
        std::cout << "Screen candidates empty!" << std::endl;
        return;
    }

    std::cout << "Screen candidates size: " << screen_candidates.size() << std::endl;

    // Divide up the screen/treatment approaches based on the enrollment method.
    // NOTE We still ust the treatmentEnrollmentProb  property to split up the screening
    //      methods.
    // NOTE Does not consider daily residuals like the oringal enrollment methods.
    for (EnrollmentMethod mthd : EnrollmentMethod::values()){
        double screening_Target = number_to_screen * treatmentEnrollmentProb[mthd];

        if(screening_Target < 1) {
            continue;
        }

        // Use Repast random to ensure repeatability
//		repast::shuffleList(screen_candidates);

        // TODO maybe figure out how to use random_shuffle with the Repast generator

        std::random_shuffle(screen_candidates.begin(), screen_candidates.end(), myrandom);

        // Person IDs to be enrolled
        //  This works just like the original DAA treatment enrollment, such that all persons
        //  that end up in this list are actually enrolled.
        std::vector<PersonPtr> enrolled;

        treatment_selection_all_PWID(mthd, screen_candidates, enrolled, screening_Target);

        for (PersonPtr person: enrolled){
            person->startTreatment();
        }
    }
}

/**
* Treatment selection when screeing all PWID, including non-infected persons. This
*   approach is based on HCV screening efforts, by which we have limited resources
*   to screen a certain number of PWID per day, and only those PWID who are screened
*   and HCV+ can be enrolled in DAA treatment.
*
* Here, candidate persons are all PWID who are not currently in treatment.
*
*/
void HCModel::treatment_selection_all_PWID(EnrollmentMethod enrMethod,
        std::vector<PersonPtr>& candidates, std::vector<PersonPtr>& enrolled,
        double screening_target){

    std::vector<PersonPtr>::iterator iter;

    int num_screened = 0;

    double tick = repast::RepastProcess::instance()->getScheduleRunner().currentTick();
        
    if(enrMethod == EnrollmentMethod::UNBIASED) {
        for (iter = candidates.begin(); iter != candidates.end(); ){
            PersonPtr person = *iter;
                         
            // Continue screening if less than the target screening number.
            if (num_screened < screening_target){
                person->set_last_hcv_screen_date(tick);
                if(person->isTreatable()) {           
                    enrolled.push_back(person);       // Enroll person
                }

                // At this point, whether the person is enrolled or not, remove the
                // person from the candidates list, and increment the num screened counter.
                iter = candidates.erase(iter);    // Remove person from candidates
                ++iter;
                num_screened++;
            }
            // Otherwise the screening target is met, so stop screening.
            else{
                break;
            }
        }
    }
    else if(enrMethod == EnrollmentMethod::FULLNETWORK) {
        for (iter = candidates.begin(); iter != candidates.end(); ){
            PersonPtr person = *iter;

            // Continue screening if less than the target screening number.
            if (num_screened < screening_target){

                if (person == nullptr){
                    // std::cout << "WARNING: null pointer on person in FULLNETWORK screening!" << std::endl;
                    return;  // Means the candiates list is empty?

                }

                person->set_last_hcv_screen_date(tick);
                if(person->isTreatable()) {           
                    enrolled.push_back(person);       // Enroll person

                    // Try to enroll all connected persons (if treatable)
                    // NOTE: Does not consider connected persons in screening target limit
                    std::vector<EdgePtrT<HCPerson>> inEdges;
                    std::vector<EdgePtrT<HCPerson>> outEdges;

                    network->inEdges(person,inEdges);
                    network->outEdges(person,outEdges);

                    for (EdgePtrT<HCPerson> edge : inEdges){
                        PersonPtr other = edge->v1();   // Other agent is edge v1

                        if (other == nullptr){
                            std::cout << "WARNING: null pointer on in edge for person: " << person->id() << std::endl;
                            continue;
                        }

                        other->set_last_hcv_screen_date(tick);
                        if (other->isTreatable()){
                            enrolled.push_back(other);       // Enroll person
                        }
                    }
                    for (EdgePtrT<HCPerson> edge : outEdges){
                        PersonPtr other = edge->v2();  // Other agent is edge v2

                        if (other == nullptr){
                            std::cout << "WARNING: null pointer on out edge for person: " << person->id() << std::endl;
                            continue;
                        }

                        other->set_last_hcv_screen_date(tick);
                        if (other->isTreatable()){
                            enrolled.push_back(other);       // Enroll person
                        }
                    }
                }

                // At this point, whether the person is enrolled or not, remove the
                // person from the candidates list, and increment the num screened counter.
                iter = candidates.erase(iter);    // Remove person from candidates
                ++iter;
                num_screened++;
            }
            // Otherwise the screening target is met, so stop screening.
            else{
                break;
            }
        }
    }

    // Here we only want to check against PWID in HRP and not check against PWID not in HRP
    else if(enrMethod == EnrollmentMethod::HRP) {
        for (iter = candidates.begin(); iter != candidates.end(); ){
            PersonPtr person = *iter;
                         
            // In reality here we want to just subset the candidates on PWID in HRP so that
            //   this is simulating a total effort just with HRPs.
            if (!person->isInHarmReduction()){
                ++iter;
                continue;
            }

            // Continue screening if less than the target screening number.
            if (num_screened < screening_target){
                person->set_last_hcv_screen_date(tick);
                if(person->isTreatable()) {           
                    enrolled.push_back(person);       // Enroll person
                }

                // At this point, whether the person is enrolled or not, remove the
                // person from the candidates list, and increment the num screened counter.
                iter = candidates.erase(iter);    // Remove person from candidates
                ++iter;
                num_screened++;
            }
            // Otherwise the screening target is met, so stop screening.
            else{
                break;
            }
        }
    }

    // else if(enrMethod == EnrollmentMethod::INPARTNER || enrMethod == EnrollmentMethod::OUTPARTNER) {        
    //     for (iter = candidates.begin(); iter != candidates.end();   ){
    //         PersonPtr person = *iter;
                         
    //         // Continue enrolling while enrollment target is not met.
    //         if (enrolled.size() < screening_target){
    //             if(person->getTestedHCV()) {          // HCV test returns true if person can be treated now
    //                 enrolled.push_back(person);       // Enroll person
    //                 iter = candidates.erase(iter);    // Remove person from candidates

    //                 // Try to enroll connected persons
    //                 if(enrMethod == EnrollmentMethod::INPARTNER) {
    //                     std::vector<EdgePtrT<HCPerson>> inEdges;
    //                     network->inEdges(person,inEdges);

    //                     for (EdgePtrT<HCPerson> edge : inEdges){
    //                         PersonPtr other = edge->v1();   // Other agent is edge v1
    //                         if (other->getTestedHCV()){
    //                             enrolled.push_back(other);   // Enroll person
    //                             break; //only one in-edge person
    //                         }
    //                     }
    //                 }
    //                 else {  // outpartner
    //                     std::vector<EdgePtrT<HCPerson>> outEdges;
    //                     network->outEdges(person,outEdges);

    //                     for (EdgePtrT<HCPerson> edge : outEdges){
    //                         PersonPtr other = edge->v2();  // Other agent is edge v2
    //                         if (other->getTestedHCV()){
    //                             enrolled.push_back(other);   // Enroll person
    //                             break; //only one out-edge person
    //                         }
    //                     }
    //                 }
    //             }
    //             else {
    //                 ++iter;
    //             }
    //         }
    //         // Otherwise the enrollment target is met, so stop enrolling.
    //         else{
    //             break;
    //         }
    //     }
    // }
}

/**
* Treatment enrollment when only checking infected persons. This approach is based
*  on DAA treatment enrollment resources, which means that the model is always aware
*  of which PWID is treatable, but can only enroll a certain number of PWID in treatment
*  per day.
*
* Here, candidate persons are all PWID who have already been checked for treatment
*  eligibility, meaning they are not currently in treatment AND are also HCV+.
*/
void HCModel::treatment_selection_infected_only(EnrollmentMethod enrMethod,
        std::vector<PersonPtr>& candidates, std::vector<PersonPtr>& enrolled,
        double enrollmentTarget){

    std::vector<PersonPtr>::iterator iter;
        
    if(enrMethod == EnrollmentMethod::UNBIASED) {
        for (iter = candidates.begin(); iter != candidates.end();   ){
            PersonPtr person = *iter;
                         
            // Continue enrolling while enrollment target is not met.
            if (enrolled.size() < enrollmentTarget){
                if(person->getTestedHCV()) {          // HCV test returns true if person can be treated now
                    enrolled.push_back(person);       // Enroll person
                    iter = candidates.erase(iter);    // Remove person from candidates
                }
                else {
                    ++iter;
                }
            }
            // Otherwise the enrollment target is met, so stop enrolling.
            else{
                break;
            }
        }
    }
    else if(enrMethod == EnrollmentMethod::HRP) {
        for (iter = candidates.begin(); iter != candidates.end();   ){
            PersonPtr person = *iter;
                         
            // Continue enrolling while enrollment target is not met.
            if (enrolled.size() < enrollmentTarget){
                // HCV test returns true if person can be treated now / only enroll HRP
                if(person->getTestedHCV() && person->isInHarmReduction()) {  
                    enrolled.push_back(person);       // Enroll person
                    iter = candidates.erase(iter);    // Remove person from candidates
                }
                else {
                    ++iter;
                }
            }
            // Otherwise the enrollment target is met, so stop enrolling.
            else{
                break;
            }
        }
    }
    else if(enrMethod == EnrollmentMethod::FULLNETWORK) {
        for (iter = candidates.begin(); iter != candidates.end();   ){
            PersonPtr person = *iter;
                         
            // Continue enrolling while enrollment target is not met.
            if (enrolled.size() < enrollmentTarget){
                // HCV test returns true if person can be treated now
                if(person->getTestedHCV() && person->isInHarmReduction()) {  
                    enrolled.push_back(person);       // Enroll person
                    iter = candidates.erase(iter);    // Remove person from candidates
                    
                    // Try to enroll all connected persons
                    std::vector<EdgePtrT<HCPerson>> inEdges;
                    std::vector<EdgePtrT<HCPerson>> outEdges;
                    
                    network->inEdges(person,inEdges);
                    network->outEdges(person,outEdges);
                    
                    for (EdgePtrT<HCPerson> edge : inEdges){
                        PersonPtr other = edge->v1();   // Other agent is edge v1
                        if (other->getTestedHCV()){
                            enrolled.push_back(other);       // Enroll person
                        }
                    }
                    for (EdgePtrT<HCPerson> edge : outEdges){
                        PersonPtr other = edge->v2();  // Other agent is edge v2
                        if (other->getTestedHCV()){
                            enrolled.push_back(other);       // Enroll person
                        }
                    }
                }
                else {
                    ++iter;
                }
            }
            // Otherwise the enrollment target is met, so stop enrolling.
            else{
                break;
            }
        }
    }
    else if(enrMethod == EnrollmentMethod::INPARTNER || enrMethod == EnrollmentMethod::OUTPARTNER) {        
        for (iter = candidates.begin(); iter != candidates.end();   ){
            PersonPtr person = *iter;
                         
            // Continue enrolling while enrollment target is not met.
            if (enrolled.size() < enrollmentTarget){
                if(person->getTestedHCV()) {          // HCV test returns true if person can be treated now
                    enrolled.push_back(person);       // Enroll person
                    iter = candidates.erase(iter);    // Remove person from candidates

                    // Try to enroll connected persons
                    if(enrMethod == EnrollmentMethod::INPARTNER) {
                        std::vector<EdgePtrT<HCPerson>> inEdges;
                        network->inEdges(person,inEdges);

                        for (EdgePtrT<HCPerson> edge : inEdges){
                            PersonPtr other = edge->v1();   // Other agent is edge v1
                            if (other->getTestedHCV()){
                                enrolled.push_back(other);   // Enroll person
                                break; //only one in-edge person
                            }
                        }
                    }
                    else {  // outpartner
                        std::vector<EdgePtrT<HCPerson>> outEdges;
                        network->outEdges(person,outEdges);

                        for (EdgePtrT<HCPerson> edge : outEdges){
                            PersonPtr other = edge->v2();  // Other agent is edge v2
                            if (other->getTestedHCV()){
                                enrolled.push_back(other);   // Enroll person
                                break; //only one out-edge person
                            }
                        }
                    }
                }
                else {
                    ++iter;
                }
            }
            // Otherwise the enrollment target is met, so stop enrolling.
            else{
                break;
            }
        }
    }
}

void start_opioid_treatment(PersonPtr person, DrugName drug_name) {
    std::shared_ptr<OpioidTreatmentDrug> drug = OpioidTreatmentDrugs::instance()->getDrug(drug_name);
	std::shared_ptr<OpioidTreatment> treatment = make_shared<OpioidTreatment>(person->getZone(), drug);
	bool success = treatment->treat(person);
	if (success) {
		repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
		// -0.0001 so goes on / off before regularly scheduled actions
		double at = runner.currentTick() + drug->duration() - 0.0001;
    	runner.scheduleEvent(at, boost::make_shared<OpioidContinueTreatmentEvent>(person, treatment));
        
        person->setLastOpioidTreatmentStartTime(runner.currentTick());
        Statistics::instance()->logStatusChange(LogType::STARTED_OPIOID_TREATMENT, person, drug->label());
	}
    else{
        // TODO log treatment failure info?
    }
}

/*
 * Opioid Treatment Enrollment
 *
 */
void HCModel::opioid_treatment(){
    
    double treatmentMeanDaily = totalIDUPopulation * opioidTreatmentEnrollPerPY / 365.0;

    PoissonGen treat_gen(repast::Random::instance()->engine(), boost::random::poisson_distribution<>(treatmentMeanDaily));
    repast::DefaultNumberGenerator<PoissonGen> gen(treat_gen);

    double todaysTotalEnrollment = gen.next();

    if (todaysTotalEnrollment <= 0) {
        return; //do nothing.  occurs when we previously over-enrolled
    }

    std::vector<PersonPtr> candidates;
    for (auto entry : local_persons) {
            PersonPtr & person = entry.second;

//          if (person->isTreatable()){
            if(!person->isInOpioidTreatment()) {
                candidates.push_back(person);
            }
    }

    if (candidates.size() == 0){
        return;
    }
    
    // Drugs that are available for treatment
    // TODO maybe these need to be customized though
    for (DrugName drug : DRUG_NAMES){
        double enrollmentTarget = todaysTotalEnrollment *
                opioidTreatmentEnrollmentProb[drug] + opioidTreatmentEnrollmentResidual[drug];
        
        opioidTreatmentEnrollmentResidual[drug] = enrollmentTarget;  //update.  might be fractional increase

        if(enrollmentTarget < 1) {
            continue;
        }

        // Use Repast random to ensure repeatability
//		repast::shuffleList(candidates);

        // TODO maybe figure out how to use random_shuffle with the Repast generator

        std::random_shuffle(candidates.begin(), candidates.end(), myrandom);

        // Person IDs to be enrolled
        std::vector<PersonPtr> enrolled;

        // TODO simple unbiased
        opioidTreatmentSelection(drug, candidates, enrolled, enrollmentTarget);

        //carried over from day to the next day.  this can give below 0
        opioidTreatmentEnrollmentResidual[drug] = (enrollmentTarget - enrolled.size());

        for (PersonPtr person: enrolled){
            start_opioid_treatment(person, drug);
        }
    }
}

void HCModel::opioidTreatmentSelection(DrugName drug,
        std::vector<PersonPtr>& candidates, std::vector<PersonPtr>& enrolled,
        double enrollmentTarget){

    std::vector<PersonPtr>::iterator iter;
        
    for (iter = candidates.begin(); iter != candidates.end();   ){
        PersonPtr person = *iter;
                        
        // Continue enrolling while enrollment target is not met.
        if (enrolled.size() < enrollmentTarget){
//            if(!person->isInOpioidTreatment()) {          
                enrolled.push_back(person);       // Enroll person
                iter = candidates.erase(iter);    // Remove person from candidates
//            }
//            else {
//                ++iter;
//            }
        }
        // Otherwise the enrollment target is met, so stop enrolling.
        else{
            break;
        }
    }
}

// random generator function used in std lib functions that need a random generator
int myrandom (int i) {
    repast::IntUniformGenerator gen = repast::Random::instance()->createUniIntGenerator(0, i-1);
    return gen.next();
}

} /* namespace hepcep */
