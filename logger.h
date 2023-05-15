#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>
#include <ctime>
#include <iostream>
#include <sstream>

std::string getCurrentTime();

// Define the base case function directly in the header file
inline void writeToLogFileHelper(std::ostringstream& logStream) {
    // Do nothing, end of recursion
}

template<typename... Args>
void writeToLogFile(Args... args);

// Function implementations

// Recursive variadic template function to process the arguments
template<typename T, typename... Args>
void writeToLogFileHelper(std::ostringstream& logStream, T value, Args... args) {
    logStream << value << " ";
    writeToLogFileHelper(logStream, args...);
}

// Function to write a message to the log file
template<typename... Args>
void writeToLogFile(Args... args) {
    std::ofstream logFileStream;
    std::ostringstream logMessageStream;
    std::string logFile = "blunder-matic.log";

    // Process the arguments and create the message string
    writeToLogFileHelper(logMessageStream, args...);
    std::string message = logMessageStream.str();

    // Open the log file in append mode
    logFileStream.open(logFile, std::ios_base::app);

    if (logFileStream.is_open()) {
        // Write the timestamp and message to the log file
        logFileStream << "[" << getCurrentTime() << "] " << message << std::endl;

        // Close the log file
        logFileStream.close();
    } else {
        std::cerr << "Unable to open log file: " << logFile << std::endl;
    }
}


void clearLogs();
#endif
