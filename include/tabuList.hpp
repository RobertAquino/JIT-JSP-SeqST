#pragma once

#include <vector>

class TabuList
{
public:
    TabuList() = default;
    explicit TabuList(int totalOperations);

    void insertMovement(int op1, int op2, int currentIteration, int tenure);

    bool isTabu(int op1, int op2, int currentIteration) const;

    int age(int op1, int op2, int currentIteration, int tenure) const;

    int timeToLeave(int op1, int op2, int currentIteration) const;

private:
    std::vector<std::vector<int>> tabuList;
};