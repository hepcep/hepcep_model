/*
 * network_utils.cpp
 *
 *  Created on: Jan 26, 2018
 *      Author: nick
 */

#include <iomanip>

#include <boost/filesystem.hpp>
#include "chi_sim/file_utils.h"

#include "network_utils.h"

namespace fs = boost::filesystem;

namespace hepcep {

void open_ofstream(const std::string& filename, std::ofstream& out) {
    std::string fname = chi_sim::unique_file_name(filename);
    fs::path filepath(fname);
    if (!fs::exists(filepath.parent_path())) {
        fs::create_directories(filepath.parent_path());
    }
    out.open(filepath.string().c_str());
}


AttributeWriter::AttributeWriter(std::ofstream& out) : out_{out} {}
AttributeWriter::~AttributeWriter() {}

void AttributeWriter::operator() (const std::string& name, double value) {
    out_ << std::setprecision(8) << std::fixed << INDENT_2 << name << " " << value << "\n";
}

void AttributeWriter::operator() (const std::string& name, const std::string& value) {
    out_ << INDENT_2 << name << " " << value << "\n";
}

}
