#pragma once

#include "csvUtils.hpp"
#include "instance.hpp"
#include <string>
#include <vector>

std::vector<JobInfo> parseJobs(const std::string &path);