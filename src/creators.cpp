/*
 * creators.cpp
 *
 *  Created on: Nov 27, 2017
 *      Author: nick
 */

#include "repast_hpc/RepastProcess.h"

#include "chi_sim/CSVReader.h"

#include "creators.h"

namespace hepcep {

const int ID_IDX = 0;
const int RANK_IDX = 2;

void create_persons(const std::string& filename, std::map<unsigned int, PersonPtr>& persons) {
    chi_sim::CSVReader reader(filename);
    std::vector<std::string> line;

    // skip header
    reader.next(line);

    // not strictly necessary in initial version, as world size is always 1
    // but kept here to illustrate how to only create "local persons"
    int my_rank = repast::RepastProcess::instance()->rank();
    int world_size = repast::RepastProcess::instance()->worldSize();

    while (reader.next(line)) {
        unsigned int id = std::stoul(line[ID_IDX]);
        int p_rank = std::stoi(line[RANK_IDX]);
        if (p_rank == my_rank || world_size == 1) {
            persons.emplace(id, std::make_shared<HCPerson>(id));
        }
    }
}

}



