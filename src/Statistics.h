/*
 * Statistics.h
 */

#ifndef SRC_STATISTICS_H_
#define SRC_STATISTICS_H_

#include <vector>

#include "AggregateStats.h"
#include "FileOut.h"
#include "LogType.h"
#include "Filter.h"

namespace hepcep {

struct LogEvent {
    double tick;
    LogType type;
    unsigned int person;
    std::string other;
};

struct EventCounts {

  unsigned int activations_daily, cured_daily, aggregate_posttreat,
        losses_daily, incidence_daily, treatment_recruited_daily,
         aggregate_courses;

  void reset();
  void writeHeader(FileOut& out);
  void write(FileOut& out);

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
    std::vector<LogEvent> log_events;
    MeanStats means;
    EventCounts event_counts;
    FileOut out, events_out, arrivingPersonsOut;
    bool burninMode;
    std::shared_ptr<Filter<LogType>> filter_;
    int run_number_;

    Statistics(const std::string& fname, const std::string& events_fname,
    		const std::string& arrivingPersonsFilename, std::shared_ptr<Filter<LogType>>& filter,
            int run_number);

    void calculatePrevalence(std::map<std::string, double>& prevalences);
    void writeEvents();

public:
    static Statistics* instance();
    static void init(const std::string& fname, const std::string& events_fname,
    		const std::string&, std::shared_ptr<Filter<LogType>>& filter, int run_number);
    virtual ~Statistics();

    void logStatusChange(LogType logType, PersonPtr person, const std::string& msg);
    void logStatusChange(LogType logType, HCPerson* person, const std::string& msg);

    void recordStats(double tick, int run, std::map<unsigned int, std::shared_ptr<HCPerson>>& persons);
    void close();
    void setBurninMode(bool mode);

    int getDailyLosses();

    void logPersonArrival(PersonPtr person);

};

} /* namespace seir */


#endif /* SRC_STATISTICS_H_ */

