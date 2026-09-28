#include "../include/tabuSearch.hpp"
#include "../include/instance.hpp"
#include "../include/valuer.hpp"
#include "../include/movement.hpp"
#include "../include/parameters.hpp"
#include "../include/neighborhoodGenerators.hpp"
#include "../include/tabuList.hpp"
#include "../include/cycleDetector.hpp"
#include "../include/checkpointStack.hpp"

#include <limits>

struct SolutionState
{
    Solution solution;

    double makespan = 0;
    double cost = 0;

    bool feasible = false;

    std::vector<double> finalTimeJob;
    std::vector<double> startTimeJob;
    std::vector<int> criticalPredecessor;
    std::vector<ValidationOp> certificate;
};

void changePredecessorMach(const Solution &solution, Instance &instance)
{
    for (const auto &fila : solution.solution_matrix)
    {
        if (fila.empty())
            continue;

        instance.predecessorMach[fila[0]] = -1;

        for (size_t i = 1; i < fila.size(); i++)
        {
            instance.predecessorMach[fila[i]] = fila[i - 1];
        }
    }
}

void changeSuccessorMach(const Solution &solution, Instance &instance)
{
    for (const auto &fila : solution.solution_matrix)
    {
        if (fila.empty())
            continue;

        instance.successorMach[fila.back()] = -1;

        for (size_t i = 0; i < fila.size() - 1; i++)
        {
            instance.successorMach[fila[i]] = fila[i + 1];
        }
    }
}

void changeMachines(const Solution &solution, Instance &instance)
{
    changePredecessorMach(solution, instance);
    changeSuccessorMach(solution, instance);
}
double penaltySum(const std::vector<double> &finalTimeJob, Instance &instance, std::vector<double> &startTimeJob)
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

std::vector<Movement> chosenNeighborhood(int chosenNeighborhood, Solution &current, std::vector<double> &finalTimeJob,
                                         std::vector<int> &criticalPredecessor, Instance &instance, double &makespan)
{
    std::vector<Movement> neighborhood;
    if (chosenNeighborhood == 1)
    {
        neighborhood = adjacentNeighborhood(current);
    }
    if (chosenNeighborhood == 2)
    {
        std::vector<int> criticalPath = generateCriticalPath(finalTimeJob, criticalPredecessor, instance, makespan);
        neighborhood = criticalPathNeighborhood(current, criticalPath, instance);
    }

    return neighborhood;
}

SolutionState evaluateSolution(const Solution &solution, Instance &instance)
{
    SolutionState state;

    state.solution = solution;

    changeMachines(solution, instance);

    state.finalTimeJob = valuer(instance, state.makespan, state.criticalPredecessor, state.certificate,
                                state.startTimeJob);

    if (state.makespan == 999999999.0)
        return state;

    state.feasible = true;
    state.cost = penaltySum(state.finalTimeJob, instance, state.startTimeJob);

    return state;
}

bool chooseBestCandidate(const std::vector<Movement> &neighborhood, TabuList &tabuList, Instance &instance, SolutionState &best, int &tabuIteration,
                         int &chosenIndex, int &chosenOp1, int &chosenOp2, SolutionState &bestCandidate)
{
    double infinity = std::numeric_limits<double>::infinity();
    bestCandidate.cost = infinity;

    bool movementFound = false;
    chosenIndex = -1;
    chosenOp1 = -1;
    chosenOp2 = -1;

    for (size_t i = 0; i < neighborhood.size(); i++)
    {
        Solution candidate = neighborhood[i].solution;
        int op1 = neighborhood[i].op1;
        int op2 = neighborhood[i].op2;

        SolutionState stateCandidate = evaluateSolution(candidate, instance);

        if (!stateCandidate.feasible)
            continue;

        bool isTabu = tabuList.isTabu(op1, op2, tabuIteration);
        bool meetsAspiration = (stateCandidate.cost < best.cost);

        if (isTabu && !meetsAspiration)
            continue;

        if (stateCandidate.cost < bestCandidate.cost)
        {
            bestCandidate = stateCandidate;
            movementFound = true;
            chosenOp1 = op1;
            chosenOp2 = op2;
            chosenIndex = i;
        }
    }

    return movementFound;
}

bool restoreCheckpoint(CheckpointStack &checkpointStack, Instance &instance, SolutionState &current, TabuList &tabuList, int &tabuIteration, std::vector<Movement> &neighborhood)
{
    SearchCheckpoint checkpoint = checkpointStack.pop();

    if (checkpoint.isEmpty)
        return false;

    current = evaluateSolution(checkpoint.solution, instance);
    tabuList = checkpoint.tabuListSnapshot;
    tabuIteration = checkpoint.tabuIterationSnapshot;

    neighborhood = checkpoint.remainingMoves;

    return true;
}

