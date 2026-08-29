#falta padronizar aqui

#include "../include/setupParser.hpp"
#include "../include/instance.hpp"
#include "../include/csvUtils.hpp"

void startParser(std::string path, std::vector<std::vector<std::vector<int>>> &setup_matrix)
{

    for (const auto &fields : readRows(path))
    {
        int mach = std::stoi(fields[0]);
        int job1 = std::stoi(fields[1]);
        int job2 = std::stoi(fields[2]);
        int tempo = std::stoi(fields[3]);

        setup_matrix[mach][job1][job2] = tempo;
    }
}