#include "checkpointStack.hpp"

CheckpointStack::CheckpointStack(unsigned checkpointStackSize)
{
    stack.resize(checkpointStackSize);
    maxSize = checkpointStackSize;
    base = 0;
    top = 0;
    empty = true;
}

void CheckpointStack::push(SearchCheckpoint &checkpoint)
{
    if (checkpoint.remainingMoves.empty())
        return;

    checkpoint.isEmpty = false;

    if (empty)
    {
        empty = false;
        base = 0;
        top = 0;

        stack[0] = checkpoint;

        return;
    }

    top = (top + 1) % maxSize;

    if (top == base)
        base = (base + 1) % maxSize;

    stack[top] = checkpoint;
}
SearchCheckpoint CheckpointStack::pop()
{
    if (empty)
    {
        SearchCheckpoint checkpoint;
        return checkpoint;
    }

    while (stack[top].remainingMoves.empty())
    {
        if (top == base)
        {
            empty = true;

            SearchCheckpoint checkpoint;
            return checkpoint;
        }
        else
            top = (top + maxSize - 1) % maxSize;
    }

    return stack[top];
}
void CheckpointStack::updateRemainingMoves(const std::vector<Movement> &remaining)
{
    if (empty)
        return;

    stack[top].remainingMoves = remaining;

    while (stack[top].remainingMoves.empty())
    {
        stack[top].isEmpty = true;

        if (base == top)
        {
            empty = true;
            return;
        }
        else
            top = (top + maxSize - 1) % maxSize;
    }
}