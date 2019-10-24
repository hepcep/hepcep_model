/*
 * NetworkTests.cpp
 *
 *  Created on: Nov 2, 2015
 *      Author: nick
 */
#include <vector>

#include "gtest/gtest.h"
#include "boost/filesystem.hpp"

#include "chi_sim/Parameters.h"
#include "repast_hpc/Properties.h"
#include "repast_hpc/RepastProcess.h"

#include "Network.h"
#include "Edge.h"
#include "network_utils.h"
#include "HCPerson.h"
#include "ZoneLoader.h"
#include "serialize.h"
#include "Filter.h"
#include "Statistics.h"

using namespace hepcep;

class Agent {

private:
	unsigned int id_, age_;

public:

	Agent(unsigned int id, unsigned int age) :
			id_(id), age_(age) {
	}

	unsigned int id() const {
		return id_;
	}

	int age() const {
		return age_;
	}

	void setAge(int age) {
		age_ = age;
	}
};


typedef std::shared_ptr<Agent> AgentPtrT;


TEST(NetworkTests, TestAdds) {
	Network<Agent> net(false);

	AgentPtrT one = std::make_shared<Agent>(1, 1);
	AgentPtrT two = std::make_shared<Agent>(2, 1);
	AgentPtrT three = std::make_shared<Agent>(3, 1);
	AgentPtrT four = std::make_shared<Agent>(4, 1);

	net.addVertex(one);
	net.addVertex(two);
	ASSERT_EQ(2, net.vertexCount());
	ASSERT_EQ(0, net.edgeCount());

	EdgePtrT<Agent> e = net.addEdge(three, four);
	e->setWeight(14);
	ASSERT_EQ(4, net.vertexCount());
	int id = 1;
	for (auto iter = net.verticesBegin(); iter != net.verticesEnd(); ++iter) {
		AgentPtrT agent = (*iter);
		ASSERT_EQ(id, agent->id());
		++id;
	}

	ASSERT_EQ(1, net.edgeCount());
	int count = 0;
	for (auto iter = net.edgesBegin(); iter != net.edgesEnd(); ++iter) {
		++count;
		EdgePtrT<Agent> edge = (*iter);
		ASSERT_EQ(3, edge->v1()->id());
		ASSERT_EQ(4, edge->v2()->id());
		ASSERT_EQ(14, edge->weight());
	}
	ASSERT_EQ(1, count);

	net.addEdge(two, one);
	count = 0;
	for (auto iter = net.edgesBegin(); iter != net.edgesEnd(); ++iter) {
		if (count == 0) {
			EdgePtrT<Agent> edge = (*iter);
			ASSERT_EQ(3, edge->v1()->id());
			ASSERT_EQ(4, edge->v2()->id());
			ASSERT_EQ(14, edge->weight());
		} else {
			EdgePtrT<Agent> edge = (*iter);
			ASSERT_EQ(2, edge->v1()->id());
			ASSERT_EQ(1, edge->v2()->id());
			ASSERT_EQ(1, edge->weight());
		}
		++count;
	}
	ASSERT_EQ(2, count);

	net.addEdge(two, four);
	ASSERT_EQ(3, net.edgeCount());
	ASSERT_EQ(2, net.outEdgeCount(two));
	auto iter = net.edgesBegin();
	advance(iter, 2);
	EdgePtrT<Agent> ep = (*iter);
	ASSERT_EQ(2, ep->id());
	ASSERT_EQ(2, ep->v1()->id());
	ASSERT_EQ(4, ep->v2()->id());
	ASSERT_EQ(1, ep->weight());

	net.addEdge(two, three);
	ASSERT_TRUE(net.hasEdge(two, three));
	ASSERT_TRUE(net.hasEdge(three, two));
}


