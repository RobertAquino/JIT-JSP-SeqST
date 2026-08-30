#include "../include/operationParser.hpp"
#include "../include/csvUtils.hpp"

std::vector<Operation> parseOperation(const std::string &path)
{
    std::vector<Operation> operationsList;

    std::vector<std::vector<std::string>> rows = readRows(path);

    int id_op = 0;
    for (int i = 0; i < rows.size(); i++)
    {
        const auto &fields = rows[i];
        for (int j = 0; j < fields.size(); j = j + 2)
        {

            Operation operation;
            operation.id_machine = std::stoi(fields[j]);
            operation.processing_time = std::stod(fields[j + 1]);
            operation.id_job = i;
            operation.id_operation = id_op++;
            operationsList.push_back(operation);
        }
    }

    return operationsList;
}