/*
 * EnrollmentMethod.h
 *
 *  Enum class for treatment enrollment methods.
 *
 *	 Author: Eric Tatara
 */

#ifndef SRC_ENROLLMENTMETHOD_H_
#define SRC_ENROLLMENTMETHOD_H_


#include <string>
#include <vector>
#include <ostream>

namespace hepcep {

class EnrollmentMethod {

public:
	enum Value {
		unbiased, hrp, fullnetwork, inpartner, outpartner
	};

	static const EnrollmentMethod UNBIASED;
	static const EnrollmentMethod HRP;
	static const EnrollmentMethod FULLNETWORK;
	static const EnrollmentMethod INPARTNER;
	static const EnrollmentMethod OUTPARTNER;

	bool operator==(const EnrollmentMethod& rhs) const;
	bool operator!=(const EnrollmentMethod& rhs) const;
	bool operator<(const EnrollmentMethod& rhs) const;
	std::string stringValue() const;

	const EnrollmentMethod::Value value() const;

	static const std::vector<EnrollmentMethod>& values();

	static EnrollmentMethod valueOf(const std::string& string_val);

private:
	Value val_;
	std::string string_val_;
	static const std::vector<EnrollmentMethod> values_;

	EnrollmentMethod(const Value& val, const std::string& string_val);

};

std::ostream& operator<<(std::ostream& out, const EnrollmentMethod& val);

} /* namespace hepcep */

#endif /* SRC_ENROLLMENTMETHOD_H_ */
