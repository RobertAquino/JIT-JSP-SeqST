#pragma once

#include <vector>

// ETAPA 5 do roteiro. Implemente e teste esta classe SOZINHA antes de
// conectar no loop da Tabu -- escreva um pequeno teste manual alimentando
// detect() com uma sequência de custos que você sabe que é cíclica
// (ex: 10, 12, 15, 10, 12, 15, 10, 12, 15, ...) e confira se ela retorna
// true na hora certa.
//
// A lógica (inspirada no TabuCycleDetector do colega, mas você pode
// simplificar): guarda uma janela circular dos últimos 'cycleWindow'
// valores de função objetivo. Quando um valor se repete dentro da janela,
// guarda a distância entre as duas ocorrências como "período suspeito de
// ciclo". Se esse período se confirmar 'cycleRepeats' vezes seguidas,
// declara ciclo.

class CycleDetector
{
public:
    CycleDetector(unsigned cycleWindow, unsigned cycleRepeats);
    bool detect(double currentCost, bool isNewBest);

private:
    std::vector<double> costHistory;
    unsigned frontPos = 0;
    unsigned suspectedPeriod = 0;
    unsigned repeatsSoFar = 0;
    unsigned window;
    unsigned requiredRepeats;
};
