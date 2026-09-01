#pragma once

#include "movement.hpp"
#include "instance.hpp"
#include <vector>

std::vector<Movement> adjacentNeighborhood(Solution &current);

std::vector<int> generateCriticalPath(std::vector<double> &final_time_job, std::vector<int> &criticalPredecessor, Instance &instance,
                                      double &makespan);

std::vector<Movement> criticalPathNeighborhood(Solution &current, std::vector<int> &criticalPath, Instance &instance);

//// TODO (Etapa 8, exemplo de próxima vizinhança a implementar):
// inserção de uma operação antes/depois de outra na mesma máquina, guiada
// por earliness/tardiness -- inspirado em cands_swap_earl_late do colega,
// mas adaptado para sua representação (solution_matrix), sem precisar da
// lista ligada mach/_mach dele.
// std::vector<Movement> earlinessTardinessNeighborhood(Solution &current, Instance &instance, ...);