// tests behavior specific to undirected net.
TEST(NetworkTests, TestUndirected) {
    Network<Agent> net(false);

    AgentPtrT one = std::make_shared<Agent>(1, 1);
    AgentPtrT two = std::make_shared<Agent>(2, 1);
    AgentPtrT three = std::make_shared<Agent>(3, 1);
    AgentPtrT four = std::make_shared<Agent>(4, 1);

    EdgePtrT<Agent> e = net.addEdge(three, four);
    ASSERT_EQ(1, net.edgeCount());

    ASSERT_TRUE(net.hasEdge(three, four));
    ASSERT_TRUE(net.hasEdge(four, three));
    e = net.removeEdge(four, three);
    ASSERT_EQ(three, e->v1());
    ASSERT_EQ(0, net.edgeCount());

    net.addEdge(one, two);
    net.addEdge(three, one);
    ASSERT_EQ(2, net.inEdgeCount(one));
    std::vector<EdgePtrT<Agent>> vec;
    net.inEdges(one, vec);
    ASSERT_EQ(2, vec.size());

    ASSERT_EQ(3, vec[0]->v1()->id());
    ASSERT_EQ(1, vec[0]->v2()->id());
    ASSERT_EQ(1, vec[1]->v1()->id());
    ASSERT_EQ(2, vec[1]->v2()->id());

    vec.clear();
    net.outEdges(one, vec);
    ASSERT_EQ(2, vec.size());

    ASSERT_EQ(1, vec[0]->v1()->id());
    ASSERT_EQ(2, vec[0]->v2()->id());
    ASSERT_EQ(3, vec[1]->v1()->id());
    ASSERT_EQ(1, vec[1]->v2()->id());

    // add an edge that already exists under undirected semantics
    unsigned int count = net.edgeCount();
    net.addEdge(two, one);
    ASSERT_EQ(count, net.edgeCount());
}

// tests behavior specific to directed net.
TEST(NetworkTests, TestDirected) {
    Network<Agent> net(true);

    AgentPtrT one = std::make_shared<Agent>(1, 1);
    AgentPtrT two = std::make_shared<Agent>(2, 1);
    AgentPtrT three = std::make_shared<Agent>(3, 1);
    AgentPtrT four = std::make_shared<Agent>(4, 1);

    EdgePtrT<Agent> e = net.addEdge(three, four);
    ASSERT_EQ(1, net.edgeCount());

    ASSERT_TRUE(net.hasEdge(three, four));
    ASSERT_FALSE(net.hasEdge(four, three));
    e = net.removeEdge(four, three);
    ASSERT_EQ(nullptr, e.get());
    ASSERT_EQ(1, net.edgeCount());

    e = net.removeEdge(three, four);
    ASSERT_EQ(three, e->v1());
    ASSERT_EQ(0, net.edgeCount());

    net.addEdge(one, two);
    net.addEdge(three, one);
    ASSERT_EQ(1, net.inEdgeCount(one));
    std::vector<EdgePtrT<Agent>> vec;
    net.inEdges(one, vec);
    ASSERT_EQ(1, vec.size());

    ASSERT_EQ(3, vec[0]->v1()->id());
    ASSERT_EQ(1, vec[0]->v2()->id());

    vec.clear();
    net.outEdges(one, vec);
    ASSERT_EQ(1, vec.size());

    ASSERT_EQ(1, vec[0]->v1()->id());
    ASSERT_EQ(2, vec[0]->v2()->id());

    // add an edge that already exists under undirected semantics,
    // but not under directed
   unsigned int count = net.edgeCount();
   net.addEdge(two, one);
   ASSERT_EQ(count + 1, net.edgeCount());
}


TEST(NetworkTests, TestEdgeGet) {
	Network<Agent> net(false);

	AgentPtrT one = std::make_shared<Agent>(1, 1);
	AgentPtrT two = std::make_shared<Agent>(2, 1);
	AgentPtrT three = std::make_shared<Agent>(3, 1);
	AgentPtrT four = std::make_shared<Agent>(4, 1);

	net.addVertex(one);
	net.addEdge(three, one);
	net.addEdge(one, two);
	net.addEdge(one, four);
	net.addEdge(three, four);

	std::vector<EdgePtrT<Agent>> vec;
	net.edges(one, vec);

	ASSERT_EQ(3, vec.size());
	EdgePtrT<Agent> edge = vec[0];
	ASSERT_EQ(3, edge->v1()->id());
	ASSERT_EQ(1, edge->v2()->id());

	edge = vec[1];
	ASSERT_EQ(1, edge->v1()->id());
	ASSERT_EQ(2, edge->v2()->id());

	edge = vec[2];
	ASSERT_EQ(1, edge->v1()->id());
	ASSERT_EQ(4, edge->v2()->id());
}

