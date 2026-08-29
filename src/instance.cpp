#include "../include/instance.hpp"
#include "../include/operationParser.hpp"
#include "../include/setupParser.hpp"

void Instance::configure(int machineCount, int jobCount, const std::vector<Operation> &operationsList)
{
    int totalOperations = operationsList.size();

    operToJob.resize(totalOperations, 0);
    operToMach.resize(totalOperations, 0);
    processingTime.resize(totalOperations, 0);
    jobOperation.resize(jobCount);
    machOperation.resize(machineCount);
    predecessorJob.resize(totalOperations, -1);
    successorJob.resize(totalOperations, -1);
    predecessorMach.resize(totalOperations, -1);
    successorMach.resize(totalOperations, -1);
}

void Instance::generateOperToJob(const std::vector<Operation> &operationsList)
{
    for (size_t i = 0; i < operationsList.size(); i++)
    {
        operToJob[operationsList[i].id_operation] = operationsList[i].id_job;
    }
}

void Instance::generateOperToMach(const std::vector<Operation> &operationsList)
{
    for (size_t i = 0; i < operationsList.size(); i++)
    {
        operToMach[operationsList[i].id_operation] = operationsList[i].id_machine;
    }
}

void Instance::generateJobOperation(const std::vector<Operation> &operationsList)
{
    for (size_t i = 0; i < operationsList.size(); i++)
    {
        jobOperation[operationsList[i].id_job].push_back(operationsList[i].id_operation);
    }
}

void Instance::generateMachOperation(const std::vector<Operation> &operationsList)
{
    for (size_t i = 0; i < operationsList.size(); i++)
    {
        machOperation[operationsList[i].id_machine].push_back(operationsList[i].id_operation);
    }
}

void Instance::generateProcessingTime(const std::vector<Operation> &operationsList)
{
    for (size_t i = 0; i < operationsList.size(); i++)
    {
        processingTime[operationsList[i].id_operation] = operationsList[i].processing_time;
    }
}

void Instance::generatePredecessorJob()
{
    for (const auto &queue : jobOperation)
    {
        for (size_t i = 1; i < queue.size(); i++)
        {
            predecessorJob[queue[i]] = queue[i - 1];
        }
    }
}

void Instance::generateSuccessorJob()
{
    for (const auto &queue : jobOperation)
    {
        for (size_t i = 0; i + 1 < queue.size(); i++)
        {
            successorJob[queue[i]] = queue[i + 1];
        }
    }
}

void Instance::generatePredecessorMach()
{
    for (const auto &queue : machOperation)
    {
        for (size_t i = 1; i < queue.size(); i++)
        {
            predecessorMach[queue[i]] = queue[i - 1];
        }
    }
}

void Instance::generateSuccessorMach()
{
    for (const auto &queue : machOperation)
    {
        for (size_t i = 0; i + 1 < queue.size(); i++)
        {
            successorMach[queue[i]] = queue[i + 1];
        }
    }
}

void Instance::initializeInstance(const std::string &operationPath, const std::string &setupPath, int machineCount, int jobCount)
{
    std::vector<Operation> operationsList = parseOperation(operationPath);
    configure(machineCount, jobCount, operationsList);

    setupMatrix = parseSetup(setupPath, machineCount, jobCount);

    generateOperToJob(operationsList);
    generateOperToMach(operationsList);
    generateJobOperation(operationsList);
    generateMachOperation(operationsList);
    generateProcessingTime(operationsList);
    generatePredecessorJob();
    generateSuccessorJob();
    generatePredecessorMach();
    generateSuccessorMach();
}
