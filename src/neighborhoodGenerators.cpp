#include "../include/neighborhoodGenerators.hpp"

void createSPT(Instance &instance, Solution &solution, int n_mach)
{
    solution.solution_matrix.clear();
    solution.solution_matrix.resize(n_mach);
    int size_op = instance.operationsList.size();
    int size_job = instance.jobsList.size();
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
                double op_time = instance.operationsList[op_index].processing_time;

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
            solution.solution_matrix[instance.operationsList[best_op].id_machine].push_back(best_op);
            job_processing[best_job]++;
            processed++;
        }
        else
        {
            break; // Se não achar ninguém, sai do loop pra não travar o PC
        }
    }
}

std::vector<Movement> adjacentNeighborhood(Solution &current)
{
    std::vector<Movement> neighbors;
    for (size_t mach = 0; mach < current.solution_matrix.size(); mach++)
    {
        if (current.solution_matrix[mach].size() < 2)
            continue;

        for (size_t i = 0; i < current.solution_matrix[mach].size() - 1; i++)
        {
            Movement mov;
            mov.solution = current;
            mov.op1 = mov.solution.solution_matrix[mach][i];
            mov.op2 = mov.solution.solution_matrix[mach][i + 1];

            std::swap(mov.solution.solution_matrix[mach][i], mov.solution.solution_matrix[mach][i + 1]);

            neighbors.push_back(mov);
        }
    }
    return neighbors;
}

std::vector<int> generateCriticalPath(std::vector<double> &final_time_job, std::vector<int> &criticalPredecessor, Instance &instance,
                                      double &makespan)
{
    std::vector<int> criticalPath;
    int index_last_job = -1;

    for (size_t i = 0; i < final_time_job.size(); i++)
    {
        if (std::abs(final_time_job[i] - makespan) < 1e-9)
        {
            index_last_job = i;
        }
    }

    if (index_last_job == -1)
    {
        for (size_t i = 0; i < final_time_job.size(); i++)
        {
            if (index_last_job == -1 || final_time_job[i] > final_time_job[index_last_job])
                index_last_job = i;
        }
    }

    int current = instance.jobOperation[index_last_job].back();

    while (current != -1)
    {
        criticalPath.push_back(current);
        current = criticalPredecessor[current];
    }

    return criticalPath;
}

std::vector<Movement> criticalPathNeighborhood(Solution &current, std::vector<int> &criticalPath, Instance &instance)
{
    std::vector<Movement> neighbors;

    for (size_t j = 0; j < criticalPath.size() - 1; j++)
    {
        int op1 = criticalPath[j];
        int op2 = criticalPath[j + 1];

        int mach1 = instance.operToMach[op1];
        int mach2 = instance.operToMach[op2];

        if (mach1 == mach2)
        {
            Movement mov;
            mov.op1 = op1;
            mov.op2 = op2;
            mov.solution = current;

            int index_op1 = -1;
            int index_op2 = -1;

            for (size_t i = 0; i < mov.solution.solution_matrix[mach1].size(); i++)
            {
                if (mov.solution.solution_matrix[mach1][i] == op1)
                    index_op1 = i;

                if (mov.solution.solution_matrix[mach1][i] == op2)
                    index_op2 = i;
            }

            if (index_op1 != -1 && index_op2 != -1 && std::abs(index_op1 - index_op2) == 1)
            {
                std::swap(mov.solution.solution_matrix[mach1][index_op1], mov.solution.solution_matrix[mach1][index_op2]);
                neighbors.push_back(mov);
            }
        }
    }
    return neighbors;
}
