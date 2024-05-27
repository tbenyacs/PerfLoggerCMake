#pragma once

#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <ctime>
#include <iomanip>
#include <vector>
#include <map>
#include <cmath>
#include <mutex>
#include <sstream>
#include <numeric>

class PerformanceRecorder {
public:
    enum VerbLevels { VerbLevel_Low, VerbLevel_Mid, VerbLevel_High };

    struct FnPerfRec {
        std::string FnID;
        double runtime;
    };

    PerformanceRecorder(const std::string& functionIdentifier, VerbLevels verbosity = VerbLevel_High);

    void start();
    void stopAndWrite();

    ~PerformanceRecorder();

    static void setCSVFileName(const std::string& filename);
    static void generateSummary();

private:
    // Static members
    static std::string sm_CsvFileName;
    static std::vector<FnPerfRec> sm_FnPerformances;
    static std::mutex sm_Mutex;
    static bool sm_AnalHeader;
    static std::mutex sm_TimeMutex; // to protect std::localtime
    static std::string getTimestamp();

    // Instance members
    std::chrono::high_resolution_clock::time_point m_Start_time;
    std::string m_FnID;
    bool m_Stopped = true;
    VerbLevels m_VerboseLevel;
};
