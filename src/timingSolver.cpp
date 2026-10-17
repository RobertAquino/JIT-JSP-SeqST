#include "../include/timingSolver.hpp"
#include "../include/tabuSearch.hpp"

#include <limits>
#include <iostream>

TimingSolver::TimingSolver(const Instance &instance)
    : model(env),
      S(env, instance.operationsList.size(), 0.0, IloInfinity, ILOFLOAT),
      earliness(env, instance.jobsList.size(), 0.0, IloInfinity, ILOFLOAT),
      tardiness(env, instance.jobsList.size(), 0.0, IloInfinity, ILOFLOAT),
      objective(env),
      cplex(env),
      machineConstraints(env)
{
    size_t sizeJob = instance.jobsList.size();

    lastOp.resize(sizeJob, -1);
    firstOp.resize(sizeJob, -1);

    for (size_t i = 0; i < sizeJob; i++)
    {
        if (instance.jobOperation[i].empty())
        {
            error = true;
            return;
        }

        firstOp[i] = instance.jobOperation[i].front();

        lastOp[i] = instance.jobOperation[i].back();
    }

    addJobPrecedenceConstraints(model, S, instance);
    addEarlinessTardinessConstraints(model, S, instance, lastOp, earliness, tardiness);
    buildObjectiveExpression(instance, earliness, tardiness, objective, S, lastOp, firstOp);

    model.add(IloMinimize(env, objective));

    cplex.setParam(IloCplex::Param::Threads, 1);
    // cplex.setParam(IloCplex::Param::TimeLimit, 1);

    cplex.setOut(env.getNullStream());
    cplex.setWarning(env.getNullStream());

    cplex.extract(model);
}

TimingSolver::~TimingSolver()
{
    env.end();
}

void TimingSolver::clearMachineConstraints()
{
    if (machineConstraints.getSize() == 0)
        return;

    model.remove(machineConstraints);

    machineConstraints.endElements();

    machineConstraints.clear();
}
void TimingSolver::addJobPrecedenceConstraints(IloModel &model, IloNumVarArray &S, const Instance &instance)
{
    size_t sizeOp = instance.operationsList.size();

    for (size_t i = 0; i < sizeOp; i++)
    {
        if (instance.predecessorJob[i] == -1)
        {
            int indexOp = instance.operationsList[i].id_job;
            model.add(S[i] >= instance.jobsList[indexOp].release_date);
        }
        else
        {
            int predecessor = instance.predecessorJob[i];
            model.add(S[i] >= S[predecessor] + instance.processingTime[predecessor]);
        }
    }
}

void TimingSolver::addMachinePrecedenceConstraints(const Instance &instance)
{
    size_t sizeOp = instance.operationsList.size();

    for (size_t i = 0; i < sizeOp; i++)
    {
        int machine = instance.operationsList[i].id_machine;
        int job = instance.operationsList[i].id_job;

        double st = -1;
        if (instance.predecessorMach[i] == -1)
        {
            int initialJob = instance.initialSetup[machine];

            if (initialJob != -1 && initialJob != job)
            {
                st = instance.setupMatrix[machine][initialJob][job];

                IloConstraint constraint = S[i] >= st;

                machineConstraints.add(constraint);
            }
        }

        if (instance.predecessorMach[i] != -1)
        {
            int predecessorOp = instance.predecessorMach[i];
            int predecessor = instance.operationsList[predecessorOp].id_job;
            st = instance.setupMatrix[machine][predecessor][job];
            IloConstraint constraint = S[i] >= S[predecessorOp] + instance.processingTime[predecessorOp] + st;

            machineConstraints.add(constraint);
        }
    }

    model.add(machineConstraints);
}

TimingResult makeFailedTimingResult()
{
    TimingResult failed;

    failed.feasible = false;
    failed.optimal = false;
    failed.cost = std::numeric_limits<double>::infinity();
    failed.makespan = std::numeric_limits<double>::infinity();

    return failed;
}

CostResult makeFailedCostResult()
{
    CostResult failed;

    failed.feasible = false;
    failed.optimal = false;
    failed.cost = std::numeric_limits<double>::infinity();

    return failed;
}

void TimingSolver::addEarlinessTardinessConstraints(IloModel &model, IloNumVarArray &S, const Instance &instance,
                                                    std::vector<int> &lastOp, IloNumVarArray &earliness, IloNumVarArray &tardiness)
{
    size_t sizeJob = instance.jobsList.size();

    for (size_t i = 0; i < sizeJob; i++)
    {
        int last = lastOp[i];
        double dueDate = instance.jobsList[i].due_date;
        model.add(dueDate == (S[last] + instance.processingTime[last]) + earliness[i] - tardiness[i]);
    }
}

