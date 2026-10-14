#pragma once

#include "instance.hpp"
#include "../include/instance.hpp"
#include "../include/valuer.hpp"
#include "../include/movement.hpp"
#include "../include/parameters.hpp"
#include "../include/neighborhoodGenerators.hpp"
#include "../include/tabuList.hpp"
#include "../include/cycleDetector.hpp"
#include "../include/checkpointStack.hpp"
#include <vector>

struct SolutionState
{
    Solution solution;

    double makespan = 0;
    double cost = 0;

    bool feasible = false;

    std::vector<double> finalTimeJob;
    std::vector<double> startTimeJob;
    std::vector<int> criticalPredecessor;
    std::vector<ValidationOp> certificate;
};

void changeMachines(const Solution &solution, Instance &instance);

double TabuSearch(Instance &instance, Parameters &parameters, std::vector<int> &criticalPredecessor,
                  std::vector<double> &finalTimeJob, int machineCount, std::vector<ValidationOp> &certificate,
                  std::vector<double> &startTimeJob, double &bestMakespan);
