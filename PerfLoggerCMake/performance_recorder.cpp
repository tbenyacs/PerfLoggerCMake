#include "performance_recorder.h"

void PerformanceRecorder::setCSVFileName(const std::string& filename)
{
    std::lock_guard<std::mutex> lock(sm_Mutex);
    sm_CsvFileName = filename;
}

PerformanceRecorder::PerformanceRecorder(const std::string& functionIdentifier, VerbLevels verbosity)
    : m_FnID(functionIdentifier), m_VerboseLevel(verbosity)
{
    start();
}

void PerformanceRecorder::start()
{
    m_Start_time = std::chrono::high_resolution_clock::now();
    m_Stopped = false;
}

void PerformanceRecorder::stopAndWrite()
{
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

PerformanceRecorder::~PerformanceRecorder()
{
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
        file << "Label,NumCalls,MinTime,MaxTime,AvgTime,StdDev,TotalTime\n";
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
                << "num_calls:" << numCalls << ","
                << "min:" << minTime << "s,"
                << "max:" << maxTime << "s,"
                << "avg:" << avgTime << "s,"
                << "stddev:" << stdDevTime << "s,"
                << "total:" << sumTime << "s\n";
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
