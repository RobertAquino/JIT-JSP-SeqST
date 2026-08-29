#pragma once

#include "instance.hpp"
#include <vector>
#include <string>

void startParser(std::string path, std::vector<std::vector<std::vector<int>>> &setup_matrix)
{
    std::ifstream arquivo(path);
    std::string line;

    if (!arquivo.is_open())
    {
        std::cout << "Error opening file: " << std::endl;
        return;
    }

    getline(arquivo, line);

    while (getline(arquivo, line))
    {
        if (line.empty())
            continue;

        std::stringstream ss(line);
        std::string m, j1, j2, t;

        if (getline(ss, m, ',') && getline(ss, j1, ',') &&
            getline(ss, j2, ',') && getline(ss, t))
        {
            int mach = stoi(m);
            int job1 = stoi(j1);
            int job2 = stoi(j2);
            int tempo = stoi(t);

            setup_matrix[mach][job1][job2] = tempo;
        }
    }
    arquivo.close();
}