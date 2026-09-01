#pragma once

#include <string>

enum class NeighborhoodType
{
    ADJACENT,
    CRITICAL_PATH
};
enum class InitialSolutin
{
    SPT
};

class Parameters
{
public:
    int maxIteration = 300;
    int tabuTenure = 10;

    int initialSolution = 1; // SPT
    int neighborhood = 1;    // Adjacent

    long maxMilliSeconds = -1;

    unsigned cycleWindow = 6;  // Number of value observed
    unsigned cycleRepeats = 3; // Number of repeat values

    unsigned checkpointStackSize = 10; // checkpoints
};