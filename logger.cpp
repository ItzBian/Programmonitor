#include "logger.h"

std::vector<std::string> logs; // Definition of logs

void AddLog(const std::string& message) { // Definition of AddLog
    logs.push_back(message);
}
