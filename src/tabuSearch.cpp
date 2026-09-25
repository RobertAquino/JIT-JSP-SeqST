#include "../include/tabuSearch.hpp"
#include "../include/instance.hpp"
#include "../include/valuer.hpp"
#include "../include/movement.hpp"
#include "../include/parameters.hpp"
#include "../include/neighborhoodGenerators.hpp"
#include "../include/tabuList.hpp"

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

    int currentIteration = 0;
    int iterationsWithoutImprovement = 0;

    while (iterationsWithoutImprovement < parameters.iterationsWithoutImprovement)
    {
        currentIteration++;

        std::vector<Movement> neighborhood = chosenNeighborhood(parameters.neighborhood, current.solution, current.finalTimeJob,
                                                                current.criticalPredecessor, instance, current.makespan);

        if (neighborhood.empty())
            break;

        double infinity = std::numeric_limits<double>::infinity();
        SolutionState bestCandidate;
        bestCandidate.cost = infinity;

        bool movementFound = false;
        int chosenOp1 = -1;
        int chosenOp2 = -1;

        for (size_t i = 0; i < neighborhood.size(); i++)
        {
            Solution candidate = neighborhood[i].solution;
            int op1 = neighborhood[i].op1;
            int op2 = neighborhood[i].op2;

            SolutionState stateCandidate = evaluateSolution(candidate, instance);

            if (!stateCandidate.feasible)
                continue;

            bool isTabu = tabuList.isTabu(op1, op2, currentIteration);
            bool meetsAspiration = (stateCandidate.cost < best.cost);

            if (isTabu && !meetsAspiration)
                continue;

            if (stateCandidate.cost < bestCandidate.cost)
            {
                bestCandidate = stateCandidate;
                movementFound = true;
                chosenOp1 = op1;
                chosenOp2 = op2;
            }
        }

        if (!movementFound)
            break;

        current = bestCandidate;

        changeMachines(current.solution, instance);

        tabuList.insertMovement(chosenOp1, chosenOp2, currentIteration, parameters.tabuTenure);
        tabuList.insertMovement(chosenOp2, chosenOp1, currentIteration, parameters.tabuTenure);

        if (current.cost < best.cost)
        {
            best = current;
            iterationsWithoutImprovement = 0;
        }
        else
        {
            iterationsWithoutImprovement++;
        }
    }

    changeMachines(best.solution, instance);

    certificate = best.certificate;
    criticalPredecessor = best.criticalPredecessor;
    startTimeJob = best.startTimeJob;
    finalTimeJob = best.finalTimeJob;
    bestMakespan = best.makespan;

    return best.cost;
}
