// PerfLoggerCMake.cpp : Defines the entry point for the application.
//

#include "PerfLoggerCMake.h"

using namespace std;


class RuntimeMeasurer {
public:
    // Constructor to initialize the filename
    RuntimeMeasurer(const std::string& filename) : filename(filename) {}

    // Method to start timing
    void start() {
        start_time = std::chrono::high_resolution_clock::now();
    }

    // Method to stop timing and write the result to the CSV file
    void stopAndWrite() {
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end_time - start_time;

        // Open file in append mode
        std::ofstream file;
        file.open(filename, std::ios::app);

        if (file.is_open()) {
            file << "Start Time,End Time,Duration (seconds)\n";
            file << std::chrono::duration_cast<std::chrono::milliseconds>(start_time.time_since_epoch()).count() << ",";
            file << std::chrono::duration_cast<std::chrono::milliseconds>(end_time.time_since_epoch()).count() << ",";
            file << duration.count() << "\n";
            file.close();
        }
        else {
            std::cerr << "Unable to open file: " << filename << std::endl;
        }
    }

private:
    std::string filename;
    std::chrono::high_resolution_clock::time_point start_time;
};

class Timer {
public:
    // Constructor that starts the timer and sets the CSV file name
    Timer(const std::string& file_name)
        : start_time_point(std::chrono::high_resolution_clock::now()), csv_file_name(file_name) {}

    // Destructor that stops the timer and writes the duration to the CSV file
    ~Timer() {
        stop();
    }

    // Method to stop the timer and write the elapsed time to the CSV file
    void stop() {
        auto end_time_point = std::chrono::high_resolution_clock::now();
        auto start = std::chrono::time_point_cast<std::chrono::microseconds>(start_time_point).time_since_epoch().count();
        auto end = std::chrono::time_point_cast<std::chrono::microseconds>(end_time_point).time_since_epoch().count();

        auto duration = end - start;
        double ms = duration * 0.001;

        std::ofstream file;
        file.open(csv_file_name, std::ios_base::app); // Open the file in append mode
        if (file.is_open()) {
            file << "Duration (us),Duration (ms)\n";
            file << duration << "," << ms << "\n";
            file.close();
        }
        else {
            std::cerr << "Unable to open file: " << csv_file_name << "\n";
        }
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time_point;
    std::string csv_file_name;
};

// Example function to demonstrate the Timer class usage
void testFunction(const std::string& file_name) {
    Timer timer(file_name); // Timer starts here

    // Simulate some work whose runtime you want to measure
    for (volatile int i = 0; i < 1000000; ++i);

    // Timer stops and writes duration to the CSV file when it goes out of scope
}

int main() {
    RuntimeMeasurer measurer("runtime_data.csv");

    measurer.start();
    // Code block whose runtime you want to measure
    for (int i = 0; i < 1000000; ++i);
    measurer.stopAndWrite();

    const std::string file_name = "timing_data.csv";
    testFunction(file_name);

    return 0;
}
