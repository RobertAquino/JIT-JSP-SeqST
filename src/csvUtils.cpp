#include "../include/csvUtils.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

std::vector<std::string> splitLine(const std::string &line, char delimiter = ',')
{
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string field;
    while (std::getline(ss, field, delimiter))
        fields.push_back(field);
    return fields;
}

std::vector<std::vector<std::string>> readRows(const std::string &path, bool skipHeader)
{
    std::ifstream file(path);

    if (!file.is_open())
        throw std::runtime_error("Could not open file: " + path);

    std::vector<std::vector<std::string>> rows;
    std::string line;

    if (skipHeader)
        std::getline(file, line);

    while (std::getline(file, line))
    {
        if (line.empty())
            continue;

        rows.push_back(splitLine(line));
    }

    return rows;
}