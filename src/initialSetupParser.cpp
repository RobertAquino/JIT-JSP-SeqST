#include "../include/initialSetupParser.hpp"
#include "../include/csvUtils.hpp"
#include <stdexcept>

std::vector<int> parseInitialSetup(const std::string path, int machineCount)
{
    std::vector<int> initialSetup;
    initialSetup.resize(machineCount, -1);

    for (const auto &fields : readRows(path))
    {
        int machine = std::stoi(fields[0]);
        int job = std::stoi(fields[1]);

        if (machine < 0 && machine >= machineCount)
            throw std::runtime_error("Invalid machine index in initial setup file: " + std::to_string(machine));

        initialSetup[machine] = job;
    }

    return initialSetup;
}