std::vector<Movement> remainingAfterChoice(const std::vector<Movement> &neighborhood, int chosenIndex)
{
    std::vector<Movement> remaining = neighborhood;
    remaining.erase(remaining.begin() + chosenIndex);

    return remaining;
}

void checkpointAfterChoice(CheckpointStack &checkpointStack, const SolutionState &current, const TabuList &tabuList,
                           int tabuIteration, const std::vector<Movement> &remaining)
{
    SearchCheckpoint newCheckpoint;

    newCheckpoint.solution = current.solution;
    newCheckpoint.remainingMoves = remaining;
    newCheckpoint.tabuListSnapshot = tabuList;
    newCheckpoint.tabuIterationSnapshot = tabuIteration;

    checkpointStack.push(newCheckpoint);
    return;
}

void updateBest(const SolutionState &current, SolutionState &best, int &iterationsWithoutImprovement, bool &isNewBest)
{
    if (current.cost < best.cost)
    {
        best = current;
        iterationsWithoutImprovement = 0;
        isNewBest = true;
    }
    else
    {
        iterationsWithoutImprovement++;
        isNewBest = false;
    }
}
double TabuSearch(Instance &instance, Parameters &parameters, std::vector<int> &criticalPredecessor,
                  std::vector<double> &finalTimeJob, int machineCount, std::vector<ValidationOp> &certificate,
                  std::vector<double> &startTimeJob, double &bestMakespan)
{
    Solution initialSolution;

    createSPT(instance, initialSolution, machineCount);

    SolutionState current = evaluateSolution(initialSolution, instance);
    SolutionState best = current;

    int tabuSize = instance.operationsList.size();
    TabuList tabuList(tabuSize);

    CycleDetector cycleDetector(parameters.cycleWindow, parameters.cycleRepeats);
    CheckpointStack checkpointStack(parameters.checkpointStackSize);

    bool isNewBest = true;

    int currentIteration = 0;
    int tabuIteration = 0;
    int iterationsWithoutImprovement = 0;

    while (iterationsWithoutImprovement < parameters.iterationsWithoutImprovement)
    {
        currentIteration++;
        tabuIteration++;

        bool cycling = cycleDetector.detect(current.cost, isNewBest);
        bool jumped;

        std::vector<Movement> neighborhood;

        if (cycling)
        {

            bool restored = restoreCheckpoint(checkpointStack, instance, current, tabuList, tabuIteration, neighborhood);

            if (!restored)
                break;

            jumped = true;
        }
        else
        {
            neighborhood = chosenNeighborhood(parameters.neighborhood, current.solution, current.finalTimeJob,
                                              current.criticalPredecessor, instance, current.makespan);
            jumped = false;
        }

        if (neighborhood.empty())
        {
            bool restored = restoreCheckpoint(checkpointStack, instance, current, tabuList, tabuIteration, neighborhood);

            if (!restored)
                break;

            jumped = true;
        }

        int chosenIndex;
        int chosenOp1;
        int chosenOp2;

        SolutionState bestCandidate;

        bool movementFound = chooseBestCandidate(neighborhood, tabuList, instance, best, tabuIteration, chosenIndex, chosenOp1, chosenOp2, bestCandidate);

        bool terminateSearch = false;

        while (!movementFound)
        {
            if (jumped)
            {
                std::vector<Movement> emptyRemaining;
                checkpointStack.updateRemainingMoves(emptyRemaining);
            }

            bool restored = restoreCheckpoint(checkpointStack, instance, current, tabuList, tabuIteration, neighborhood);

            if (!restored)
            {
                terminateSearch = true;
                break;
            }

            movementFound = chooseBestCandidate(neighborhood, tabuList, instance, best, tabuIteration, chosenIndex, chosenOp1, chosenOp2, bestCandidate);

            jumped = true;
        }

        if (terminateSearch)
            break;

        std::vector<Movement> remaining = remainingAfterChoice(neighborhood, chosenIndex);

        if (isNewBest && !jumped)
        {
            checkpointAfterChoice(checkpointStack, current, tabuList, tabuIteration, remaining);
        }

        if (jumped)
        {
            checkpointStack.updateRemainingMoves(remaining);
        }

        current = bestCandidate;

        changeMachines(current.solution, instance);

        tabuList.insertMovement(chosenOp1, chosenOp2, tabuIteration, parameters.tabuTenure);
        tabuList.insertMovement(chosenOp2, chosenOp1, tabuIteration, parameters.tabuTenure);

        updateBest(current, best, iterationsWithoutImprovement, isNewBest);
    }

    changeMachines(best.solution, instance);

    certificate = best.certificate;
    criticalPredecessor = best.criticalPredecessor;
    startTimeJob = best.startTimeJob;
    finalTimeJob = best.finalTimeJob;
    bestMakespan = best.makespan;

    return best.cost;
}
