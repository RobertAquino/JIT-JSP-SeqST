#pragma once

#include "instance.hpp"

#include <vector>

struct ASAPResult
{
    double cost = 0.0;
    bool feasible = false;

    double makespan = 0.0;

    std::vector<double> finalTimeOperation;
    std::vector<double> finalTimeJob;

    std::vector<int> criticalPredecessor;
};

ASAPResult calculateASAP(const Instance &instance);