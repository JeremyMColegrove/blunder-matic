#include "logger.h"


// Function to get the current time as a string
std::string getCurrentTime() {
    std::time_t now = std::time(0);
    std::tm* timeStruct = std::localtime(&now);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeStruct);
    return std::string(buffer);
}
