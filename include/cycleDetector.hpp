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
    // represent the cost of windows analised
    std::vector<double> costHistory;
    // guarda o indice do proximo valor a entrar no vetor
    unsigned writePointer = 0;
    // guarda a distancia do valor que repetiu
    unsigned suspectedPeriod = 0;
    // guarda a quantidade de vezes que a hipótese se confirmou
    unsigned repeatsSoFar = 0;
    // guarda o tamanho da janela analisada
    unsigned window;
    // Quantidade de confirmações para decretar o ciclo
    unsigned requiredRepeats;
};
/*private:
  // FIFO queue of last maxD selected objetives implemented as circular list
  vector<double> objQ;
  // position of last inserted element on objQ
  unsigned qFrontPos;
  // (qFrontPos+distFrontCycleLast)%maxD is supposed next objective value in
  // possible current exploring cycle
  unsigned distFrontCycleLast;
  // current size of sequence of repeated objective values
  unsigned cycleSize;
  // max size of sequence of repeated objective values
  unsigned maxD;
  // max times a sequence of repeated objective values can repeat
  unsigned maxC;
};*/