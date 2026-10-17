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

void releaseSuccessorJob(std::queue<int> &readyQueue, std::vector<int> &degree, const Instance &instance, int current)
{
    int successor = instance.successorJob[current];
    if (successor == -1)
        return;

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

bool valuer(const Instance &instance)
{
    int size = instance.operationsList.size();
    int processed = 0;

    std::vector<int> degree = computeDegrees(instance, size);
    std::queue<int> readyQueue = buildInitialReadyQueue(degree, size);

    while (!readyQueue.empty())
    {
        int current = readyQueue.front();
        readyQueue.pop();

        releaseSuccessorJob(readyQueue, degree, instance, current);
        releaseSuccessorMachine(readyQueue, degree, instance, current);

        processed++;
    }

    if (processed != size)
        return false;

    return true;
}