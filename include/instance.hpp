#pragma once

class JobInfo
{
public:
    int id_job;
    int release_date;
    double due_date;
    double earliness_penalty;
    double tardiness_penalty;
    double flow_time;
};

class Operation
{
public:
    int id_operation;
    int id_job;
    int id_machine;
    double processing_time;
    double setup_time;
};
