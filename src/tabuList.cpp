#include "../include/tabuList.hpp"

TabuList::TabuList(int totalOperations)
{
    tabuList.resize(totalOperations, std::vector<int>(totalOperations, 0));
}

void TabuList::insertMovement(int op1, int op2, int currentIteration, int tenure)
{
    tabuList[op1][op2] = currentIteration + tenure;
}

bool TabuList::isTabu(int op1, int op2, int currentIteration) const
{
    if (tabuList[op1][op2] < currentIteration)
        return false;

    return true;
}

int TabuList::age(int op1, int op2, int currentIteration, int tenure) const
{
    return tenure - (tabuList[op1][op2] - currentIteration);
}

int TabuList::timeToLeave(int op1, int op2, int currentIteration) const
{
    return tabuList[op1][op2] - currentIteration;
}
