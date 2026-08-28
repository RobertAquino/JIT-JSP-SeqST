#include "../include/jobParser.hpp"
#include "../include/csvUtils.hpp"

std::vector<JobInfo> parseJobs(const std::string &path)
{
    std::vector<JobInfo> jobs;

    for (const auto &fields : readRows(path))
    {
        JobInfo job;
        job.id_job = std::stoi(fields[0]);
        job.release_date = std::stoi(fields[1]);
        job.due_date = std::stod(fields[2]);
        job.earliness_penalty = std::stod(fields[3]);
        job.tardiness_penalty = std::stod(fields[4]);
        job.flow_time = std::stod(fields[5]);
        jobs.push_back(job);
    }
    return jobs;
}