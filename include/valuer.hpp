#pragma once
#include "instancePaths.hpp"
#include "instance.hpp"
#include <vector>

std::vector<double> valuer(Instance &instance, double &makespan, std::vector<int> &criticalPredecessor,
                           std::vector<ValidationOp> &certificate, std::vector<double> &startTimeJob);

std::vector<double> calculator(const InstancePaths &instancePaths, double &makespan,
                               std::vector<int> &criticalPredecessor, std::vector<ValidationOp> &certificate,
                               std::vector<double> &startTimeJob);