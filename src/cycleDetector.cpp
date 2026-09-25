/*class CycleDetector
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
*/

#include "../include/cycleDetector.hpp"

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

    ++writePointer;
    writePointer = writePointer % window;
    costHistory[writePointer] = currentCost;
}
