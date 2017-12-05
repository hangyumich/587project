#pragma once

#include "structures.hpp"
#include "variables.hpp"
#include <deque>
#include <ctime>

// Take an initial partial plan, and a variable tracker,
// return a complete plan
void planSearch(Plan& p, VariableTracker& tracker, vector<float>& momentum, int effortLimit, Plan& res);
//Plan planSearch(Plan& p, VariableTracker& tracker, int effortLimit);