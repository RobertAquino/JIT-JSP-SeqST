#include "../include/cycleDetector.hpp"

#include <cmath>

CycleDetector::CycleDetector(unsigned cycleWindow, unsigned cycleRepeats)
{
    costHistory.resize(cycleWindow, -1);
    window = cycleWindow;
    requiredRepeats = cycleRepeats;
}
//(ex: 10, 12, 15, 10, 12, 15, 10, 12, 15, ...) d
bool CycleDetector::detect(double currentCost, bool isNewBest)
{
    if (isNewBest)
    {
        writePointer = 0;
        suspectedPeriod = 0;
        repeatsSoFar = 0;
        for (size_t i = 0; i < costHistory.size(); i++)
            costHistory[i] = -1;
    }

    double tolerance = 0.10;
    if (suspectedPeriod != 0)
    {
        size_t index = (writePointer + window - suspectedPeriod) % window;
        if (std::abs(currentCost - costHistory[index]) <= tolerance)
        {
            repeatsSoFar++;

            if (repeatsSoFar >= requiredRepeats)
            {
                costHistory[writePointer] = currentCost;
                ++writePointer;
                writePointer = writePointer % window;
                return true;
            }
        }
        else
        {
            suspectedPeriod = 0;
            repeatsSoFar = 0;

            unsigned int count = 1;

            while (count < window)
            {
                int index = (writePointer + window - count) % window;
                if (std::abs(currentCost - costHistory[index]) <= tolerance)
                    suspectedPeriod = count;
                count++;
            }
        }
    }
    else
    {
        unsigned int count = 1;

        while (count < window)
        {
            int index = (writePointer + window - count) % window;
            if (std::abs(currentCost - costHistory[index]) <= tolerance)
                suspectedPeriod = count;
            count++;
        }
    }

    costHistory[writePointer] = currentCost;
    ++writePointer;
    writePointer = writePointer % window;

    return false;
}