TEST(NetworkTests, TestRemoves) {
	Network<Agent> net(false);

	AgentPtrT one = std::make_shared<Agent>(1, 1);
	AgentPtrT two = std::make_shared<Agent>(2, 1);
	AgentPtrT three = std::make_shared<Agent>(3, 1);
	AgentPtrT four = std::make_shared<Agent>(4, 1);

	net.addVertex(one);
	net.addEdge(three, one);
	net.addEdge(one, two);
	net.addEdge(one, four);
	net.addEdge(three, four);

	ASSERT_EQ(4, net.vertexCount());
	ASSERT_EQ(4, net.edgeCount());

	ASSERT_EQ(3, net.outEdgeCount(one));
	ASSERT_EQ(1, net.outEdgeCount(two));
	ASSERT_EQ(2, net.outEdgeCount(three));
	ASSERT_EQ(2, net.outEdgeCount(four));

	ASSERT_EQ(3, net.outEdgeCount(one));
    ASSERT_EQ(1, net.outEdgeCount(two));
    ASSERT_EQ(2, net.outEdgeCount(three));
    ASSERT_EQ(2, net.outEdgeCount(four));

	net.removeVertex(one);
	ASSERT_EQ(3, net.vertexCount());
	ASSERT_EQ(1, net.edgeCount());

	ASSERT_EQ(0, net.outEdgeCount(one));
	ASSERT_EQ(0, net.outEdgeCount(two));
	ASSERT_EQ(1, net.outEdgeCount(three));
	ASSERT_EQ(1, net.outEdgeCount(four));

	ASSERT_EQ(0, net.inEdgeCount(one));
	ASSERT_EQ(0, net.inEdgeCount(two));
	ASSERT_EQ(1, net.inEdgeCount(three));
	ASSERT_EQ(1, net.inEdgeCount(four));

	int count = 0;
	for (auto iter = net.edgesBegin(); iter != net.edgesEnd(); ++iter) {
		EdgePtrT<Agent> edge = (*iter);
		ASSERT_EQ(3, edge->v1()->id());
		ASSERT_EQ(4, edge->v2()->id());
		ASSERT_EQ(1, edge->weight());
		++count;
	}

	ASSERT_EQ(1, count);
	int id = 2;
	count = 0;
	for (auto iter = net.verticesBegin(); iter != net.verticesEnd(); ++iter) {
		AgentPtrT ap = (*iter);
		ASSERT_EQ(id, ap->id());
		++id;
		++count;
	}
	ASSERT_EQ(3, count);

	unsigned int edge_count = net.edgeCount();
	net.addEdge(two, one);
	ASSERT_EQ(edge_count + 1, net.edgeCount());
	ASSERT_TRUE(net.removeEdge(two->id(), one->id()));
	ASSERT_EQ(0, net.outEdgeCount(two));
	ASSERT_EQ(0, net.inEdgeCount(one));
	ASSERT_EQ(edge_count, net.edgeCount());
}

