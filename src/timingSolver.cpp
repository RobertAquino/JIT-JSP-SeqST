#include "../include/timingSolver.hpp"

#include <ilcplex/ilocplex.h>
#include <limits>
#include <iostream>
ILOSTLBEGIN

bool findFirstAndLastOperations(std::vector<int> &firstOp, std::vector<int> &lastOp, const Instance &instance)
{
    int sizeOp = instance.operationsList.size();
    int sizeJob = instance.jobsList.size();

    for (size_t i = 0; i < sizeJob; i++)
    {
        for (size_t j = 0; j < sizeOp; j++)
        {
            int index = instance.operationsList[j].id_job;

            if (index == i)
            {
                if (instance.predecessorJob[instance.operationsList[j].id_operation] == -1)
                {
                    firstOp[i] = instance.operationsList[j].id_operation;
                }
                if (instance.successorJob[instance.operationsList[j].id_operation] == -1)
                {
                    lastOp[i] = instance.operationsList[j].id_operation;
                }
            }
        }
        if (firstOp[i] == -1 || lastOp[i] == -1)
            return false;
    }
    return true;
}

void addProcessingConstraints(IloModel &model, IloNumVarArray &C, IloNumVarArray &S, const Instance &instance)
{
    int sizeOp = instance.operationsList.size();

    for (size_t i = 0; i < sizeOp; i++)
    {
        double p = instance.processingTime[i];

        model.add(C[i] == S[i] + p);
    }
}

void addJobPrecedenceConstraints(IloModel &model, IloNumVarArray &C, IloNumVarArray &S, const Instance &instance)
{
    int sizeOp = instance.operationsList.size();

    for (size_t i = 0; i < sizeOp; i++)
    {
        if (instance.predecessorJob[i] == -1)
        {
            int indexOp = instance.operationsList[i].id_job;
            model.add(S[i] >= instance.jobsList[indexOp].release_date);
        }
        else
        {
            model.add(S[i] >= C[instance.predecessorJob[i]]);
        }
    }
}

void addMachinePrecedenceConstraints(IloModel &model, IloNumVarArray &C, IloNumVarArray &S, const Instance &instance)
{
    int sizeOp = instance.operationsList.size();
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
                model.add(S[i] >= st);
            }
        }

        if (instance.predecessorMach[i] != -1)
        {
            int predecessorOp = instance.predecessorMach[i];
            int predecessor = instance.operationsList[predecessorOp].id_job;
            st = instance.setupMatrix[machine][predecessor][job];
            model.add(S[i] >= C[instance.predecessorMach[i]] + st);
        }
    }
}

