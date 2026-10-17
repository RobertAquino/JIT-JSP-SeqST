#include "../include/tabuSearch.hpp"

#include <chrono>
#include <limits>
#include <iostream>

long long evaluationCount = 0;
long long cplexCount = 0;

bool searchTimeExpired(
    const std::chrono::steady_clock::time_point &startTime,
    long maxMilliSeconds)
{
    // -1 significa sem limite de tempo
    if (maxMilliSeconds < 0)
        return false;

    auto now = std::chrono::steady_clock::now();

    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            now - startTime)
            .count();

    return elapsed >= maxMilliSeconds;
}
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

std::vector<Movement> chosenNeighborhood(int chosenNeighborhood, Solution &current, Instance &instance)
{
    std::vector<Movement> neighborhood;
    if (chosenNeighborhood == 1)
    {
        neighborhood = adjacentNeighborhood(current);
    }
    if (chosenNeighborhood == 2)
    {
        changeMachines(current, instance);
        std::vector<int> criticalPath = generateCriticalPath(instance);
        neighborhood = criticalPathNeighborhood(current, criticalPath, instance);
    }

    return neighborhood;
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

SolutionState evaluateASAPSolution(const Solution &solution, Instance &instance)
{
    evaluationCount++;

    SolutionState state;

    state.solution = solution;

    changeMachines(solution, instance);

    ASAPResult asap =
        calculateASAP(instance);

    if (!asap.feasible)
        return state;

    state.feasible = true;

    state.cost = asap.cost;

    return state;
}
SolutionState evaluateSolution(const Solution &solution, Instance &instance, TimingSolver &solver)
{
    SolutionState state;

    state.solution = solution;

    changeMachines(solution, instance);

    bool isFeasible = valuer(instance);

    if (!isFeasible)
        return state;

    state.feasible = true;

    cplexCount++;
    CostResult timing = solver.solveCost(state, instance);

    if (!timing.feasible)
    {
        state.feasible = false;
        return state;
    }

    state.cost = timing.cost;

    state.timingOptimal = timing.optimal;

    return state;
}

bool chooseBestCandidate(const std::vector<Movement> &neighborhood, TabuList &tabuList, Instance &instance,
                         int &tabuIteration, int &chosenIndex, int &chosenOp1, int &chosenOp2,
                         SolutionState &bestCandidate, const std::chrono::steady_clock::time_point &searchStart,
                         long maxMilliSeconds, bool &timeExpired, TimingSolver &solver)
{
    double infinity = std::numeric_limits<double>::infinity();

    double bestASAPCost = infinity;

    bool movementFound = false;

    chosenIndex = -1;
    chosenOp1 = -1;
    chosenOp2 = -1;

    Solution bestASAPSolution;

    for (size_t i = 0; i < neighborhood.size(); i++)
    {
        if (searchTimeExpired(searchStart, maxMilliSeconds))
        {
            timeExpired = true;
            return false;
        }

        const Solution &candidate = neighborhood[i].solution;

        int op1 = neighborhood[i].op1;

        int op2 = neighborhood[i].op2;

        // Primeiro olha tabu.
        bool isTabu = tabuList.isTabu(op1, op2, tabuIteration);

        if (isTabu)
            continue;

        SolutionState stateCandidate = evaluateASAPSolution(candidate, instance);

        if (!stateCandidate.feasible)
            continue;

        if (stateCandidate.cost < bestASAPCost)
        {
            bestASAPCost = stateCandidate.cost;

            bestASAPSolution = candidate;

            chosenIndex = static_cast<int>(i);

            chosenOp1 = op1;
            chosenOp2 = op2;

            movementFound = true;
        }
    }

    if (!movementFound)
        return false;

    if (searchTimeExpired(searchStart, maxMilliSeconds))
    {
        timeExpired = true;
        return false;
    }

    bestCandidate = evaluateSolution(bestASAPSolution, instance, solver);

    if (searchTimeExpired(searchStart, maxMilliSeconds))
    {
        timeExpired = true;
        return false;
    }

    if (!bestCandidate.feasible)
        return false;

    return true;
}

bool restoreCheckpoint(CheckpointStack &checkpointStack, Instance &instance, SolutionState &current, TabuList &tabuList,
                       int &tabuIteration, std::vector<Movement> &neighborhood, TimingSolver &solver)
{
    SearchCheckpoint checkpoint = checkpointStack.pop();

    if (checkpoint.isEmpty)
        return false;

    current = evaluateSolution(checkpoint.solution, instance, solver);
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

std::vector<ValidationOp> buildCertificate(const Instance &instance, const TimingResult &timing)
{

    size_t size = instance.operationsList.size();
    std::vector<ValidationOp> certificate;
    certificate.resize(size);

    for (size_t i = 0; i < size; i++)
    {
        ValidationOp entry;
        int current = instance.operationsList[i].id_operation;

        entry.id_operation = current;
        entry.id_job = instance.operationsList[current].id_job;
        entry.id_machine = instance.operationsList[current].id_machine;
        entry.release_date = instance.jobsList[instance.operationsList[current].id_job].release_date;
        entry.start_time = timing.startTimeOp[current];
        entry.final_time = timing.finalTimeOp[current];
        entry.processing_time = instance.processingTime[current];
        entry.setup_time = computeSetupTime(instance, current);

        certificate[current] = entry;
    }
    return certificate;
}

double TabuSearch(Instance &instance, Parameters &parameters, int machineCount, std::vector<ValidationOp> &certificate,
                  std::vector<double> &startTimeJob, double &bestMakespan, std::vector<double> &finalTimeJob)
{
    evaluationCount = 0;
    cplexCount = 0;

    auto searchStart = std::chrono::steady_clock::now();

    TimingSolver solver(instance);

    Solution initialSolution;

    createSPT(instance, initialSolution, machineCount);

    SolutionState current = evaluateSolution(initialSolution, instance, solver);

    SolutionState best = current;

    int tabuSize = instance.operationsList.size();

    TabuList tabuList(tabuSize);

    CycleDetector cycleDetector(parameters.cycleWindow, parameters.cycleRepeats);

    CheckpointStack checkpointStack(parameters.checkpointStackSize);

    bool isNewBest = true;

    int currentIteration = 0;
    int tabuIteration = 0;

    int iterationsWithoutImprovement = 0;

    bool stopByTime = false;

    while (iterationsWithoutImprovement < parameters.iterationsWithoutImprovement)
    {
        if (searchTimeExpired(searchStart, parameters.maxMilliSeconds))
        {
            break;
        }

        currentIteration++;
        tabuIteration++;

        bool cycling = cycleDetector.detect(current.cost, isNewBest);

        bool jumped;

        std::vector<Movement> neighborhood;

        if (cycling)
        {
            bool restored = restoreCheckpoint(checkpointStack, instance, current, tabuList, tabuIteration, neighborhood, solver);

            if (!restored)
                break;

            jumped = true;
        }
        else
        {
            neighborhood = chosenNeighborhood(parameters.neighborhood, current.solution, instance);

            jumped = false;
        }

        if (neighborhood.empty())
        {
            bool restored = restoreCheckpoint(checkpointStack, instance, current, tabuList, tabuIteration, neighborhood, solver);

            if (!restored)
                break;

            jumped = true;
        }

        int chosenIndex;
        int chosenOp1;
        int chosenOp2;

        SolutionState bestCandidate;

        bool timeExpired = false;

        bool movementFound = chooseBestCandidate(neighborhood, tabuList, instance, tabuIteration, chosenIndex, chosenOp1,
                                                 chosenOp2, bestCandidate, searchStart, parameters.maxMilliSeconds, timeExpired,
                                                 solver);

        if (timeExpired)
        {
            stopByTime = true;
            break;
        }

        bool terminateSearch = false;

        while (!movementFound)
        {
            // Também precisamos verificar o
            // tempo dentro deste segundo loop.
            if (searchTimeExpired(searchStart, parameters.maxMilliSeconds))
            {
                stopByTime = true;
                break;
            }

            if (jumped)
            {
                std::vector<Movement> emptyRemaining;

                checkpointStack.updateRemainingMoves(emptyRemaining);
            }

            bool restored = restoreCheckpoint(checkpointStack, instance, current, tabuList, tabuIteration, neighborhood,
                                              solver);

            if (!restored)
            {
                terminateSearch = true;
                break;
            }

            timeExpired = false;

            movementFound = chooseBestCandidate(neighborhood, tabuList, instance, tabuIteration, chosenIndex,
                                                chosenOp1, chosenOp2, bestCandidate, searchStart,
                                                parameters.maxMilliSeconds, timeExpired, solver);

            if (timeExpired)
            {
                stopByTime = true;
                break;
            }

            jumped = true;
        }

        if (stopByTime)
            break;

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

    // Mesmo que tenha parado pelo tempo,
    // devolvemos o MELHOR encontrado até então.

    changeMachines(best.solution, instance);

    cplexCount++;

    TimingResult finalTiming = solver.solveFull(best, instance);

    if (!finalTiming.feasible)
    {
        std::cerr << "Erro ao temporizar a melhor solucao final." << std::endl;

        return std::numeric_limits<double>::infinity();
    }

    startTimeJob = finalTiming.startTimeJob;

    finalTimeJob = finalTiming.finalTimeJob;

    bestMakespan = finalTiming.makespan;

    std::cout << "Evaluations: " << evaluationCount << std::endl;
    std::cout << "CPLEX Calls: " << cplexCount << std::endl;

    certificate = buildCertificate(instance, finalTiming);

    return best.cost;
}
