#pragma once
#include "instance.hpp"
#include "movement.hpp"
#include "tabuSearch.hpp"

#include <vector>

struct TimingResult
{
    double cost = 0.0;
    double makespan = 0.0;
    bool feasible = true;
    bool optimal = false;

    std::vector<double> startTimeOp;
    std::vector<double> finalTimeOp;

    std::vector<double> startTimeJob;
    std::vector<double> finalTimeJob;

    std::vector<double> earliness;
    std::vector<double> tardiness;
    std::vector<double> flowTime;
};

TimingResult solveTime(const SolutionState &solutionState, Instance &instance);