TEST(NetworkTests, TestRemovesByIter) {
	Network<Agent> net(true);

	AgentPtrT one = std::make_shared<Agent>(1, 1);
	AgentPtrT two = std::make_shared<Agent>(2, 1);
	AgentPtrT three = std::make_shared<Agent>(3, 1);
	AgentPtrT four = std::make_shared<Agent>(4, 1);

	net.addVertex(one);
	net.addEdge(three, one);
	net.addEdge(one, two);
	net.addEdge(one, four);
	net.addEdge(three, four);

	ASSERT_EQ(2, net.outEdgeCount(one));
	ASSERT_EQ(0, net.outEdgeCount(two));
	ASSERT_EQ(2, net.outEdgeCount(three));
	ASSERT_EQ(0, net.outEdgeCount(four));

	ASSERT_EQ(1, net.inEdgeCount(one));
	ASSERT_EQ(1, net.inEdgeCount(two));
	ASSERT_EQ(0, net.inEdgeCount(three));
	ASSERT_EQ(2, net.inEdgeCount(four));

	ASSERT_EQ(4, net.vertexCount());
	ASSERT_EQ(4, net.edgeCount());

	int i = 1;
	for (auto iter = net.verticesBegin(); iter != net.verticesEnd();) {
		AgentPtrT p = (*iter);
		ASSERT_EQ(i, p->id());
		if (i == 1) {
			iter = net.removeVertex(iter);
		} else {
			++iter;
		}
		++i;
	}

	ASSERT_EQ(3, net.vertexCount());
	ASSERT_EQ(1, net.edgeCount());

	ASSERT_EQ(0, net.outEdgeCount(one));
	ASSERT_EQ(0, net.outEdgeCount(two));
	ASSERT_EQ(1, net.outEdgeCount(three));
	ASSERT_EQ(0, net.outEdgeCount(four));

	ASSERT_EQ(0, net.inEdgeCount(one));
	ASSERT_EQ(0, net.inEdgeCount(two));
	ASSERT_EQ(0, net.inEdgeCount(three));
	ASSERT_EQ(1, net.inEdgeCount(four));

	int count = 0;
	for (auto iter = net.edgesBegin(); iter != net.edgesEnd(); ++iter) {
		EdgePtrT<Agent> edge = (*iter);
		ASSERT_EQ(3, edge->v1()->id());
		ASSERT_EQ(4, edge->v2()->id());
		ASSERT_EQ(1, edge->weight());
		++count;
	}

	ASSERT_EQ(1, count);
	int id = 2;
	count = 0;
	for (auto iter = net.verticesBegin(); iter != net.verticesEnd(); ++iter) {
		AgentPtrT ap = (*iter);
		ASSERT_EQ(id, ap->id());
		++id;
		++count;
	}
	ASSERT_EQ(3, count);
}

TEST(NetworkTests, TestRemovesByVertexIds) {
	Network<Agent> net(true);

	AgentPtrT one = std::make_shared<Agent>(1, 1);
	AgentPtrT two = std::make_shared<Agent>(2, 1);
	AgentPtrT three = std::make_shared<Agent>(3, 1);
	AgentPtrT four = std::make_shared<Agent>(4, 1);

	net.addVertex(one);
	net.addEdge(three, one);
	net.addEdge(one, two);
	net.addEdge(one, four);
	net.addEdge(three, four);

	ASSERT_EQ(4, net.edgeCount());

	ASSERT_EQ(2, net.outEdgeCount(one));
	ASSERT_EQ(0, net.outEdgeCount(two));
	ASSERT_EQ(2, net.outEdgeCount(three));
	ASSERT_EQ(0, net.outEdgeCount(four));

	ASSERT_EQ(1, net.inEdgeCount(one));
    ASSERT_EQ(1, net.inEdgeCount(two));
    ASSERT_EQ(0, net.inEdgeCount(three));
    ASSERT_EQ(2, net.inEdgeCount(four));

	net.removeEdge(1, 2);

	ASSERT_EQ(3, net.edgeCount());

	auto eiter = net.edgesBegin();
	EdgePtrT<Agent> edge = (*eiter);
	ASSERT_EQ(3, edge->v1()->id());
	ASSERT_EQ(1, edge->v2()->id());

	++eiter;
	edge = (*eiter);
	ASSERT_EQ(1, edge->v1()->id());
	ASSERT_EQ(4, edge->v2()->id());

	++eiter;
	edge = (*eiter);
	ASSERT_EQ(3, edge->v1()->id());
	ASSERT_EQ(4, edge->v2()->id());

	ASSERT_EQ(1, net.outEdgeCount(one));
	ASSERT_EQ(0, net.outEdgeCount(two));
	ASSERT_EQ(2, net.outEdgeCount(three));
	ASSERT_EQ(0, net.outEdgeCount(four));

	ASSERT_EQ(1, net.inEdgeCount(one));
	ASSERT_EQ(0, net.inEdgeCount(two));
	ASSERT_EQ(0, net.inEdgeCount(three));
	ASSERT_EQ(2, net.inEdgeCount(four));
}

