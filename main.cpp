#include "include/tabuList.hpp"
#include <iostream>
int main()
{
    TabuList tabu(5);

    tabu.insertMovement(0, 1, 0, 5);
    std::cout << tabu.isTabu(0, 1, 4) << std::endl;
    std::cout << tabu.isTabu(0, 1, 10) << std::endl;
    std::cout << tabu.isTabu(0, 2, 4) << std::endl;
    std::cout << tabu.age(0, 1, 2, 5) << std::endl;
    std::cout << tabu.timeToLeave(0, 1, 2) << std::endl;

    return 0;
}