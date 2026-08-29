#ifndef ESTRUTURAS_HPP
#define ESTRUTURAS_HPP
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

class JobInfo
{
public:
    int id_job;
    int release_date;
    double due_date;
    double earliness_penalty;
    double tardiness_penalty;
    double flow_time;
};

class Operation
{
public:
    int id_operation;
    int id_job;
    int id_machine;
    double processing_time;
    double setup_time;
};

class ValidationOp
{
public:
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
    std::vector<std::vector<std::vector<int>>> setup_matrix;
    std::vector<int> processing_time;
    std::vector<int> operToJob;
    std::vector<int> operToMach;

    std::vector<std::vector<int>> jobOperation;
    std::vector<std::vector<int>> machOperation;

    std::vector<int> predecessorJob;
    std::vector<int> sucessorJob;

    std::vector<int> predecessorMach;
    std::vector<int> sucessorMach;

    void generateOperToJob(const std::vector<Operation> &operations_list);
    void generateOperToMach(const std::vector<Operation> &operations_list);
    void generateJobOper(const std::vector<Operation> &operations_list);
    void generateMachOper(const std::vector<Operation> &operations_list);
    void generateTempoProcessamento(const std::vector<Operation> &operations_list);
    void generatePredecessorJob();
    void generateSucessorJob();
    void generatePredecessorMach();
    void generateSucessorMach();
    void initializeInstance(std::string operation_path, std::string setup_path, int n_mach, int n_jobs);
    void configure(int n_mach, int n_jobs, const std::vector<Operation> &operatons_list);
};

#endif