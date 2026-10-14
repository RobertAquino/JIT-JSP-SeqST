#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "include/instance.hpp"
#include "include/instancePaths.hpp"
#include "include/parameters.hpp"
#include "include/tabuSearch.hpp"

namespace fs = std::filesystem;

InstancePaths buildInstancePaths(const fs::path &instanceDirectory)
{
    InstancePaths paths;
    paths.operationPath = (instanceDirectory / "operations.csv").string();
    paths.jobPath = (instanceDirectory / "jobs.csv").string();
    paths.setupPath = (instanceDirectory / "setup.csv").string();
    paths.initialSetupPath = (instanceDirectory / "initial_setup.csv").string();
    return paths;
}

bool isInstanceDirectory(const fs::path &directory)
{
    if (!fs::is_directory(directory))
        return false;

    return fs::exists(directory / "operations.csv") &&
           fs::exists(directory / "jobs.csv") &&
           fs::exists(directory / "setup.csv") &&
           fs::exists(directory / "initial_setup.csv");
}

Parameters buildCriticalParameters()
{
    Parameters parameters;

    parameters.neighborhood = 2; // Critical Path
    parameters.tabuTenure = 5;

    parameters.cycleWindow = 6;
    parameters.cycleRepeats = 3;
    parameters.checkpointStackSize = 20;

    parameters.iterationsWithoutImprovement = 555000;
    parameters.maxMilliSeconds = 120000;

    return parameters;
}

double runCritical(
    const fs::path &instanceDirectory,
    double &bestMakespan,
    double &executionTime)
{
    Instance instance;

    InstancePaths paths =
        buildInstancePaths(instanceDirectory);

    instance.initializeInstance(paths);

    int machineCount =
        static_cast<int>(instance.machOperation.size());

    Parameters parameters =
        buildCriticalParameters();

    std::vector<int> criticalPredecessor;
    std::vector<double> finalTimeJob;
    std::vector<double> startTimeJob;
    std::vector<ValidationOp> certificate;

    auto start = std::chrono::steady_clock::now();

    double cost =
        TabuSearch(
            instance,
            parameters,
            criticalPredecessor,
            finalTimeJob,
            machineCount,
            certificate,
            startTimeJob,
            bestMakespan);

    auto end = std::chrono::steady_clock::now();

    executionTime =
        std::chrono::duration<double>(end - start).count();

    return cost;
}

std::vector<fs::path> findInstances(
    const fs::path &dataDirectory)
{
    std::vector<fs::path> instances;

    for (const auto &entry :
         fs::directory_iterator(dataDirectory))
    {
        if (!entry.is_directory())
            continue;

        std::string name =
            entry.path().filename().string();

        if (name.rfind("I-", 0) != 0)
            continue;

        if (!isInstanceDirectory(entry.path()))
            continue;

        instances.push_back(entry.path());
    }

    std::sort(instances.begin(), instances.end());

    return instances;
}

int main(int argc, char *argv[])
{
    try
    {
        // Por padrão usa ./data.
        // Também permite:
        // ./bin/tabu_scheduler data_antiga
        fs::path dataDirectory =
            (argc > 1)
                ? fs::path(argv[1])
                : fs::path("data");

        if (!fs::exists(dataDirectory))
        {
            std::cerr
                << "Diretorio nao encontrado: "
                << dataDirectory
                << '\n';

            return 1;
        }

        std::vector<fs::path> instances =
            findInstances(dataDirectory);

        if (instances.empty())
        {
            std::cerr
                << "Nenhuma instancia I-* encontrada em "
                << dataDirectory
                << '\n';

            return 1;
        }

        std::ofstream output("critical_results_120s.csv");

        if (!output.is_open())
        {
            std::cerr
                << "Nao foi possivel criar critical_results.csv\n";

            return 1;
        }

        // Arquivo propositalmente simples:
        // apenas nome da instancia + custo Critical.
        output << "Instance,Tabu_Critical_Cost\n";
        output.flush();

        std::cout
            << "Encontradas "
            << instances.size()
            << " instancias.\n\n";

        auto totalStart =
            std::chrono::steady_clock::now();

        for (std::size_t i = 0;
             i < instances.size();
             ++i)
        {
            const fs::path &instanceDirectory =
                instances[i];

            std::string instanceName =
                instanceDirectory.filename().string();

            std::cout
                << "["
                << (i + 1)
                << "/"
                << instances.size()
                << "] "
                << instanceName
                << " ... "
                << std::flush;

            try
            {
                double bestMakespan = 0.0;
                double executionTime = 0.0;

                double cost =
                    runCritical(
                        instanceDirectory,
                        bestMakespan,
                        executionTime);

                output
                    << instanceName
                    << ","
                    << std::fixed
                    << std::setprecision(6)
                    << cost
                    << '\n';

                // Salva cada linha imediatamente.
                output.flush();

                std::cout
                    << "cost="
                    << std::fixed
                    << std::setprecision(2)
                    << cost
                    << " | "
                    << std::setprecision(4)
                    << executionTime
                    << " s\n";
            }
            catch (const std::exception &error)
            {
                output
                    << instanceName
                    << ",ERROR\n";

                output.flush();

                std::cerr
                    << "ERROR: "
                    << error.what()
                    << '\n';
            }
        }

        output.close();

        auto totalEnd =
            std::chrono::steady_clock::now();

        double totalTime =
            std::chrono::duration<double>(
                totalEnd - totalStart)
                .count();

        std::cout
            << "\n============================================\n"
            << "TESTE FINALIZADO\n"
            << "============================================\n"
            << "Arquivo: critical_results_120s.csv\n"
            << "Tempo total: "
            << std::fixed
            << std::setprecision(2)
            << totalTime
            << " s\n"
            << "============================================\n";

        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr
            << "Erro fatal: "
            << error.what()
            << '\n';

        return 1;
    }
}
