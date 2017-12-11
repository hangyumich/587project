#ifndef GETPLAN_HPP
#define GETPLAN_HPP

#include "utilize.hpp"
#include "topsort.hpp"
#include "variables.hpp"
#include <queue>
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <vector>

void managerPlanSearch(priority_queue<Plan*, vector<Plan*>, planCmp>& pq, int expected_size);

bool planSearch(Plan& p);



#endif