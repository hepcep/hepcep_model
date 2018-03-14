/*
 * Statistics.cpp
 */
#include <iostream>
#include <memory>

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

void Statistics::init(const std::string& fname) {
    if (Statistics::instance_ != nullptr) {
        delete Statistics::instance_;
    }

    instance_ = new Statistics(fname);
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

Statistics::Statistics(const std::string& fname) :
        stats(), metrics(), means(), out(fname) {

    init_metrics(metrics);

    stats.emplace(POPULATION, AggregateStats(POPULATION, metrics));
    stats.emplace(INFECTED, AggregateStats(INFECTED, metrics, &filter_hcv_rna));
    stats.emplace(HCV_ABPOS, AggregateStats(HCV_ABPOS, metrics, &filter_hcv_abpos));
    stats.emplace(IN_TREATMENT, AggregateStats(IN_TREATMENT, metrics, &filter_in_treatment));
    stats.emplace(CURED, AggregateStats(CURED, metrics, &filter_cured));
    means.reset();

    // write the header
    out << "tick";
    for (auto& stat : stats) {
        stat.second.writeHeader(out);
    }
    means.writeHeader(out);

    for (auto& metric : metrics) {
       out << "," << PREVALENCE + metric << "," << RNA_PREVALENCE + metric << "," << FRACTION + metric;
    }
    out << "\n";
}

void Statistics::close() {
    out.flush();
    out.close();
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

void Statistics::collectStats(double tick,
        std::map<unsigned int, std::shared_ptr<HCPerson>>& persons) {
    StatKeySuffix sks;
    for (auto& kv : persons) {
        sks.set(kv.second);

        for (auto& stat : stats) {
            stat.second.increment(kv.second, sks, tick);
        }
        means.increment(kv.second);
    }

    out << tick;

    for (auto& stat : stats) {
       stat.second.write(out);
    }

    means.calcMean();
    means.write(out);

    std::map<std::string, double> prevalences;
    calculatePrevalence(prevalences);
    for (auto& metric : metrics) {
       out << "," << prevalences[PREVALENCE + metric] << "," << prevalences[RNA_PREVALENCE + metric]
               << "," << prevalences[FRACTION + metric];
    }

    out << "\n";

    for (auto& stat : stats) {
        stat.second.reset();
    }
    means.reset();
}

void Statistics::logStatusChange(LogType logType, HCPerson* person, const std::string& msg) {

}

} /* namespace seir */

