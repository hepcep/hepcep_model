/*
 * Statistics.h
 */

#ifndef SRC_STATISTICS_H_
#define SRC_STATISTICS_H_

#include <vector>

#include "AggregateStats.h"
#include "FileOut.h"

namespace hepcep {

enum class LogType {ACTIVATED, EXPOSED, INFECTED, INFECTIOUS, CHRONIC, RECOVERED, DEACTIVATED, INFO, STATUS,
    STARTED_TREATMENT, CURED, REGULAR_STATUS, FAILED_TREATMENT
};

class HCPerson;

struct MeanStats {
    double age, career, daily_inj, in_deg,
        out_deg, sharing;
    unsigned int count;

    void increment(std::shared_ptr<HCPerson> person);
    void calcMean();
    void reset();
    void write(FileOut& out);
    void writeHeader(FileOut& out);
};


class Statistics {

private:
    static Statistics* instance_;
    std::map<std::string, AggregateStats> stats;
    std::vector<std::string> metrics;
    MeanStats means;
    FileOut out;

    Statistics(const std::string& fname);

    void calculatePrevalence(std::map<std::string, double>& prevalences);

public:
    static Statistics* instance();
    static void init(const std::string& fname);
    virtual ~Statistics();

    void logStatusChange(LogType logType, HCPerson* person, const std::string& msg);

    void collectStats(double tick, std::map<unsigned int, std::shared_ptr<HCPerson>>& persons);
    void close();

};

} /* namespace seir */


#endif /* SRC_STATISTICS_H_ */

