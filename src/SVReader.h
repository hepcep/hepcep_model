
/*
 * CSVReader.h
 *
 *  Created on: Apr 17, 2012
 *      Author: nick
 */

#ifndef SVREADER_H_
#define SVREADER_H_

#include <vector>
#include <string>
#include <fstream>

namespace hepcep {

/**
 * Reads CSV formatted data files. Each line of the file is read into a vector
 * where each element of the vector is a data element from that line.
 */
class SVReader {
public:

	/**
	 * Create a CSVReader that will read the specified file.
	 */
	SVReader(const std::string& file, const char separator = ',');

	// copy constructor
	SVReader(const SVReader& reader);

	virtual ~SVReader();

	/**
	 * Reads the next line into the vector, returning true
	 * if a line was read, otherwise false. This will return
	 * false when it reaaches the end of the file.
	 */
	bool next(std::vector<std::string>& vec);

	/**
	 * Skips the specified number of lines in the file.
	 */
	void skip(int lines);

	// assignment operator
	SVReader& operator=(const SVReader& rhs);

private:
	void findDelimeter();

	std::string fname;
	char delim;
	char sep;
	std::ifstream in;
};

}

#endif /* SVREADER_H_ */
