#pragma once

#include <string>
#include <vector>

// Reads a CSV file and returns each row already split by comma.
// Skips the first line if skipHeader is true.
// Throws std::runtime_error if the file cannot be opened.
std::vector<std::vector<std::string>> readRows(const std::string &path, bool skipHeader = true);