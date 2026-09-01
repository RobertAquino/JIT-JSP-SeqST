#include "../include/tabuSearch.hpp"
#include "../include/instance.hpp"
#include "../include/valuer.hpp"
#include "../include/Movement.hpp"

void createSPT(Instance &instance, const std::vector<Operation> &operations_list, const std::vector<JobInfo> &jobs_list,
               Solution &solution, int n_mach)
{
    solution.solution_matrix.clear();
    solution.solution_matrix.resize(n_mach);
    int size_op = operations_list.size();
    int size_job = jobs_list.size();
    int processed = 0;
    std::vector<int> job_processing;
    job_processing.resize(size_job, 0);

    while (processed < size_op)
    {
        int best_op = -1;
        int best_job = -1;
        double lower_time = 999999999.0;

        for (int i = 0; i < size_job; i++)
        {
            if (job_processing[i] < (int)instance.jobOperation[i].size())
            {
                int op_index = instance.jobOperation[i][job_processing[i]];
                double op_time = operations_list[op_index].processing_time;

                if (op_time < lower_time)
                {
                    best_job = i;
                    best_op = op_index;
                    lower_time = op_time;
                }
            }
        }

        if (best_op != -1)
        {
            solution.solution_matrix[operations_list[best_op].id_machine].push_back(best_op);
            job_processing[best_job]++;
            processed++;
        }
        else
        {
            break; // Se não achar ninguém, sai do loop pra não travar o PC
        }
    }
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

void changeSucessorMach(const Solution &solution, Instance &instance)
{
    for (const auto &fila : solution.solution_matrix)
    {
        if (fila.empty())
            continue;

        instance.sucessorMach[fila.back()] = -1;

        for (size_t i = 0; i < fila.size() - 1; i++)
        {
            instance.sucessorMach[fila[i]] = fila[i + 1];
        }
    }
}
double penaltySum(const std::vector<double> &final_time, const std::vector<JobInfo> &jobs_list, std::vector<double> &start_time_job)
{
    double sum = 0;
    for (size_t i = 0; i < final_time.size(); i++)
    {
        double due_date = jobs_list[i].due_date;
        double C_last = final_time[i];
        double S_first = start_time_job[i];
        double flow_time = C_last - S_first;

        if (C_last < due_date)
            sum += (due_date - C_last) * jobs_list[i].earliness_penalty;
        else if (C_last > due_date)
            sum += (C_last - due_date) * jobs_list[i].tardiness_penalty;

        sum += flow_time * jobs_list[i].flow_time;
    }
    return sum;
}
double TabuSearch(Instance &instance, double &makespan, std::string setupPath, std::string operationPath, std::string jobPath,
                  std::vector<int> &criticalPredecessor, int optionNeighborhood, int optionImprovementStrategy,
                  std::vector<double> &final_time_job, const std::vector<Operation> &operations_list, std::vector<JobInfo> &jobs_list,
                  int n_mach, int &total_iteration, std::vector<ValidationOp> &certificate, std::vector<double> &start_time_job,
                  std::vector<int> &initialSetup)
{
    std::vector<Movement> neighborhood;
    double temp_makespan = makespan;
    double best_cost;
    std::vector<double> best_global_cost = final_time_job;
    Solution initial_solution;

    total_iteration = 0;
    int max_iteration = 300;
    int iteration = 0;
    int tabu_tenure = (optionNeighborhood == 2) ? 5 : 50;

    int total_ops = operations_list.size();
    std::vector<std::vector<int>> tabu_list(total_ops, std::vector<int>(total_ops, 0));
    createSPT(instance, operations_list, jobs_list, initial_solution, n_mach);

    Solution current = initial_solution;
    Solution best_global = initial_solution;

    changePredecessorMach(initial_solution, instance);
    changeSucessorMach(initial_solution, instance);

    std::vector<double> current_final_time = valuer(operations_list, instance, temp_makespan, criticalPredecessor, jobs_list,
                                                    certificate, start_time_job, initialSetup);
    best_cost = penaltySum(current_final_time, jobs_list, start_time_job);

    std::vector<ValidationOp> best_certificate = certificate;

    while (iteration < max_iteration)
    {
        bool find_movement = false;
        total_iteration++;

        if (optionNeighborhood == 1)
            neighborhood = adjacentNeighborhood(current);

        if (optionNeighborhood == 2)
        {
            std::vector<int> criticalPath = generateCriticalPath(current_final_time, criticalPredecessor, instance, makespan);
            neighborhood = criticalPathNeighborhood(current, criticalPath, instance);
        }

        if (neighborhood.empty())
            break;

        double best_current_makespan = 0;
        double best_cost_current = 99999999999.0;
        Solution best_current = initial_solution;
        int chosen_op1 = -1;
        int chosen_op2 = -1;
        std::vector<double> best_current_final_time;
        std::vector<ValidationOp> best_current_certificate;

        for (size_t i = 0; i < neighborhood.size(); i++)
        {
            Solution next_solution = neighborhood[i].solution;
            double next_makespan;
            int op1 = neighborhood[i].op1;
            int op2 = neighborhood[i].op2;

            changePredecessorMach(next_solution, instance);
            changeSucessorMach(next_solution, instance);
            std::vector<ValidationOp> temp_certificate;
            std::vector<double> next_final_time = valuer(operations_list, instance, next_makespan,
                                                         criticalPredecessor, jobs_list, temp_certificate, start_time_job, initialSetup);

            double next_cost = penaltySum(next_final_time, jobs_list, start_time_job);

            if (next_makespan == 999999999.0)
                continue;

            bool isTabu = (tabu_list[op1][op2] >= total_iteration);
            if (isTabu && next_cost < best_cost)
                isTabu = false;

            if (!isTabu && next_cost < best_cost_current)
            {
                best_cost_current = next_cost;
                best_current_makespan = next_makespan;
                best_current = next_solution;
                chosen_op1 = op1;
                chosen_op2 = op2;
                find_movement = true;
                best_current_final_time = next_final_time;
                best_current_certificate = temp_certificate;
            }
        }

        if (!find_movement)
            break;

        tabu_list[chosen_op1][chosen_op2] = total_iteration + tabu_tenure;
        tabu_list[chosen_op2][chosen_op1] = total_iteration + tabu_tenure;

        current = best_current;
        changePredecessorMach(current, instance);
        changeSucessorMach(current, instance);
        current_final_time = best_current_final_time;

        if (best_cost_current < best_cost)
        {
            temp_makespan = best_current_makespan;
            best_global = best_current;
            best_cost = best_cost_current;
            iteration = 0;
            best_certificate = best_current_certificate;
        }
        else
        {
            iteration++;
        }
    }

    makespan = temp_makespan;
    certificate = best_certificate;

    return best_cost;
}