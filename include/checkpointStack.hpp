#pragma once

#include <vector>
#include "movement.hpp"
#include "tabuList.hpp"
#include "parameters.hpp"

struct SearchCheckpoint
{
    bool isEmpty = true; // equivalente ao isDummy do colega
    Solution solution;
    std::vector<Movement> remainingMoves;
    TabuList tabuListSnapshot;
    int tabuIterationSnapshot;
};

class CheckpointStack
{
public:
    CheckpointStack(unsigned checkpointStackSize);

    // empilha um novo checkpoint (chamar quando a busca bate um novo recorde)
    void push(SearchCheckpoint &checkpoint);

    // devolve o checkpoint mais recente com candidatos ainda não testados;
    // isEmpty=true no retorno significa "não há mais nenhum aproveitável"
    SearchCheckpoint pop();

    // depois de testar um candidato a partir de um checkpoint puxado por
    // pop(), grave de volta a lista reduzida aqui -- para não repetir os
    // mesmos candidatos numa próxima visita a esse mesmo checkpoint
    void updateRemainingMoves(const std::vector<Movement> &remaining);

private:
    std::vector<SearchCheckpoint> stack;
    bool empty;
    unsigned maxSize;
    unsigned base;
    unsigned top;
};
