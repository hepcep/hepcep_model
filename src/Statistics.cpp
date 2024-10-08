/*
 * Statistics.cpp
 */
#include <iostream>
#include <memory>

#include "repast_hpc/RepastProcess.h"

#include "Statistics.h"
#include "HCPerson.h"

namespace hepcep {

const std::string POPULATION = "population";
const std::string INFECTED = "infected";
const std::string HCV_ABPOS = "hcvabpos";
const std::string IN_TREATMENT = "intreatment";
const std::string INFECTED_TODAY = "infected_daily";
const std::string IN_OPIOID_TREATMENT = "inopioidtreatment";

const std::string IN_OPIOID_TREATMENT_M = "inopioidtreatment_m";
const std::string IN_OPIOID_TREATMENT_B = "inopioidtreatment_b";
const std::string IN_OPIOID_TREATMENT_N = "inopioidtreatment_n";

const std::string CURED = "cured";
const std::string ACUTE = "acute";
const std::string CHRONIC = "chronic";
const std::string SUSCEPTIBLE = "susceptible";
const std::string RECOVERED = "recovered";

const std::string PREVALENCE = "prevalence";
const std::string RNA_PREVALENCE = "RNApreval";
const std::string FRACTION = "fraction";

const std::string VK_NONE = "vk_none";
const std::string VK_ACUTE_INFECT_CLEAR = "vk_acute_infect_clear";
const std::string VK_ACUTE_INFECT_INCOM = "vk_acute_infect_incomplete";
const std::string VK_ACUTE_INFECT_PERSI = "vk_acute_infect_persist";
const std::string VK_REINFECT_HIGH_CLEAR = "vk_reinfect_high_clear";
const std::string VK_REINFECT_LOW_CLEAR = "vk_reinfect_low_clear";
const std::string VK_REINFECT_CHRONIC = "vk_reinfect_chronic";  
const std::string VK_TREATMENT = "vk_treatment";

void EventCounts::reset() {
	activations_daily = cured_daily = losses_daily = aggregate_posttreat = 
        incidence_daily = incidence_daily_chronic =	treatment_recruited_daily = 
        aggregate_courses = opioid_treatment_recruited_daily = 0;
}

void EventCounts::writeHeader(FileOut& out) {
	out << "," << "activations_daily,cured_daily,aggregate_posttreat,losses_daily,incidence_daily,incidence_daily_chronic,treatment_recruited_daily,aggregate_courses,opioid_treatment_recruited_daily";	
}

void EventCounts::write(FileOut& out) {
	out << "," << activations_daily << "," << cured_daily << "," << aggregate_posttreat << "," <<
            losses_daily << "," << incidence_daily << "," << incidence_daily_chronic << "," << 
            treatment_recruited_daily << "," << aggregate_courses << "," << opioid_treatment_recruited_daily;
}

void MeanStats::calcMean() {
	age /= count;
	career /= count;
	daily_inj /= count;
	in_deg /= count;
	out_deg /= count;
	sharing /= count;

	transmissibility /= count_trans;
	viral_load /= count_trans;
}

void MeanStats::reset() {
	age = career = daily_inj = in_deg = out_deg = sharing = transmissibility = viral_load = 0;
	count = 0;
	count_trans = 0;
}

void MeanStats::increment(std::shared_ptr<HCPerson> person) {
	age += person->getAge();
	career += person->getAge() - person->getAgeStarted();
	daily_inj += person->getInjectionIntensity();
	in_deg += person->getDrugReceptDegree();
	out_deg += person->getDrugGivingDegree();
	sharing += person->getFractionReceptSharing();
	++count;

	// Log transmissibility and viral load for infected people
	if (person->isHcvRNA()){
		transmissibility += person->get_transmissibility();
		viral_load += person->get_viral_load();
		++count_trans;
	}
}

void MeanStats::write(FileOut& out) {
	out << "," << age << "," << career << "," << daily_inj << "," << in_deg << "," << out_deg << ","
			<< sharing << "," << transmissibility << "," << viral_load;
}

void MeanStats::writeHeader(FileOut& out) {
	out << ",mean_age_ALL,mean_career_ALL,mean_daily_inj_ALL,mean_in_deg_ALL,mean_out_deg_ALL,mean_sharing_ALL,mean_transmissibility_ALL,mean_viral_load_ALL";
}

Statistics* Statistics::instance_ = nullptr;

void Statistics::init(const std::string& fname, const std::string& events_fname,
		 const std::string& personsFilename, const std::string& needle_sharing_filename, 
         std::map<unsigned int,ZonePtr> zoneMap, std::shared_ptr<Filter<LogType>>& filter,
		 int run_number) {
	if (Statistics::instance_ != nullptr) {
		delete Statistics::instance_;
	}

	instance_ = new Statistics(fname, events_fname, personsFilename, needle_sharing_filename, 
        zoneMap, filter, run_number);
}

void init_metrics(std::vector<std::string>& metrics) {
	// TODO could use model.props to define these, if really needed.
	// NOTE: Only need the _ALL metric for most recent analysis
	// for (auto& gender : Gender::values()) {
	// 	metrics.push_back(GENDER_INFIX + gender.stringValue());
	// }

	// for (auto& state : HCVState::values()) {
	// 	metrics.push_back(HCV_INFIX + state.stringValue());
	// }

	// for (auto& race : Race::values()) {
	// 	metrics.push_back(RACE_INFIX + race.stringValue());
	// }

	// for (auto& dec : AgeDecade::values()) {
	// 	metrics.push_back(AGEDEC_INFIX + dec.stringValue());
	// }

	// for (auto& grp : AgeGroup::values()) {
	// 	metrics.push_back(AGEGRP_INFIX + grp.stringValue());
	// }

	// for (auto& at : AreaType::values()) {
	// 	metrics.push_back(AREATYPE_INFIX + at.stringValue());
	// }

	// for (auto& hr : HarmReduction::values()) {
	// 	metrics.push_back(SYRSRC_INFIX + hr.stringValue());
	// }

	metrics.push_back("_ALL");
}

Statistics::Statistics(const std::string& fname, const std::string& events_fname,
		const std::string& personsFilename, const std::string& needle_sharing_filename, 
        std::map<unsigned int,ZonePtr> zoneMap, std::shared_ptr<Filter<LogType>>& filter, 
        int run_number) :
        		stats(), metrics(), log_events(), means(), event_counts(),
                out(fname), events_out(events_fname), personsOut(personsFilename), 
                        needle_sharing_out(needle_sharing_filename),
						burninMode(false), filter_(filter), run_number_(run_number), 
                        needle_sharing_map() {

	init_metrics(metrics);

	stats.emplace(POPULATION, AggregateStats(POPULATION, metrics));
	stats.emplace(INFECTED, AggregateStats(INFECTED, metrics, &filter_hcv_rna));
	stats.emplace(HCV_ABPOS, AggregateStats(HCV_ABPOS, metrics, &filter_hcv_abpos));
	stats.emplace(IN_TREATMENT, AggregateStats(IN_TREATMENT, metrics, &filter_in_treatment));
	stats.emplace(INFECTED_TODAY, AggregateStats(INFECTED_TODAY, metrics, &filter_infected_today));
    
    stats.emplace(IN_OPIOID_TREATMENT, AggregateStats(IN_OPIOID_TREATMENT, metrics, &filter_in_opioid_treatment));
    stats.emplace(IN_OPIOID_TREATMENT_M, AggregateStats(IN_OPIOID_TREATMENT_M, metrics, &filter_in_opioid_treatment_M));
    stats.emplace(IN_OPIOID_TREATMENT_B, AggregateStats(IN_OPIOID_TREATMENT_B, metrics, &filter_in_opioid_treatment_B));
    stats.emplace(IN_OPIOID_TREATMENT_N, AggregateStats(IN_OPIOID_TREATMENT_N, metrics, &filter_in_opioid_treatment_N));

	stats.emplace(SUSCEPTIBLE, AggregateStats(SUSCEPTIBLE, metrics, &filter_hcv_state_susceptible));
	stats.emplace(CURED, AggregateStats(CURED, metrics, &filter_hcv_state_cured));
	stats.emplace(CHRONIC, AggregateStats(CHRONIC, metrics, &filter_hcv_state_chronic));
	stats.emplace(ACUTE, AggregateStats(ACUTE, metrics, &filter_hcv_state_acute));
	stats.emplace(RECOVERED, AggregateStats(RECOVERED, metrics, &filter_hcv_state_recovered));

	stats.emplace(VK_NONE, AggregateStats(VK_NONE, metrics, &filter_vkprofile_none));
	stats.emplace(VK_ACUTE_INFECT_CLEAR, AggregateStats(VK_ACUTE_INFECT_CLEAR, metrics, &filter_vkprofile_acute_infect_clear));
	stats.emplace(VK_ACUTE_INFECT_INCOM, AggregateStats(VK_ACUTE_INFECT_INCOM, metrics, &filter_vkprofile_acute_infect_incomplete));
	stats.emplace(VK_ACUTE_INFECT_PERSI, AggregateStats(VK_ACUTE_INFECT_PERSI, metrics, &filter_vkprofile_acute_infect_persist));
	stats.emplace(VK_REINFECT_HIGH_CLEAR, AggregateStats(VK_REINFECT_HIGH_CLEAR, metrics, &filter_vkprofile_reinfect_high_clearance));
	stats.emplace(VK_REINFECT_LOW_CLEAR, AggregateStats(VK_REINFECT_LOW_CLEAR, metrics, &filter_vkprofile_reinfect_low_clearance));
	stats.emplace(VK_REINFECT_CHRONIC, AggregateStats(VK_REINFECT_CHRONIC, metrics, &filter_vkprofile_reinfect_chronic));
	stats.emplace(VK_TREATMENT, AggregateStats(VK_TREATMENT, metrics, &filter_vkprofile_treatment));
    
	means.reset();
	event_counts.reset();

	// write the header
	out << "tick,run";
	for (auto& stat : stats) {
		stat.second.writeHeader(out);
	}
	means.writeHeader(out);
	event_counts.writeHeader(out);

	for (auto& metric : metrics) {
		out << "," << PREVALENCE + metric << "," << RNA_PREVALENCE + metric << "," << FRACTION + metric;
	}
	out << "\n";

	events_out << "run,tick,event_type,person_id,other\n";
	personsOut << "Tick,Id,Age,Gender,Race,Zip Code,HCV State,Network In Degree,Network Out Degree\n";
    needle_sharing_out << "Tick, Run";
    
    // Initialize the needle sharing map for all zip codes
    for (auto& entry : zoneMap){
        needle_sharing_map[entry.first] = 0;
        needle_sharing_out << "," << entry.first;
    }
    needle_sharing_out << "\n";    
    
}

void Statistics::close() {
	out.flush();
	out.close();
	events_out.flush();
	events_out.close();
	personsOut.flush();
	personsOut.close();
    needle_sharing_out.flush();
	needle_sharing_out.close();
}

Statistics::~Statistics() {
	close();
}

Statistics* Statistics::instance() {
	if (instance_ == nullptr) {
		throw std::domain_error("Statistics must be initialized before use");
	}
	return instance_;
}

void Statistics::calculatePrevalence(std::map<std::string, double>& prevalences) {
	AggregateStats& pop = stats.at(POPULATION);
	AggregateStats& infected = stats.at(INFECTED);
	AggregateStats& hcv = stats.at(HCV_ABPOS);

	double total_pop = pop.get("_ALL");

	for (auto& metric : metrics) {
		double pop_count = pop.get(metric);
		double hcvabpos_count = hcv.get(metric);
		double infected_count = infected.get(metric);

		if (pop_count == 0) {
			prevalences[PREVALENCE + metric] = 0;
			prevalences[RNA_PREVALENCE + metric] = 0;
			prevalences[FRACTION + metric] = 0;
		} else {
			prevalences[PREVALENCE + metric] = hcvabpos_count / pop_count;
			prevalences[RNA_PREVALENCE + metric] = infected_count / pop_count;
			prevalences[FRACTION + metric] = pop_count / total_pop;
		}
	}
}

void Statistics::writeEvents() {
	for (auto& evt : log_events) {
		events_out << run_number_ << "," << evt.tick << "," << evt.type.stringValue() << "," <<
				evt.person << "," << evt.other << "\n";
	}
	log_events.clear();
}

void Statistics::recordStats(double tick, int run,
		std::map<unsigned int, std::shared_ptr<HCPerson>>& persons) {
	StatKeySuffix sks;
	for (auto& kv : persons) {
		sks.set(kv.second);

		for (auto& stat : stats) {
			stat.second.increment(kv.second, sks, tick);
		}
		means.increment(kv.second);
	}


	out << tick << "," << run;

	for (auto& stat : stats) {
		stat.second.write(out);
	}

	means.calcMean();
	means.write(out);
	event_counts.write(out);

	std::map<std::string, double> prevalences;
	calculatePrevalence(prevalences);
	for (auto& metric : metrics) {
		out << "," << prevalences[PREVALENCE + metric] << "," << prevalences[RNA_PREVALENCE + metric]
			<< "," << prevalences[FRACTION + metric];
	}

	out << "\n";

	writeEvents();

	for (auto& stat : stats) {
		stat.second.reset();
	}
	means.reset();
	event_counts.reset();
    
	// NOTE: Disabled needle sharing logging to reduce output
    // // Set all the needle sharing zip counts to zero
    // needle_sharing_out << tick << "," << run;
    // for (auto& entry : needle_sharing_map){ 
    //     needle_sharing_out << "," << entry.second;
    //     entry.second = 0;
    // }
    // needle_sharing_out << "\n";
    //needle_sharing_map.clear();
}

void Statistics::recordNeedleSharing(unsigned int zipCode){
    
    // Increment the number of needle sharing episodes in this zipcode
    needle_sharing_map[zipCode]++;
}

void Statistics::logStatusChange(LogType logType, PersonPtr person, const std::string& msg){
	logStatusChange(logType, person.get(), msg);
}

void Statistics::logStatusChange(LogType logType, HCPerson* person, const std::string& msg) {

	double tick = repast::RepastProcess::instance()->getScheduleRunner().currentTick();

	if (filter_->evaluate(logType)) {
		log_events.push_back({tick, logType, person->id(), msg});
	}

	if (logType == LogType::ACTIVATED) {
		++event_counts.activations_daily;
	} else if (logType == LogType::CURED) {
		++event_counts.cured_daily;
		++event_counts.aggregate_posttreat;
	} else if (logType == LogType::DEACTIVATED) {
		++event_counts.losses_daily;
	} else if (logType == LogType::FAILED_TREATMENT) {
		++event_counts.aggregate_posttreat;
	} else if (logType == LogType::INFECTED) {
		++event_counts.incidence_daily;
    } else if (logType == LogType::CHRONIC) {
		++event_counts.incidence_daily_chronic;
	} else if (logType == LogType::STARTED_TREATMENT) {
		++event_counts.treatment_recruited_daily;
		++event_counts.aggregate_courses;
	} else if (logType == LogType::STARTED_OPIOID_TREATMENT) {
		++event_counts.opioid_treatment_recruited_daily;
	}
}

void Statistics::logPerson(PersonPtr person){
	double tick = repast::RepastProcess::instance()->getScheduleRunner().currentTick();

	personsOut << tick << "," << person->id() << "," << person->getAge() <<
			"," << person->getGender() << "," << person->getRace() << "," <<
			person->getZipcode() << "," << person->getHCVState() << "," <<
			person->getDrugReceptDegree() << "," << person->getDrugGivingDegree();

	personsOut << "\n";
}

void Statistics::setBurninMode(bool mode){
	burninMode = mode;
}

int Statistics::getDailyLosses(){
	return event_counts.losses_daily;
}

} /* namespace seir */
