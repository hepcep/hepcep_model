
/*
 * CSVReader.cpp
 *
 *  Created on: Apr 17, 2012
 *      Author: nick
 */

#include "SVReader.h"

#include <boost/tokenizer.hpp>

namespace hepcep {

using namespace std;
using namespace boost;

SVReader::SVReader(const string& file, const char separator) : fname(file),  delim('\n'), sep(separator),
		in(file.c_str()) {
	// if we can't open the file throw exception
	if (!in.is_open()) throw invalid_argument("Error opening: " + file);
	findDelimeter();

}

SVReader::SVReader(const SVReader& reader) : fname(reader.fname), delim(reader.delim), sep(reader.sep), in(reader.fname.c_str()) {
	// if we can't open the file throw exception
	if (!in.is_open()) throw invalid_argument("Error opening: " + fname);
}

void SVReader::findDelimeter() {

	char c;
	while (in.good()) {
	    c = in.get();
	    if (in.good()) {
	    	if (c == '\n') {
	    		delim = '\n';
	    		break;
	    	}
	    	if (c == '\r') {
	    		delim = '\r';
	    		break;
	    	}
	    }
	}
	in.close();
	in.open(fname.c_str());
}

void SVReader::skip(int lines) {
	int count = 0;
	string str;
	while (count < lines && getline(in, str)) {
		++count;
	}
}

bool SVReader::next(vector<string>& vec) {
	string line;

	// read the line
	if (getline(in, line, delim)) {
		// tokenize the line using boost's escaped list separator
		// which parses CSV format

		// Erase leading carriage return char
		if (line[0] == '\n' || line[0] == '\r'){
			line.erase(0,1);
		}

		escaped_list_separator<char> els('\\', sep, '\"');
		tokenizer<escaped_list_separator<char> > tok(line, els);
		// assign those values to the vector
		vec.assign(tok.begin(), tok.end());
		return true;
	}
	// return false when there are no more lines to
	// read
	return false;
}

// copy the file name and close the existing file
// and reopen it with the new file.
SVReader& SVReader::operator=(const SVReader& rhs) {
	if (&rhs != this) {
		fname = rhs.fname;
		in.close();
		delim = rhs.delim;
		in.open(fname.c_str());
	}

	return *this;
}

SVReader::~SVReader() {
	in.close();
}

}