bool addFlowTimeConstraints(IloModel &model, IloNumVarArray &C, IloNumVarArray &S, const Instance &instance,
                            IloNumVarArray &flowTime, std::vector<int> &lastOp, std::vector<int> &firstOp)
{
    int sizeJob = instance.jobsList.size();

    for (size_t i = 0; i < sizeJob; i++)
    {
        if (lastOp[i] != -1 && firstOp[i] != -1)
            model.add(flowTime[i] == C[lastOp[i]] - S[firstOp[i]]);
        else
        {
            return false;
        }
    }
    return true;
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

void addEarlinessTardinessConstraints(IloModel &model, IloNumVarArray &C, const Instance &instance,
                                      std::vector<int> &lastOp, IloNumVarArray &earliness, IloNumVarArray &tardiness)
{
    int sizeJob = instance.jobsList.size();
    for (size_t i = 0; i < sizeJob; i++)
    {
        double dueDate = instance.jobsList[i].due_date;
        model.add(dueDate == C[lastOp[i]] + earliness[i] - tardiness[i]);
    }
}

void buildObjectiveExpression(const Instance &instance, IloNumVarArray &earliness, IloNumVarArray &tardiness,
                              IloNumVarArray &flowTime, IloExpr &objective)
{
    int sizeJob = instance.jobsList.size();

    for (size_t i = 0; i < sizeJob; i++)
    {
        double weight_e, weight_t, weight_ft;
        weight_e = instance.jobsList[i].earliness_penalty;
        weight_t = instance.jobsList[i].tardiness_penalty;
        weight_ft = instance.jobsList[i].flow_time;

        objective += ((weight_e * earliness[i]) + (weight_t * tardiness[i]) + (weight_ft * flowTime[i]));
    }
}

void extractTimingResult(IloNumVarArray &C, IloNumVarArray &S, const Instance &instance,
                         IloNumVarArray &earliness, IloNumVarArray &tardiness, IloNumVarArray &flowTime,
                         TimingResult &result, IloCplex &cplex, std::vector<int> &lastOp,
                         std::vector<int> &firstOp)
{
    int sizeJob = instance.jobsList.size();
    int sizeOp = instance.operationsList.size();

    result.earliness.resize(sizeJob, 0);
    result.tardiness.resize(sizeJob, 0);
    result.flowTime.resize(sizeJob, 0);
    result.startTimeOp.resize(sizeOp, 0);
    result.finalTimeOp.resize(sizeOp, 0);
    result.finalTimeJob.resize(sizeJob, 0);
    result.startTimeJob.resize(sizeJob, 0);

    for (size_t i = 0; i < sizeJob; i++)
    {
        result.earliness[i] = cplex.getValue(earliness[i]);
        result.tardiness[i] = cplex.getValue(tardiness[i]);
        result.flowTime[i] = cplex.getValue(flowTime[i]);
    }

    for (size_t i = 0; i < sizeOp; i++)
    {
        result.startTimeOp[i] = cplex.getValue(S[i]);
        result.finalTimeOp[i] = cplex.getValue(C[i]);
    }

    for (size_t i = 0; i < sizeJob; i++)
    {
        double startTime = cplex.getValue(S[firstOp[i]]);
        double finalTime = cplex.getValue(C[lastOp[i]]);

        result.startTimeJob[i] = startTime;
        result.finalTimeJob[i] = finalTime;
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

TimingResult solveTime(const SolutionState &solutionState, Instance &instance)
{
    if (!solutionState.feasible)
    {
        return makeFailedTimingResult();
    }

    changeMachines(solutionState.solution, instance);

    size_t sizeJob = instance.jobsList.size();

    std::vector<int> lastOp, firstOp;
    lastOp.resize(sizeJob, -1);
    firstOp.resize(sizeJob, -1);

    if (!findFirstAndLastOperations(firstOp, lastOp, instance))
    {

        return makeFailedTimingResult();
    }

    IloEnv env;
    TimingResult result;

    try
    {
        IloModel model(env);

        IloNumVarArray C(env, instance.operationsList.size(), 0.0, IloInfinity, ILOFLOAT);
        IloNumVarArray S(env, instance.operationsList.size(), 0.0, IloInfinity, ILOFLOAT);

        IloNumVarArray earliness(env, instance.jobsList.size(), 0.0, IloInfinity, ILOFLOAT);
        IloNumVarArray tardiness(env, instance.jobsList.size(), 0.0, IloInfinity, ILOFLOAT);
        IloNumVarArray flowTime(env, instance.jobsList.size(), 0.0, IloInfinity, ILOFLOAT);

        IloExpr objective(env);

        addProcessingConstraints(model, C, S, instance);
        addJobPrecedenceConstraints(model, C, S, instance);
        addMachinePrecedenceConstraints(model, C, S, instance);
        addFlowTimeConstraints(model, C, S, instance, flowTime, lastOp, firstOp);
        addEarlinessTardinessConstraints(model, C, instance, lastOp, earliness, tardiness);
        buildObjectiveExpression(instance, earliness, tardiness, flowTime, objective);

        model.add(IloMinimize(env, objective));

        IloCplex cplex(env);

        cplex.setParam(IloCplex::Param::Threads, 1);
        // cplex.setParam(IloCplex::Param::TimeLimit, 1);

        // cplex.setOut(env.getNullStream());
        // cplex.setWarning(env.getNullStream());

        cplex.extract(model);

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
            extractTimingResult(C, S, instance, earliness, tardiness, flowTime, result, cplex, lastOp, firstOp);
            cplex.exportModel("timing.lp");
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

    env.end();

    return result;
}