TEST(NetworkTests, testSimpleWriting) {
    std::string fname("../test_output/net.gml");
    boost::filesystem::path filepath(fname);
    if (boost::filesystem::exists(filepath)) {
        boost::filesystem::remove(filepath);
    }

    NetworkPtr<Agent> net = std::make_shared<Network<Agent>>(true);

    AgentPtrT one = std::make_shared<Agent>(1, 1);
    AgentPtrT two = std::make_shared<Agent>(2, 1);
    AgentPtrT three = std::make_shared<Agent>(3, 1);
    AgentPtrT four = std::make_shared<Agent>(4, 1);

    net->addVertex(one);
    net->addEdge(three, one);
    net->addEdge(one, two);
    net->addEdge(one, four);
    net->addEdge(three, four);

    write_network(fname, net);
}


void write_agent(Agent* agent, AttributeWriter& write) {
    write("age", agent->age());
}

void write_edge(Edge<Agent>* edge, AttributeWriter& write) {
    write("distance", edge->getAttribute("distance", 0));
}

TEST(NetworkTests, testAttribWriting) {
    std::string fname("../test_output/net_2.gml");
    boost::filesystem::path filepath(fname);
    if (boost::filesystem::exists(filepath)) {
        boost::filesystem::remove(filepath);
    }

    NetworkPtr<Agent> net = std::make_shared<Network<Agent>>(true);

    AgentPtrT one = std::make_shared<Agent>(1, 3);
    AgentPtrT two = std::make_shared<Agent>(2, 12);
    AgentPtrT three = std::make_shared<Agent>(3, 2);
    AgentPtrT four = std::make_shared<Agent>(4, 15);

    net->addVertex(one);
    net->addEdge(three, one)->putAttribute("distance", 12.343);
    net->addEdge(one, two)->putAttribute("distance", 2.343);
    net->addEdge(one, four)->putAttribute("distance", 1832.0);
    net->addEdge(three, four)->putAttribute("distance", 324);

    write_network(fname, 0, net, &write_agent, &write_edge);
}

std::shared_ptr<Edge<HCPerson>> find_edge(NetworkPtr<HCPerson>& net, unsigned int source, unsigned int target) {
	for (auto iter = net->edgesBegin(); iter != net->edgesEnd(); ++iter) {
		if ((*iter)->v1()->id() == source && (*iter)->v2()->id() == target) {
			return (*iter);
		}
	}

	return nullptr;
}

