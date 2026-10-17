#include "../include/ASAPResult.hpp"

#include <queue>

namespace
{
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

    double computeJobReadyTime(const Instance &instance, std::vector<double> &finalTimeOperation, int current)
    {
        int predecessorJob = instance.predecessorJob[current];

        if (predecessorJob == -1)
            return instance.jobsList[instance.operationsList[current].id_job].release_date;

        return finalTimeOperation[predecessorJob];
    }

    double computeMachineReadyTime(const Instance &instance, std::vector<double> &finalTimeOperation, int current,
                                   double setupTime)
    {
        int predecessorMachine = instance.predecessorMach[current];

        if (predecessorMachine == -1)
            return setupTime;

        return finalTimeOperation[predecessorMachine] + setupTime;
    }

    double selectBottleneck(double timeJobFinal, double timeMachineFinal, int predecessorJob, int predecessorMach,
                            int &criticalPredecessorOut)
    {
        if (timeJobFinal > timeMachineFinal)
        {
            criticalPredecessorOut = predecessorJob;

            return timeJobFinal;
        }

        if (timeMachineFinal > timeJobFinal)
        {
            criticalPredecessorOut = predecessorMach;

            return timeMachineFinal;
        }

        if (predecessorMach != -1)
        {
            criticalPredecessorOut = predecessorMach;
        }
        else
        {
            criticalPredecessorOut = predecessorJob;
        }

        return timeJobFinal;
    }
    void releaseSuccessorJob(std::queue<int> &readyQueue, std::vector<int> &degree, const Instance &instance,
                             int current, bool &isLastOperation)
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
}
double penaltySum(const std::vector<double> &finalTimeJob, const Instance &instance,
                  const std::vector<double> &startTimeJob)
{
    double sum = 0;
    for (size_t i = 0; i < finalTimeJob.size(); i++)
    {
        double due_date = instance.jobsList[i].due_date;
        double C_last = finalTimeJob[i];
        double S_first = startTimeJob[i];
        double flow_time = C_last - S_first;

        if (C_last < due_date)
            sum += (due_date - C_last) * instance.jobsList[i].earliness_penalty;
        else if (C_last > due_date)
            sum += (C_last - due_date) * instance.jobsList[i].tardiness_penalty;

        sum += flow_time * instance.jobsList[i].flow_time;
    }
    return sum;
}
ASAPResult calculateASAP(const Instance &instance)
{
    ASAPResult result;

    double startTimeOperation;
    double finalTimeJob;
    double finalTimeMachine;

    int size = instance.operationsList.size();
    int jobsCount = instance.jobsList.size();
    int processed = 0;
    int currentJob;

    std::vector<double> startTimeJob(jobsCount, 0.0);

    result.finalTimeJob.resize(jobsCount, 0.0);

    std::vector<int> degree = computeDegrees(instance, size);

    std::queue<int> readyQueue = buildInitialReadyQueue(degree, size);

    result.finalTimeOperation.resize(size, 0.0);
    result.criticalPredecessor.resize(size, -1);

    while (!readyQueue.empty())
    {
        int current = readyQueue.front();
        readyQueue.pop();

        double setupTime = computeSetupTime(instance, current);

        int criticalPredecessorOut = -1;

        finalTimeJob = computeJobReadyTime(instance, result.finalTimeOperation, current);

        finalTimeMachine = computeMachineReadyTime(instance, result.finalTimeOperation, current, setupTime);

        startTimeOperation = selectBottleneck(finalTimeJob, finalTimeMachine, instance.predecessorJob[current],
                                              instance.predecessorMach[current], criticalPredecessorOut);

        result.criticalPredecessor[current] = criticalPredecessorOut;

        currentJob = instance.operationsList[current].id_job;

        if (instance.predecessorJob[current] == -1)
        {
            startTimeJob[currentJob] = startTimeOperation;
        }

        double finalTimeOperation = startTimeOperation + instance.processingTime[current];

        result.finalTimeOperation[current] = finalTimeOperation;

        bool isLastOperation = false;

        releaseSuccessorJob(readyQueue, degree, instance, current, isLastOperation);

        releaseSuccessorMachine(readyQueue, degree, instance, current);

        if (isLastOperation)
        {
            result.finalTimeJob[currentJob] = finalTimeOperation;
        }

        processed++;
    }

       if (processed != size)
    {
        result.feasible = false;
        return result;
    }

    result.feasible = true;

    result.makespan = findMakespan(result.finalTimeJob);

    result.cost = penaltySum(result.finalTimeJob, instance, startTimeJob);

    return result;
}