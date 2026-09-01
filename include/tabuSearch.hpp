#pragma once

#include "instance.hpp"
#include <vector>

void createSPT(Instance &instance, const std::vector<Operation> &operations_list, const std::vector<JobInfo> &jobs_list,
               Solution &solution, int n_mach);

void changePredecessorMach(const Solution &solution, Instance &instance);
void changeSucessorMach(const Solution &solution, Instance &instance);
std::vector<Movement> adjacentNeighborhood(Solution &current);
std::vector<int> generateCriticalPath(std::vector<double> &final_time_job, std::vector<int> &criticalPredecessor, Instance &instance,
                                      double &makespan);
std::vector<Movement> criticalPathNeighborhood(Solution &current, std::vector<int> &criticalPath, Instance &instance);
double penaltySum(const std::vector<double> &final_time, const std::vector<JobInfo> &jobs_list, std::vector<double> &start_time_job);

double TabuSearch(Instance &instance, double &makespan, std::string setupPath, std::string operationPath, std::string jobPath,
                  std::vector<int> &criticalPredecessor, int optionNeighborhood, int optionImprovementStrategy,
                  std::vector<double> &final_time_job, const std::vector<Operation> &operations_list, std::vector<JobInfo> &jobs_list,
                  int n_mach, int &total_iteration, std::vector<ValidationOp> &certificate, std::vector<double> &start_time_job,
                  std::vector<int> &initialSetup);