void TimingSolver::buildObjectiveExpression(const Instance &instance, IloNumVarArray &earliness, IloNumVarArray &tardiness,
                                            IloExpr &objective, IloNumVarArray &S, std::vector<int> &lastOp, std::vector<int> &firstOp)
{
    size_t sizeJob = instance.jobsList.size();

    for (size_t i = 0; i < sizeJob; i++)
    {
        double weight_e, weight_t, weight_ft;
        weight_e = instance.jobsList[i].earliness_penalty;
        weight_t = instance.jobsList[i].tardiness_penalty;
        weight_ft = instance.jobsList[i].flow_time;

        int last = lastOp[i];
        int first = firstOp[i];
        double pt = instance.processingTime[last];
        objective += ((weight_e * earliness[i]) + (weight_t * tardiness[i]) + (weight_ft * ((S[last] + pt) - S[first])));
    }
}

void extractTimingResult(IloNumVarArray &S, const Instance &instance, TimingResult &result, IloCplex &cplex,
                         std::vector<int> &lastOp, std::vector<int> &firstOp)
{
    size_t sizeJob = instance.jobsList.size();
    size_t sizeOp = instance.operationsList.size();

    result.startTimeOp.resize(sizeOp, 0);
    result.finalTimeOp.resize(sizeOp, 0);
    result.finalTimeJob.resize(sizeJob, 0);
    result.startTimeJob.resize(sizeJob, 0);

    for (size_t i = 0; i < sizeOp; i++)
    {
        result.startTimeOp[i] = cplex.getValue(S[i]);
        result.finalTimeOp[i] = result.startTimeOp[i] + instance.processingTime[i];
    }

    for (size_t i = 0; i < sizeJob; i++)
    {
        int first = firstOp[i];
        int last = lastOp[i];

        result.startTimeJob[i] = result.startTimeOp[first];

        result.finalTimeJob[i] = result.finalTimeOp[last];
    }

    double makespan = 0.0;

    for (size_t i = 0; i < sizeJob; i++)
    {
        if (result.finalTimeJob[i] > makespan)
            makespan = result.finalTimeJob[i];
    }

    result.makespan = makespan;
    result.cost = cplex.getObjValue();
    result.feasible = true;
}

CostResult TimingSolver::solveCost(
    const SolutionState &state,
    Instance &instance)
{
    if (!state.feasible)
    {
        return makeFailedCostResult();
    }

    if (error)
    {
        return makeFailedCostResult();
    }

    CostResult result;

    try
    {
        clearMachineConstraints();

        addMachinePrecedenceConstraints(
            instance);

        bool solved =
            cplex.solve();

        if (!solved)
        {
            return makeFailedCostResult();
        }

        result.feasible = true;

        result.optimal =
            cplex.getStatus() ==
            IloAlgorithm::Optimal;

        result.cost =
            cplex.getObjValue();
    }
    catch (IloException &e)
    {
        std::cerr
            << "Concert/CPLEX Error: "
            << e
            << std::endl;

        return makeFailedCostResult();
    }
    catch (const std::exception &e)
    {
        std::cerr
            << "C++ Error: "
            << e.what()
            << std::endl;

        return makeFailedCostResult();
    }
    catch (...)
    {
        std::cerr
            << "Erro desconhecido no TimingSolver."
            << std::endl;

        return makeFailedCostResult();
    }

    return result;
}

TimingResult TimingSolver::solveFull(const SolutionState &solutionState, Instance &instance)
{
    if (!solutionState.feasible)
    {
        return makeFailedTimingResult();
    }

    if (error)
    {
        return makeFailedTimingResult();
    }

    TimingResult result;

    try
    {
        clearMachineConstraints();

        addMachinePrecedenceConstraints(instance);

        bool solved = cplex.solve();

        if (!solved)
        {
            result = makeFailedTimingResult();
        }
        else
        {
            IloAlgorithm::Status status = cplex.getStatus();

            if (status == IloAlgorithm::Optimal)
            {
                result.optimal = true;
            }
            else
            {
                result.optimal = false;
                std::cout << "O resultado não é ótimo! Status: "
                          << status << std::endl;
            }
            extractTimingResult(S, instance, result, cplex, lastOp, firstOp);
            // cplex.exportModel("timing.lp");
        }
    }
    catch (IloException &e)
    {
        std::cerr << "Concert/CPLEX Error: " << e << std::endl;
        result = makeFailedTimingResult();
    }
    catch (const std::exception &e)
    {
        std::cerr << "C++ Error: "
                  << e.what() << std::endl;

        result = makeFailedTimingResult();
    }
    catch (...)
    {
        std::cerr << "Erro desconhecido no TimingSolver."
                  << std::endl;

        result = makeFailedTimingResult();
    }

    return result;
}