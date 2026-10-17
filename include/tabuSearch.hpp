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
#include "../include/timingSolver.hpp"
#include <vector>

struct SolutionState
{
    Solution solution;

    double makespan = 0;
    double cost = 0;

    bool feasible = false;
    bool timingOptimal = false;
};

void changeMachines(const Solution &solution, Instance &instance);

double TabuSearch(Instance &instance, Parameters &parameters, int machineCount, std::vector<ValidationOp> &certificate,
                  std::vector<double> &startTimeJob, double &bestMakespan, std::vector<double> &finalTimeJob);
