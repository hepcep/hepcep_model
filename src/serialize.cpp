/*
 * creators.cpp
 *
 *  Created on: Nov 27, 2017
 *      Author: nick
 */

#include <iomanip>

#include "repast_hpc/RepastProcess.h"

//#include "chi_sim/CSVReader.h"

#include "serialize.h"
#include "SVReader.h"
#include "EndTreatmentFunctor.h"
#include "EndRelationshipFunctor.h"

using namespace std;

namespace hepcep {

const int ID_IDX = 0;
const int RANK_IDX = 2;

std::string quote_string(const std::string& str) {
    return "\"" + str + "\"";
}

void write_person(HCPerson* person, AttributeWriter& write, double tick) {
	write("age", person->getAge());
	write("age_started", person->getAgeStarted());
	write("race", "\"" + person->getRace().stringValue() +"\"");
	write("gender", "\"" + person->getGender().stringValue() +"\"");
	write("syringe_source", "\"" + person->getSyringeSource().stringValue() +"\"");
	write("zipcode", person->getZipcode());

	write("drug_in_deg", person->getDrugReceptDegree());
	write("drug_out_deg", person->getDrugGivingDegree());
	write("inject_intens", person->getInjectionIntensity());
	write("frac_recept", person->getFractionReceptSharing());

	write("lat", person->getZone()->getLat() + 0.1 * (repast::Random::instance()->nextDouble() - 0.5));
	write("lon", person->getZone()->getLon() + 0.1 * (repast::Random::instance()->nextDouble() - 0.5));

	write("last_exposure_date", person->getLastExposureDate());
	write("last_infection_date", person->getLastInfectionDate());
	write("active", person->isActive());
    write("deactivate_at", person->getDeactivateAt());

	write_immunology(person->immunology, write, tick);
}

void read_immunology(NamedListAttribute* list, shared_ptr<Immunology> imm, HCPerson* person, double serialized_at) {
    imm->idu_ = person;
    imm->hcv_state = HCVState::valueOf(list->getAttribute<string>("hcv_state"));
    imm->in_treatment = (bool)list->getAttribute<int>("in_treatment");
    
    NamedListAttribute* params = list->getAttribute<NamedListAttribute*>("params");
    imm->params_->mean_days_acute_naive = params->getAttribute<double>("mean_days_acute_naive");
    imm->params_->mean_days_acute_rechallenged = params->getAttribute<double>("mean_days_acute_rechallenged");
    imm->params_->mean_days_naive_to_infectious = params->getAttribute<double>("mean_days_naive_to_infectious");
    imm->params_->mean_days_residual_hcv_infectivity = params->getAttribute<double>("mean_days_residual_hcv_infectivity");
    imm->params_->prob_clearing = params->getAttribute<double>("prob_clearing");
    imm->params_->prob_self_limiting_female = params->getAttribute<double>("prob_self_limiting_female");
    imm->params_->prob_self_limiting_male = params->getAttribute<double>("prob_self_limiting_male");
    imm->params_->transmissibility = params->getAttribute<double>("transmissibility");
    imm->params_->treatment_duration = params->getAttribute<double>("treatment_duration");
    imm->params_->treatment_repeatable = (bool)params->getAttribute<int>("treatment_repeatable");
    imm->params_->treatment_susceptibility = params->getAttribute<double>("treatment_susceptibility");
    imm->params_->treatment_svr = params->getAttribute<double>("treatment_svr");

    imm->past_cured = (bool)list->getAttribute<int>("past_cured");
    imm->past_recovered = (bool)list->getAttribute<int>("past_recovered");
    imm->in_treatment = (bool)list->getAttribute<int>("in_treatment");
    imm->treatment_start_date = list->getAttribute<double>("treatment_start_date");
    imm->treatment_failed = (bool)list->getAttribute<int>("treatment_failed");
    
    if (list->hasAttribute("events")) {
        NamedListAttribute* evts = list->getAttribute<NamedListAttribute*>("events");
        repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();

        for (auto kv : evts->attributes_) {
            if (kv.first.find("event") == 0) {
                NamedListAttribute* evt_list = dynamic_cast<NamedListAttribute*>(kv.second);
                if (!(bool)evt_list->getAttribute<int>("canceled")) {
                    double at = evt_list->getAttribute<double>("scheduled_for");
                    if (at >= 0 && at > serialized_at) {
                        // std::cout << person->id() << ": scheduling event for " << at << std::endl;
                        EventFuncType ft = static_cast<EventFuncType>(evt_list->getAttribute<int>("ef_type"));
                        boost::shared_ptr<Event> evt;
                        if (ft == EventFuncType::LEAVE_ACUTE) {
                            evt = boost::make_shared<Event>(at, EventFuncType::LEAVE_ACUTE,
                                new MethodFunctor<Immunology, bool>(imm.get(), &Immunology::leaveAcute));
                        } else if (ft == EventFuncType::LEAVE_EXPOSED) {
                            evt = boost::make_shared<Event>(at, EventFuncType::LEAVE_EXPOSED,
                                new MethodFunctor<Immunology, void>(imm.get(), &Immunology::leaveExposed));
                        } else {
                            // EventFuncType::END_TREATMENT
                            bool succeeds = (bool)evt_list->getAttribute<int>("success");
                            evt = boost::make_shared<Event>(at, EventFuncType::END_TREATMENT,
                                new EndTreatmentFunctor(succeeds, imm.get()));
                        }
                        //std::cout << "evt scheduled at " << at << std::endl;
                        imm->scheduled_actions.push_back(evt);
                        runner.scheduleEvent(at, evt);
                    } 
                }
            }
        }
    }
}

void write_event(int idx, boost::shared_ptr<Event> evt, std::stringstream& ss) {
    ss << "          event_" + std::to_string(idx) <<  " [\n";
    ss << "          canceled "  <<  evt->canceled << "\n";
    ss <<  std::setprecision(8) << std::fixed << "          scheduled_for " <<  evt->scheduled_for_ << "\n";
    ss << "          ef_type " << static_cast<int>(evt->ef_type_) << "\n";
    if (evt->ef_type_ == EventFuncType::END_TREATMENT) {
        ss << "          success " <<  dynamic_cast<EndTreatmentFunctor*>(evt->func_)->isSuccess() << "\n";
    }
    ss << "]";;
}

PersonPtr read_person(NamedListAttribute* node, std::map<unsigned int,ZonePtr>& zoneMap, double serialized_at) {
    unsigned int id = node->getAttribute<int>("id");
    HCPersonData data;
    data.age = node->getAttribute<double>("age");
    data.ageStarted = node->getAttribute<double>("age_started");
    data.gender = node->getAttribute<string>("gender");
    data.drug_inDegree = node->getAttribute<int>("drug_in_deg");
    data.drug_outDegree =  node->getAttribute<int>("drug_out_deg");
    data.injectionIntensity = node->getAttribute<double>("inject_intens");
    data.fractionReceptSharing = node->getAttribute<double>("frac_recept");
    data.syringeSource = node->getAttribute<string>("syringe_source");
    data.zipCode = node->getAttribute<int>("zipcode");
    data.race = node->getAttribute<string>("race");

    NamedListAttribute* imm_list = node->getAttribute<NamedListAttribute*>("immunology");
    data.hcvState = HCVState::valueOf(imm_list->getAttribute<std::string>("hcv_state"));

    // create an immunology with no associated person
    std::shared_ptr<Immunology> imm = std::make_shared<Immunology>(nullptr);
    PersonPtr person = std::make_shared<HCPerson>(id, data, imm);
    // udpates immunology with the specified person (among other things)
    read_immunology(imm_list, imm, person.get(), serialized_at);
    
    person->setZone(zoneMap.at(data.zipCode));
    person->active = (bool)node->getAttribute<int>("active");
    person->lastExposureDate = node->getAttribute<double>("last_exposure_date");
    person->lastInfectionDate = node->getAttribute<double>("last_infection_date");

    double deactivate_at = node->getAttribute<double>("deactivate_at");
    person->deactivateAt = deactivate_at;
    if (deactivate_at > serialized_at) {
        repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
        runner.scheduleEvent(deactivate_at, repast::Schedule::FunctorPtr(
            new repast::MethodFunctor<HCPerson>(person.get(), &HCPerson::deactivate)));
        //std::cout << "deactivate scheduled at " << deactivate_at << std::endl;
    }
    
    return person;
}

void write_immunology(std::shared_ptr<Immunology> imm, AttributeWriter& write, double tick) {
    write("immunology", "[");
    write("hcv_state", quote_string(imm->getHCVState().stringValue()));
    write("past_cured", imm->past_cured);
    write("past_recovered", imm->past_recovered);
    write("in_treatment", imm->in_treatment);
    write("treatment_start_date", imm->treatment_start_date);
    write("treatment_failed", imm->treatment_failed);

    if (imm->scheduled_actions.size() > 0) {
        std::stringstream ss;
        int i = 0;
        for (auto evt : imm->scheduled_actions) {
            if (evt->scheduled_for() > tick) {
                //std::cout << "writing future event scheduled for " << evt->scheduled_for() << std::endl;
                write_event(i, evt, ss);
                i++;
            }
        }

        if (!ss.str().empty()) {
            write("events", "[");
            write("", ss.str());
            write("", "]");
        }
    }
    
    write("params", "[");
    IPPtr ip = imm->params_;

    write("mean_days_acute_naive", ip->mean_days_acute_naive);
    write("mean_days_acute_rechallenged", ip->mean_days_acute_rechallenged);
    write("mean_days_naive_to_infectious", ip->mean_days_naive_to_infectious);
    write("mean_days_residual_hcv_infectivity", ip->mean_days_residual_hcv_infectivity);
    write("prob_self_limiting_female", ip->prob_self_limiting_female);
    write("prob_self_limiting_male", ip->prob_self_limiting_male);
    write("prob_clearing", ip->prob_clearing);
	write("transmissibility", ip->transmissibility);
    write("treatment_duration", ip->treatment_duration);
    write("treatment_svr", ip->treatment_svr);
    write("treatment_susceptibility", ip->treatment_susceptibility);
    write("treatment_repeatable", ip->treatment_repeatable);
    write("", "]");

    write("", "]");
}

void write_edge(Edge<HCPerson>* edge, AttributeWriter& write) {
	write("distance", edge->getAttribute("distance", 0));
	write("ends_at", edge->getAttribute("ends_at", 0));
}

void read_edge(NamedListAttribute* edge_list, NetworkPtr<HCPerson>& net, 
    std::map<unsigned int, std::shared_ptr<HCPerson>>& vertex_map) {

    unsigned int source = edge_list->getAttribute<int>("source");
    unsigned int target = edge_list->getAttribute<int>("target");
    double distance = edge_list->getAttribute<double>("distance");

    std::shared_ptr<Edge<HCPerson>> edge = net->addEdge(source, target);
    edge->putAttribute("distance", distance);
    double ends_at = edge_list->getAttribute<double>("ends_at");
    edge->putAttribute("ends_at", ends_at);

    repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
    EndRelationshipFunctor* endRelationshipEvent2 = 
        new EndRelationshipFunctor(vertex_map.at(source), vertex_map.at(target), net);
	runner.scheduleEvent(ends_at, repast::Schedule::FunctorPtr(endRelationshipEvent2));
}

}