TEST(NetworkTests, testReadNetwork) {
	repast::Properties props("../config/test.props");
	chi_sim::Parameters::initialize(props);
	std::shared_ptr<Filter<LogType>> filter = std::make_shared<NeverPassFilter<LogType>>();
	Statistics::init("../test_output/stats.csv", "../test_output/events.csv", 
		"../test_output/persons.csv", filter, 1);


	std::string fname("../test_data/net_2.gml");
	std::map<std::string,ZonePtr> zone_map;
	loadZones("../data/zones.csv", zone_map);
	double serialized_at;
	NetworkPtr<HCPerson> net = read_network<HCPerson>(fname, &read_person, &read_edge, zone_map, &serialized_at);

	ASSERT_EQ(300.0, serialized_at);
	ASSERT_EQ(4, net->vertexCount());
	std::map<unsigned int, PersonPtr> map;
	for (auto iter = net->verticesBegin(); iter != net->verticesEnd(); ++iter) {
		map.emplace((*iter)->id(), *iter);
	}
	ASSERT_EQ(4, map.size());
	for (int i = 1; i < 5; ++i) {
		ASSERT_TRUE(map.find(i) != map.end());
	}

	ASSERT_EQ(5, net->edgeCount());
	std::shared_ptr<Edge<HCPerson>> edge = find_edge(net, 3, 1);
	ASSERT_TRUE(edge);
	ASSERT_EQ(12.343, edge->getAttribute("distance"));
	ASSERT_EQ(480.1, edge->getAttribute("ends_at"));

	edge = find_edge(net, 1, 2);
	ASSERT_TRUE(edge);
	ASSERT_EQ(2.34300000, edge->getAttribute("distance"));
	ASSERT_EQ(302.1, edge->getAttribute("ends_at"));

	edge = find_edge(net, 1, 4);
	ASSERT_TRUE(edge);
	ASSERT_EQ(1832.00000000, edge->getAttribute("distance"));
	ASSERT_EQ(2000.1, edge->getAttribute("ends_at"));

	edge = find_edge(net, 3, 4);
	ASSERT_TRUE(edge);
	ASSERT_EQ(324.0, edge->getAttribute("distance"));
	ASSERT_EQ(324.1, edge->getAttribute("ends_at"));

	edge = find_edge(net, 4, 3);
	ASSERT_TRUE(edge);
	ASSERT_EQ(324.0, edge->getAttribute("distance"));
	ASSERT_EQ(1002.1, edge->getAttribute("ends_at"));

	PersonPtr person = map[3];
	ASSERT_EQ(51.24657535, person->getAge());
	ASSERT_EQ(30.39743820, person->getAgeStarted());
	ASSERT_EQ(Race::HISPANIC, person->getRace());
	ASSERT_EQ(Gender::MALE, person->getGender());
	ASSERT_EQ(HarmReduction::NON_HARM_REDUCTION, person->getSyringeSource());
	ASSERT_EQ("60647", person->getZipcode());
	ASSERT_EQ(30, person->getDrugReceptDegree());
	ASSERT_EQ(0, person->getDrugGivingDegree());
	ASSERT_EQ(2.88931634, person->getInjectionIntensity());
	ASSERT_EQ(0.41437703, person->getFractionReceptSharing());
	ASSERT_EQ(278.0, person->getLastExposureDate());
	ASSERT_EQ(278.0, person->getLastInfectionDate());
	ASSERT_EQ(true, person->isActive());
	// 300 is when the graph was recorded to deactivate minus that much
	// elapsed
    ASSERT_EQ(2430.50722435, person->getDeactivateAt());

	// person methods that call immunology
	ASSERT_EQ(HCVState::RECOVERED, person->getHCVState());
	ASSERT_EQ(false, person->isInTreatment());

	// trigger some events
	repast::ScheduleRunner& runner = repast::RepastProcess::instance()->getScheduleRunner();
 	repast::Schedule schedule = runner.schedule();

	// should trigger edge 1->2 dissolution
	schedule.execute();
	ASSERT_EQ(4, net->edgeCount());
	ASSERT_FALSE(find_edge(net, 1, 2));

	// should trigger leave exposed on person 3, changing HCV state
	schedule.execute();
	ASSERT_EQ(HCVState::INFECTIOUS_ACUTE, person->getHCVState());

	// triggers leave acute
	schedule.execute();
	ASSERT_TRUE(person->getHCVState() == HCVState::CHRONIC || 
				person->getHCVState() == HCVState::RECOVERED);

	// triggers deactivate on person 2
	ASSERT_EQ(true, map[2]->isActive());
	schedule.execute();
	ASSERT_EQ(false, map[2]->isActive());

	// should trigger edge 3->4 dissolution
	schedule.execute();
	ASSERT_EQ(3, net->edgeCount());
	ASSERT_FALSE(find_edge(net, 3, 4));

	// triggers end treament on person 4
	ASSERT_EQ(true, map[4]->isInTreatment());
	schedule.execute();
	ASSERT_EQ(false, map[4]->isInTreatment());
}


