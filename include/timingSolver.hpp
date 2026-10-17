#pragma once
#include "instance.hpp"
#include "movement.hpp"

#include <ilcplex/ilocplex.h>
#include <vector>
ILOSTLBEGIN

struct SolutionState;

struct CostResult
{
    double cost = 0.0;
    bool feasible = false;
    bool optimal = false;
};

struct TimingResult
{
    double cost = 0.0;
    double makespan = 0.0;
    bool feasible = true;
    bool optimal = false;

    std::vector<double> startTimeOp;
    std::vector<double> finalTimeOp;

    std::vector<double> startTimeJob;
    std::vector<double> finalTimeJob;
};

class TimingSolver
{
private:
    IloEnv env;
    IloModel model;

    IloNumVarArray S;
    IloNumVarArray earliness;
    IloNumVarArray tardiness;

    IloExpr objective;
    IloCplex cplex;

    IloConstraintArray machineConstraints;

    std::vector<int> firstOp;
    std::vector<int> lastOp;

    bool error = false;

    void clearMachineConstraints();

    void addMachinePrecedenceConstraints(const Instance &instance);
    void addJobPrecedenceConstraints(IloModel &model, IloNumVarArray &S, const Instance &instance);
    void addEarlinessTardinessConstraints(IloModel &model, IloNumVarArray &S, const Instance &instance,
                                          std::vector<int> &lastOp, IloNumVarArray &earliness, IloNumVarArray &tardiness);
    void buildObjectiveExpression(const Instance &instance, IloNumVarArray &earliness, IloNumVarArray &tardiness,
                                  IloExpr &objective, IloNumVarArray &S, std::vector<int> &lastOp, std::vector<int> &firstOp);

public:
    TimingSolver(const Instance &instance);
    ~TimingSolver();
    TimingResult solveFull(const SolutionState &solutionState, Instance &instance);
    CostResult solveCost(const SolutionState &state, Instance &instance);
};