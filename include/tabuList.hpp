#pragma once

#include <vector>

class TabuList
{
public:
    TabuList() = default;

    void insertMovement(int op1, int op2, int currentIteration);

    bool isTabu(int op1, int op2, int currentIteration) const;

    int age(int op1, int op2, int currentIteration) const;

    int timeToLeave(int op1, int op2, int currentIteration) const;

private:
    std::vector<std::vector<int>> tabuList;
};