#pragma once

#include <vector>
#include "movement.hpp"
#include "tabuList.hpp"

// ETAPA 6 do roteiro -- implemente depois de ter as Etapas 2 e 5 funcionando
// e testadas isoladamente. Esta é a peça mais trabalhosa.
//
// SearchCheckpoint é a "foto": tudo que a busca sabia num momento em que
// bateu um novo recorde -- a solução, os candidatos de vizinhança ainda
// não testados a partir dali, e a lista tabu daquele instante.
//
// CheckpointStack é o "álbum": uma pilha de tamanho fixo (mais recente
// primeiro) que guarda essas fotos e sabe devolver a mais recente que
// ainda tem candidatos não explorados.

struct SearchCheckpoint
{
    bool isEmpty = true; // equivalente ao isDummy do colega
    Solution solution;
    std::vector<Movement> remainingMoves;
    TabuList tabuListSnapshot;
};

class CheckpointStack
{
public:
    // empilha um novo checkpoint (chamar quando a busca bate um novo recorde)
    void push(const SearchCheckpoint &checkpoint);

    // devolve o checkpoint mais recente com candidatos ainda não testados;
    // isEmpty=true no retorno significa "não há mais nenhum aproveitável"
    SearchCheckpoint pop();

    // depois de testar um candidato a partir de um checkpoint puxado por
    // pop(), grave de volta a lista reduzida aqui -- para não repetir os
    // mesmos candidatos numa próxima visita a esse mesmo checkpoint
    void updateRemainingMoves(const std::vector<Movement> &remaining);

private:
    std::vector<SearchCheckpoint> stack;
    bool empty = true;
    unsigned maxSize;
    unsigned base = 0;
    unsigned top = 0;
};
