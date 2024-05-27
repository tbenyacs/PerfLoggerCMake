// PerfLoggerCMake.cpp : Defines the entry point for the application.
//

#include "PerfLoggerCMake.h"

void PerformanceRecorder::setCSVFileName(const std::string& filename)
{
    std::lock_guard<std::mutex> lock(sm_Mutex);
    sm_CsvFileName = filename;
}

PerformanceRecorder::PerformanceRecorder(const std::string& functionIdentifier, VerbLevels verbosity)
    : m_FnID(functionIdentifier), m_VerboseLevel(verbosity) {
    start();
}

void PerformanceRecorder::start() {
    m_Start_time = std::chrono::high_resolution_clock::now();
    m_Stopped = false;
}

void PerformanceRecorder::stopAndWrite() {
    if (m_Stopped) return;

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end_time - m_Start_time;
    double execTime = duration.count();

    std::lock_guard<std::mutex> lock(sm_Mutex);
    if (sm_CsvFileName.empty()) {
        sm_CsvFileName = "HD_PerformanceRecorder_Report_" + getTimestamp() + ".csv";
    }

    if (m_VerboseLevel > VerbLevel_Low) {
        // Log the execution time to the CSV file. Open file in append mode
        std::ofstream file(sm_CsvFileName, std::ios::app);
        if (file.is_open()) {
            if (!sm_AnalHeader) {
                const std::time_t t_c = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                file << "\nFunction performance analytics by PerformanceRecorder at " << std::ctime(&t_c);
                file << "Function,Start Time,End Time,Duration (sec)\n";
                sm_AnalHeader = true;
            }
            file << m_FnID << ",";
            file << std::chrono::duration_cast<std::chrono::milliseconds>(m_Start_time.time_since_epoch()).count() << ",";
            file << std::chrono::duration_cast<std::chrono::milliseconds>(end_time.time_since_epoch()).count() << ",";
            file << execTime << "\n";
            file.close();
        }
        else {
            std::cerr << "Unable to open file: " << sm_CsvFileName << std::endl;
        }
    }

    // Update statistics
    sm_FnPerformances.push_back({ m_FnID, execTime });
    m_Stopped = true;
}

PerformanceRecorder::~PerformanceRecorder() {
    if (!m_Stopped) {
        stopAndWrite();
    }
}

void PerformanceRecorder::generateSummary()
{
    std::lock_guard<std::mutex> lock(sm_Mutex);
    std::ofstream file(sm_CsvFileName, std::ios::app);
    if (file.is_open()) {
        const std::time_t t_c = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        file << "\nFunction performance summary at " << std::ctime(&t_c);
        file << "Function,Calls,Min.Exec Time,Max.Exec Time,Avg Exec.Time,Std.Dev,Sum Exec.Time\n";
        std::map<std::string, std::vector<double>> funcExecTimes;

        for (const auto& record : sm_FnPerformances) {
            funcExecTimes[record.FnID].push_back(record.runtime);
        }

        for (const auto& entry : funcExecTimes) {
            const std::string& funcId = entry.first;
            const std::vector<double>& times = entry.second;

            int numCalls = times.size();
            double minTime = *std::min_element(times.begin(), times.end());
            double maxTime = *std::max_element(times.begin(), times.end());
            double sumTime = std::accumulate(times.begin(), times.end(), 0.0); 1.0;
            double avgTime = sumTime / numCalls;

            double sumSquaredDiffs = std::accumulate(times.begin(), times.end(), 0.0,
                [avgTime](double acc, double t) { return acc + (t - avgTime) * (t - avgTime); });
            double stdDevTime = std::sqrt(sumSquaredDiffs / numCalls);

            file << funcId << ","
                << numCalls << ","
                << minTime << ","
                << maxTime << ","
                << avgTime << ","
                << stdDevTime << ","
                << sumTime << "\n";
        }
        file.close();
    }
}

std::string PerformanceRecorder::getTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm;

    {
        std::lock_guard<std::mutex> lock(sm_TimeMutex);
        tm = *std::localtime(&time);
    }

    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d_%H-%M.%S.") << std::setw(3) << std::setfill('0') << ms.count();

    return oss.str();
}

// Initialize static members
std::string PerformanceRecorder::sm_CsvFileName;
std::vector<PerformanceRecorder::FnPerfRec> PerformanceRecorder::sm_FnPerformances;
std::mutex PerformanceRecorder::sm_Mutex;
std::mutex PerformanceRecorder::sm_TimeMutex;
bool PerformanceRecorder::sm_AnalHeader = false;

// Example functions
void foo() {
    PerformanceRecorder recorder("foo");
    //PerformanceRecorder recorder("foo", PerformanceRecorder::VerbLevel_Low);
    for (int i = 0; i < 1000000; ++i);  // Simulate work
}

void blah() {
    PerformanceRecorder recorder("blah");
    //PerformanceRecorder recorder("blah", PerformanceRecorder::VerbLevel_Low);
    for (int i = 0; i < 2000000; ++i);  // Simulate work
}

void bar() {
    PerformanceRecorder recorder("bar");
    //PerformanceRecorder recorder("bar", PerformanceRecorder::VerbLevel_Low);
    for (int i = 0; i < 3000000; ++i);  // Simulate work
}

void baz() {
    PerformanceRecorder recorder("baz");
    //PerformanceRecorder recorder("baz", PerformanceRecorder::VerbLevel_Low);
    for (int i = 0; i < 4000000; ++i);  // Simulate work
}

int main() {
    // Set CSV file name (optional)
    //PerformanceRecorder::setCSVFileName("performance_data.csv");

    // Call example functions multiple times
    for (int i = 0; i < 15; ++i) foo();
    for (int i = 0; i < 53; ++i) blah();
    for (int i = 0; i < 29; ++i) bar();
    for (int i = 0; i < 34; ++i) baz();

    // Generate summary report
    PerformanceRecorder::generateSummary();

    return 0;
}
