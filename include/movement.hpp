#pragma once

enum class move
{
    SWAP
};

struct Solution
{
public:
    std::vector<std::vector<int>> solution_matrix;
};

struct Movement
{
    Solution solution;
    int op1 = -1;
    int op2 = -1;
    int moveType = 1;
};