#pragma once

#include <string>
#include <vector>

struct JobInfo
{
    int id_job;
    int release_date;
    double due_date;
    double earliness_penalty;
    double tardiness_penalty;
    double flow_time;
};

struct Operation
{
    int id_operation;
    int id_job;
    int id_machine;
    double processing_time;
    double setup_time;
};

struct ValidationOp
{
    int id_operation;
    int id_job;
    int id_machine;
    double processing_time;
    double setup_time = 0;
    double release_date;
    double final_time;
    double start_time;
};

class Instance
{
public:
    std::vector<std::vector<std::vector<int>>> setupMatrix;
    std::vector<int> processingTime;
    std::vector<int> operToJob;
    std::vector<int> operToMach;

    std::vector<std::vector<int>> jobOperation;
    std::vector<std::vector<int>> machOperation;

    std::vector<int> predecessorJob;
    std::vector<int> successorJob;

    std::vector<int> predecessorMach;
    std::vector<int> successorMach;

    void initializeInstance(const std::string &operationPath, const std::string &setupPath, int machineCount, int jobCount);

private:
    void generateOperToJob(const std::vector<Operation> &operations_list);
    void generateOperToMach(const std::vector<Operation> &operations_list);
    void generateJobOperation(const std::vector<Operation> &operations_list);
    void generateMachOperation(const std::vector<Operation> &operations_list);
    void generateProcessingTime(const std::vector<Operation> &operations_list);
    void generatePredecessorJob();
    void generateSuccessorJob();
    void generatePredecessorMach();
    void generateSuccessorMach();
    void configure(int n_mach, int n_jobs, const std::vector<Operation> &operatons_list);
};
