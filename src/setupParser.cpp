
#include "../include/setupParser.hpp"
#include "../include/instance.hpp"
#include "../include/csvUtils.hpp"
#include <stdexcept>

std::vector<std::vector<std::vector<int>>> startParser(std::string path, int machineCount, int jobCount)
{
    std::vector<std::vector<std::vector<int>>> setupMatrix(machineCount,
                                                           std::vector<std::vector<int>>(jobCount, std::vector<int>(jobCount, 0)));

    for (const auto &fields : readRows(path))
    {
        int mach = std::stoi(fields[0]);
        int job1 = std::stoi(fields[1]);
        int job2 = std::stoi(fields[2]);
        int tempo = std::stoi(fields[3]);

        bool validIndex = mach >= 0 && mach < machineCount &&
                          job1 >= 0 && job1 < jobCount &&
                          job2 >= 0 && job2 < jobCount;

        if (!validIndex)
            throw std::runtime_error("invalid index in setup file" + path);

        setupMatrix[mach][job1][job2] = tempo;
    }
    return setupMatrix;
}