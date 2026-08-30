#include "../include/instance.hpp"
#include <vector>
#include <queue>

std::vector<int> computeDegrees(const Instance &instance, int size)
{
    std::vector<int> degree(size, 0);

    for (int i = 0; i < size; i++)
    {
        if (instance.predecessorJob[i] != -1)
            degree[i]++;

        if (instance.predecessorMach[i] != -1)
            degree[i]++;
    }

    return degree;
}
std::queue<int> buildInitialReadyQueue(const std::vector<int> &degree, int size)
{
    std::queue<int> readyQueue;

    for (int i = 0; i < size; i++)
    {
        if (degree[i] == 0)
            readyQueue.push(i);
    }

    return readyQueue;
}

double computeJobReadyTime(const Instance &instance, std::vector<double> &finalTimeOperation, int current)
{
    int predecessorJob = instance.predecessorJob[current];

    if (predecessorJob == -1)
        return instance.jobsList[instance.operationsList[current].id_job].release_date;

    return finalTimeOperation[predecessorJob];
}

double computeMachineReadyTime(const Instance &instance, std::vector<double> &finalTimeOperation, int current)
{
    int predecessorMachine = instance.predecessorMach[current];

    if (predecessorMachine == -1)
        return 0.0;

    return finalTimeOperation[predecessorMachine];
}

double selectBottleneck(double timeJobFinal, double timeMachineFinal, int predecessorJob, int predecessorMach,
                        int &criticalPredecessorOut)
{
    if (timeJobFinal > timeMachineFinal)
    {
        criticalPredecessorOut = predecessorJob;
        return timeJobFinal;
    }
    else
    {
        criticalPredecessorOut = predecessorMach;
        return timeMachineFinal;
    }
}

double computeSetupTime(const Instance &instance, int current)
{
    int machine = instance.operationsList[current].id_machine;
    int job = instance.operationsList[current].id_job;
    int predecessorMachine = instance.predecessorMach[current];

    if (predecessorMachine != -1)
    {
        int previousJob = instance.operationsList[predecessorMachine].id_job;
        return instance.setupMatrix[machine][previousJob][job];
    }

    int initialJob = instance.initialSetup[machine];
    if (initialJob == -1 || initialJob == job)
        return 0.0;

    return instance.setupMatrix[machine][initialJob][job];
}

ValidationOp buildCertificateEntry(const Instance &instance, int current, double startOperation, double setupTime, double duration)
{
    ValidationOp entry;

    entry.id_operation = current;
    entry.id_job = instance.operationsList[current].id_job;
    entry.id_machine = instance.operationsList[current].id_machine;
    entry.release_date = startOperation;
    entry.start_time = startOperation;
    entry.processing_time = instance.processingTime[current];
    entry.setup_time = setupTime;
    entry.final_time = duration;

    return entry;
}

void releaseSuccessorJob(std::queue<int> &readyQueue, std::vector<int> &degree, const Instance &instance, int current, bool &isLastOperation)
{
    int successor = instance.successorJob[current];

    if (successor == -1)
    {
        isLastOperation = true;
        return;
    }

    degree[successor]--;

    if (degree[successor] == 0)
        readyQueue.push(successor);
}

void releaseSuccessorMachine(std::queue<int> &readyQueue, std::vector<int> &degree, const Instance &instance, int current)
{
    int successor = instance.successorMach[current];

    if (successor == -1)
        return;

    degree[successor]--;

    if (degree[successor] == 0)
        readyQueue.push(successor);
}

std::vector<double> buildInfeasibleResult(int jobCount, double &makespan)
{
    makespan = 999999999.0;
    return std::vector<double>(jobCount, 999999999.0);
}

double findMakespan(const std::vector<double> &jobsCost)
{
    double makespan = 0.0;

    for (double time : jobsCost)
    {
        if (time > makespan)
            makespan = time;
    }

    return makespan;
}
std::vector<double> valuer(Instance &instance, double &makespan, std::vector<int> &criticalPredecessor,
                           std::vector<ValidationOp> &certificate, std::vector<double> &startTimeJob)
{
    startTimeJob.clear();
    certificate.clear();

    double startTimeOperation;
    double finalTimeJob;
    double finalTimeMachine;
    int size = instance.operationsList.size();
    int jobsCount = instance.jobsList.size();
    int processed = 0;
    int predecessorJob;
    int predecessorMach;
    int criticalPredecessorOut;
    int currentJob;

    std::vector<double> jobsCost(jobsCount, 0.0);
    std::vector<double> finalTimeOperation;
    std::vector<int> topological_list;
    std::vector<int> degree = computeDegrees(instance, size);
    std::queue<int> readyQueue = buildInitialReadyQueue(degree, size);

    finalTimeOperation.resize(size, 0);
    criticalPredecessor.resize(size, -1);
    startTimeJob.resize(jobsCount, 0);

    while (!readyQueue.empty())
    {
        int current = readyQueue.front();
        readyQueue.pop();

        finalTimeJob = computeJobReadyTime(instance, finalTimeOperation, current);
        finalTimeMachine = computeMachineReadyTime(instance, finalTimeOperation, current);

        startTimeOperation = selectBottleneck(finalTimeJob, finalTimeMachine,
                                              instance.predecessorJob[current], instance.predecessorMach[current], criticalPredecessorOut);

        criticalPredecessor[current] = criticalPredecessorOut;

        if (instance.predecessorJob[current] == -1)
            startTimeJob[instance.operationsList[current].id_job] = startTimeOperation;

        double setupTime = computeSetupTime(instance, current);
        double duration = startTimeOperation + instance.processingTime[current] + setupTime;

        finalTimeOperation[current] = duration;

        ValidationOp temp = buildCertificateEntry(instance, current, startTimeOperation, setupTime, duration);
        certificate.push_back(temp);

        topological_list.push_back(current);

        bool isLastOperation = false;

        releaseSuccessorJob(readyQueue, degree, instance, current, isLastOperation);
        releaseSuccessorMachine(readyQueue, degree, instance, current);

        if (isLastOperation)
        {
            currentJob = instance.operationsList[current].id_job;
            jobsCost[currentJob] = duration;
        }

        processed++;
    }

    makespan = 0;

    if (processed != size)
        return jobsCost = buildInfeasibleResult(jobsCount, makespan);

    makespan = findMakespan(jobsCost);

    return jobsCost;
}
std::vector<double> calculator(const InstancePaths instancePaths, double &makespan,
                               std::vector<int> &criticalPredecessor, std::vector<ValidationOp> &certificate,
                               std::vector<double> &startTimeJob)
{
    Instance instance;
    instance.initializeInstance(instancePaths);
    std::vector<double> finalTimeJob;
    int size = instance.jobsList.size();
    finalTimeJob.resize(size, 0);
    finalTimeJob = valuer(instance, makespan, criticalPredecessor, certificate, startTimeJob);

    return finalTimeJob;
}