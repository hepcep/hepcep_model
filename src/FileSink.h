/*
 * FileSink.h
 *
 *  Created on: Oct 4, 2016
 *      Author: nick
 */

#ifndef SRC_FILESINK_H_
#define SRC_FILESINK_H_

#include <fstream>
#include <string>

#include <boost/filesystem.hpp>

#include "chi_sim/file_utils.h"

#include "DataSet.h"

namespace fs = boost::filesystem;

namespace hepcep {

const int BUFFER_SIZE = 2000;

template<typename T>
class FileSink {

private:
	int rank_, run_;
	std::ofstream out;
	std::shared_ptr<ReducibleDataSet<T>> data_set_;
	unsigned int record_count;

	void write();

public:
	FileSink(int rank,int run, std::shared_ptr<ReducibleDataSet<T>> data_set);
	virtual ~FileSink();

	void record(double tick);
	void open(const std::string& file_name, bool append = false);
};


template<typename T>
FileSink<T>::FileSink(int rank, int run, std::shared_ptr<ReducibleDataSet<T>> data_set) : rank_{rank},
run_{run}, out(), data_set_(data_set), record_count(0) {
}

template<typename T>
void FileSink<T>::open(const std::string& file_name, bool append) {
	if (rank_ == 0) {
		std::string fname = file_name;
		if (!append) {
			fname = chi_sim::unique_file_name(file_name);
		}

		fs::path filepath(fname);
		if (!fs::exists(filepath.parent_path()))
			fs::create_directories(filepath.parent_path());

		out.open(fname.c_str(),
				append ? std::ofstream::app : std::ofstream::out);
		out << "run,tick";

		for (auto name : data_set_->columnNames()) {
			out << "," << name;
		}
		out << "\n";
	}
}

template<typename T>
FileSink<T>::~FileSink() {
	write();
	out << "\n";
	out.flush();
	out.close();
}

template<typename T>
void FileSink<T>::record(double tick) {
	data_set_->record(tick);

	++record_count;
	if (record_count > BUFFER_SIZE) {
		write();
	}

}

template<typename T>
void FileSink<T>::write() {
	std::vector<double> data;
	data_set_->write(data);

	if (rank_ == 0) {
		size_t stride_length = data_set_->columnNames().size();
		// + 1 for the tick column
		for (unsigned int i = 0, n = data.size(); i < n; i += stride_length + 1) {
			out << run_;
			for (int j = 0, m = stride_length + 1; j < m; ++j) {
				out << "," << data[i + j];
			}

			if (i + stride_length < n - 1) {
				out << "\n";
			}
		}
	}
}

}



#endif /* SRC_FILESINK_H_ */
