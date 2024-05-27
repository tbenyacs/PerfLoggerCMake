// PerfLoggerCMake.cpp : Defines the entry point for the application.
//

//#include "PerfLoggerCMake.h"
#include "performance_recorder.h"

// Example functions
void foo() {
    PerformanceRecorder main_recorder("foo");
    //PerformanceRecorder recorder("foo", PerformanceRecorder::VerbLevel_Low);
    for (int i = 0; i < 1000000; ++i);  // Simulate work
}

void bar() {
    PerformanceRecorder main_recorder("bar");
    //PerformanceRecorder recorder("bar", PerformanceRecorder::VerbLevel_Low);
    for (int i = 0; i < 3000000; ++i);  // Simulate work
}

void baz() {
    PerformanceRecorder main_recorder("baz");
    //PerformanceRecorder recorder("baz", PerformanceRecorder::VerbLevel_Low);
    for (int i = 0; i < 4000000; ++i);  // Simulate work
}

int main() {
    // Set CSV file name (optional)
    //PerformanceRecorder::setCSVFileName("performance_data.csv");

    // Call example functions multiple times
    for (int i = 0; i < 15; ++i) foo();
    for (int i = 0; i < 29; ++i) bar();
    for (int i = 0; i < 34; ++i) baz();

    // Generate summary report
    PerformanceRecorder::generateSummary();

    return 0;
}
