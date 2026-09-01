#pragma once

#include "instance.hpp"
#include "valuer.hpp"

// criar uma função que faz a temporização do agendamento

// NOVA função desta etapa: recebe os tempos já calculados pelo valuer
// (por referência, para alterar in-place) e empurra cada operação pro
// mais tarde possível, respeitando:
//   - o predecessor de máquina (não pode terminar depois do sucessor começar)
//   - o predecessor de job (idem)
//   - a deadline/due date do job (não pode empurrar além do necessário
//     para não criar tardiness que não existia antes)
void compactStartTimes(std::vector<double> &startTimeOp,
                       const Instance &instance,
                       const std::vector<Operation> &operationsList,
                       const std::vector<JobInfo> &jobsList);