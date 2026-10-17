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

// ============================================================
// Tempo do experimento
// ============================================================
//
// Todas as instancias -> 20 segundos
//

constexpr long TIME_LIMIT_MS = 20'000;

// ============================================================
// Caminhos da instância
// ============================================================

InstancePaths buildInstancePaths(
    const fs::path &instanceDirectory)
{
    InstancePaths paths;

    paths.operationPath =
        (instanceDirectory / "operations.csv").string();

    paths.jobPath =
        (instanceDirectory / "jobs.csv").string();

    paths.setupPath =
        (instanceDirectory / "setup.csv").string();

    paths.initialSetupPath =
        (instanceDirectory / "initial_setup.csv").string();

    return paths;
}

// ============================================================
// Verifica se uma pasta realmente contém uma instância
// ============================================================

bool isInstanceDirectory(
    const fs::path &directory)
{
    if (!fs::is_directory(directory))
        return false;

    return fs::exists(directory / "operations.csv") &&
           fs::exists(directory / "jobs.csv") &&
           fs::exists(directory / "setup.csv") &&
           fs::exists(directory / "initial_setup.csv");
}

// ============================================================
// Limite de tempo
// ============================================================

long getTimeLimit(
    const std::string &instanceName)
{
    (void)instanceName;

    return TIME_LIMIT_MS;
}

// ============================================================
// Parâmetros da Adjacent híbrida
// ============================================================

Parameters buildAdjacentParameters(
    long timeLimitMs)
{
    Parameters parameters;

    // =====================================
    // Vizinhança Adjacent
    // =====================================

    parameters.neighborhood = 1;

    // Mantemos 5 para a comparação ser
    // diretamente comparável com a Critical híbrida.
    parameters.tabuTenure = 5;

    parameters.cycleWindow = 6;
    parameters.cycleRepeats = 3;

    parameters.checkpointStackSize = 20;

    parameters.iterationsWithoutImprovement =
        555000;

    parameters.maxMilliSeconds =
        timeLimitMs;

    return parameters;
}

// ============================================================
// Executa UMA instância
// ============================================================

double runAdjacent(
    const fs::path &instanceDirectory,
    double &bestMakespan,
    double &executionTime,
    long timeLimitMs)
{
    Instance instance;

    InstancePaths paths =
        buildInstancePaths(
            instanceDirectory);

    instance.initializeInstance(paths);

    int machineCount =
        static_cast<int>(
            instance.machOperation.size());

    Parameters parameters =
        buildAdjacentParameters(
            timeLimitMs);

    std::vector<double> finalTimeJob;
    std::vector<double> startTimeJob;

    std::vector<ValidationOp> certificate;

    auto start =
        std::chrono::steady_clock::now();

    double cost =
        TabuSearch(
            instance,
            parameters,
            machineCount,
            certificate,
            startTimeJob,
            bestMakespan,
            finalTimeJob);

    auto end =
        std::chrono::steady_clock::now();

    executionTime =
        std::chrono::duration<double>(
            end - start)
            .count();

    return cost;
}

// ============================================================
// Busca todas as 80 instâncias
// ============================================================

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
            entry.path()
                .filename()
                .string();

        if (name.rfind("I-", 0) != 0)
            continue;

        if (!isInstanceDirectory(
                entry.path()))
        {
            continue;
        }

        instances.push_back(
            entry.path());
    }

    std::sort(
        instances.begin(),
        instances.end());

    return instances;
}

// ============================================================
// MAIN
// ============================================================

int main(
    int argc,
    char *argv[])
{
    try
    {
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
            findInstances(
                dataDirectory);

        if (instances.empty())
        {
            std::cerr
                << "Nenhuma instancia I-* encontrada em "
                << dataDirectory
                << '\n';

            return 1;
        }

        // ====================================================
        // CSV
        // ====================================================

        std::ofstream output(
            "adjacent_hybrid_20s_results.csv");

        if (!output.is_open())
        {
            std::cerr
                << "Nao foi possivel criar "
                   "adjacent_hybrid_20s_results.csv\n";

            return 1;
        }

        output
            << "Instance,"
            << "Adjacent_Hybrid_Cost,"
            << "Best_Makespan,"
            << "Time_Limit_Seconds,"
            << "Actual_Time_Seconds\n";

        output.flush();

        // ====================================================
        // Informações do experimento
        // ====================================================

        std::cout
            << "============================================\n"
            << "TESTE ADJACENT HIBRIDA - 20s\n"
            << "ASAP EM TODOS + CPLEX NO ESCOLHIDO\n"
            << "============================================\n"
            << "Instancias: "
            << instances.size()
            << "\n\n"
            << "Tempo por instancia: 20 s\n\n"
            << "Neighborhood: Adjacent (1)\n"
            << "Tabu tenure: 5\n"
            << "Cycle window: 6\n"
            << "Cycle repeats: 3\n"
            << "Checkpoint stack: 20\n"
            << "============================================\n\n";

        auto totalStart =
            std::chrono::steady_clock::now();

        // ====================================================
        // Executa todas as instâncias
        // ====================================================

        for (std::size_t i = 0;
             i < instances.size();
             ++i)
        {
            const fs::path &instanceDirectory =
                instances[i];

            std::string instanceName =
                instanceDirectory
                    .filename()
                    .string();

            long timeLimitMs =
                getTimeLimit(
                    instanceName);

            double timeLimitSeconds =
                timeLimitMs / 1000.0;

            std::cout
                << "["
                << (i + 1)
                << "/"
                << instances.size()
                << "] "
                << instanceName
                << " | limit="
                << timeLimitSeconds
                << " s ... "
                << std::flush;

            try
            {
                double bestMakespan =
                    0.0;

                double executionTime =
                    0.0;

                double cost =
                    runAdjacent(
                        instanceDirectory,
                        bestMakespan,
                        executionTime,
                        timeLimitMs);

                // ============================================
                // CSV
                // ============================================

                output
                    << instanceName
                    << ","
                    << std::fixed
                    << std::setprecision(6)
                    << cost
                    << ","
                    << bestMakespan
                    << ","
                    << timeLimitSeconds
                    << ","
                    << executionTime
                    << '\n';

                output.flush();

                // ============================================
                // Terminal
                // ============================================

                std::cout
                    << "cost="
                    << std::fixed
                    << std::setprecision(2)
                    << cost
                    << " | makespan="
                    << bestMakespan
                    << " | time="
                    << std::setprecision(4)
                    << executionTime
                    << " s\n";
            }
            catch (const std::exception &error)
            {
                output
                    << instanceName
                    << ",ERROR,ERROR,"
                    << timeLimitSeconds
                    << ",ERROR\n";

                output.flush();

                std::cerr
                    << "\nERROR em "
                    << instanceName
                    << ": "
                    << error.what()
                    << '\n';
            }
            catch (...)
            {
                output
                    << instanceName
                    << ",ERROR,ERROR,"
                    << timeLimitSeconds
                    << ",ERROR\n";

                output.flush();

                std::cerr
                    << "\nERRO desconhecido em "
                    << instanceName
                    << '\n';
            }
        }

        output.close();

        // ====================================================
        // Tempo total
        // ====================================================

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
            << "Arquivo: adjacent_hybrid_20s_results.csv\n"
            << "Tempo total: "
            << std::fixed
            << std::setprecision(2)
            << totalTime
            << " s\n"
            << "Tempo total: "
            << totalTime / 3600.0
            << " h\n"
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
    catch (...)
    {
        std::cerr
            << "Erro fatal desconhecido.\n";

        return 1;
    }
}