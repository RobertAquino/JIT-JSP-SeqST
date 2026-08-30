#ifndef VALUER_HPP
#define VALUER_HPP
#include "Instance.hpp"
#include <vector>

std::vector<double> valuer(const std::vector<Operation> &operations_list, Instance &instance, double &makespan,
                           std::vector<int> &criticalPredecessor, const std::vector<JobInfo> &jobs_list,
                           std::vector<ValidationOp> &certificate, std::vector<double> &start_time_job, std::vector<int> &initialSetup);

void calculateComponents(int &n_mach, int &n_jobs, std::vector<Operation> &operations_list);

std::vector<double> calculator(std::string operationPath, std::string jobPath, std::string setupPath, double &makespan,
                               std::vector<int> &criticalPredecessor, std::vector<ValidationOp> &certificate,
                               std::vector<double> &start_time_job, std::vector<int> &initialSetup);

#endif