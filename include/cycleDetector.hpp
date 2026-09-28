#pragma once

#include <vector>

class CycleDetector
{
public:
    CycleDetector(unsigned cycleWindow, unsigned cycleRepeats);
    bool detect(double currentCost, bool isNewBest);

private:
    // representa os custos que estão sendo analisados
    std::vector<double> costHistory;

    // guarda o indice do proximo valor a entrar no vetor,
    // é basicamente o index que opera de maneira circular
    unsigned writePointer = 0;

    // guarda a distancia do valor que repetiu neste caso 10, 20, 30, 10
    // o valor 10 está separado por 3 posições
    unsigned suspectedPeriod = 0;

    // guarda a quantidade de vezes que a hipótese se confirmou
    unsigned repeatsSoFar = 0;

    // guarda o tamanho da janela analisada, é o tamanho de costHistory
    unsigned window;

    // Quantidade de confirmações para decretar o ciclo
    unsigned requiredRepeats;
};