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
const std::string CURED = "cured";

const std::string PREVALENCE = "prevalence";
const std::string RNA_PREVALENCE = "RNApreval";
const std::string FRACTION = "fraction";

void EventCounts::reset() {
    activations_daily = cured_daily = losses_daily = aggregate_posttreat = incidence_daily =
            treatment_recruited_daily = aggregate_courses = 0;
}

void EventCounts::writeHeader(FileOut& out) {
    out << "," << "activations_daily,cured_daily,aggregate_posttreat,losses_daily,incidence_daily,treatment_recruited_daily,aggregate_courses";
}

void EventCounts::write(FileOut& out) {
    out << "," << activations_daily << "," << cured_daily << "," << aggregate_posttreat << "," <<
            losses_daily << "," << incidence_daily << "," << treatment_recruited_daily <<
            "," << aggregate_courses;
}

void MeanStats::calcMean() {
    age /= count;
    career /= count;
    daily_inj /= count;
    in_deg /= count;
    out_deg /= count;
    sharing /= count;
}

void MeanStats::reset() {
    age = career = daily_inj = in_deg = out_deg = sharing = 0;
    count = 0;
}

void MeanStats::increment(std::shared_ptr<HCPerson> person) {
    age += person->getAge();
    career += person->getAge() - person->getAgeStarted();
    daily_inj += person->getInjectionIntensity();
    in_deg += person->getDrugReceptDegree();
    out_deg += person->getDrugGivingDegree();
    sharing += person->getFractionReceptSharing();
    ++count;
}

void MeanStats::write(FileOut& out) {
    out << "," << age << "," << career << "," << daily_inj << "," << in_deg << "," << out_deg << ","
            << sharing;
}

void MeanStats::writeHeader(FileOut& out) {
    out << ",mean_age_ALL,mean_career_ALL,mean_daily_inj_ALL,mean_in_deg_ALL,mean_out_deg_ALL,mean_sharing_ALL";
}

Statistics* Statistics::instance_ = nullptr;

void Statistics::init(const std::string& fname, const std::string& events_fname, bool eventsEnabled) {
    if (Statistics::instance_ != nullptr) {
        delete Statistics::instance_;
    }

    instance_ = new Statistics(fname, events_fname, eventsEnabled);
}

void init_metrics(std::vector<std::string>& metrics) {
    for (auto& gender : Gender::values()) {
        metrics.push_back(GENDER_INFIX + gender.stringValue());
    }

    for (auto& state : HCVState::values()) {
        metrics.push_back(HCV_INFIX + state.stringValue());
    }

    for (auto& race : Race::values()) {
        metrics.push_back(RACE_INFIX + race.stringValue());
    }

    for (auto& dec : AgeDecade::values()) {
        metrics.push_back(AGEDEC_INFIX + dec.stringValue());
    }

    for (auto& grp : AgeGroup::values()) {
        metrics.push_back(AGEGRP_INFIX + grp.stringValue());
    }

    for (auto& at : AreaType::values()) {
        metrics.push_back(AREATYPE_INFIX + at.stringValue());
    }

    for (auto& hr : HarmReduction::values()) {
        metrics.push_back(SYRSRC_INFIX + hr.stringValue());
    }
    metrics.push_back("_ALL");
}

Statistics::Statistics(const std::string& fname, const std::string& events_fname, bool eventsEnabled) :
        stats(), metrics(), log_events(), means(), event_counts(), out(fname), events_out(events_fname), burninMode(false),
				logEventsEnabled(eventsEnabled) {

    init_metrics(metrics);

    stats.emplace(POPULATION, AggregateStats(POPULATION, metrics));
    stats.emplace(INFECTED, AggregateStats(INFECTED, metrics, &filter_hcv_rna));
    stats.emplace(HCV_ABPOS, AggregateStats(HCV_ABPOS, metrics, &filter_hcv_abpos));
    stats.emplace(IN_TREATMENT, AggregateStats(IN_TREATMENT, metrics, &filter_in_treatment));
    stats.emplace(CURED, AggregateStats(CURED, metrics, &filter_cured));
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

    events_out << "tick,event_type,person_id,other\n";
}

void Statistics::close() {
    out.flush();
    out.close();
    events_out.flush();
    events_out.close();
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
        events_out << evt.tick << "," << evt.type.stringValue() << "," <<
                evt.person << "," << evt.other << "\n";
    }
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

    if (logEventsEnabled){
    	writeEvents();
    }

    for (auto& stat : stats) {
        stat.second.reset();
    }
    means.reset();
    event_counts.reset();
}

void Statistics::logStatusChange(LogType logType, PersonPtr person, const std::string& msg){
	logStatusChange(logType, person.get(), msg);
}

void Statistics::logStatusChange(LogType logType, HCPerson* person, const std::string& msg) {

	// TODO change this to toggle writing, but we still need to count stats for the model logic
//	if (!logEventsEnabled)
//		return;

	double tick = repast::RepastProcess::instance()->getScheduleRunner().currentTick();
	log_events.push_back({tick, logType, person->id(), msg});
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
	} else if (logType == LogType::STARTED_TREATMENT) {
		++event_counts.treatment_recruited_daily;
		++event_counts.aggregate_courses;
	}
}

void Statistics::setBurninMode(bool mode){
	burninMode = mode;
}

void Statistics::setLogEventsEnabled(bool enabled){
	logEventsEnabled = enabled;
}

int Statistics::getDailyLosses(){
	return event_counts.losses_daily;
}

} /* namespace seir */
