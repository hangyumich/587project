//
//  main.cpp
//  Sokoban
//
//  Created by Luyao Yuan on 2016/3/20.
//  Copyright © 2016年 Luyao Yuan. All rights reserved.
//

#include <iostream>
#include "Map.hpp"
#include "structures.hpp"
#include "utilize.hpp"
#include "planner.hpp"
#include <chrono>
#include <ctime>
#include <omp.h>

using namespace std;
int main(int argc, const char * argv[])
{
    //define the map
    Map newM(9, 9);
    for(unsigned i = 0; i < newM.getWidth(); ++i)
    {
        for(unsigned j = 0; j < newM.getHeight(); ++j)
        {
            if(i * j == 0 || i == newM.getWidth() - 1 || j == newM.getHeight() - 1)
                newM.setPosition(j, i, OBSTACLE);
        }
    }
    
    newM.setPosition(3, 1, OBSTACLE);
    newM.setPosition(3, 2, OBSTACLE);
    newM.setPosition(4, 2, BOX);
    //newM.setPosition(2, 3, BOX);
    newM.setPosition(7, 7, ROBOT);
     
    
    newM.setPosition(8, 1, OBSTACLE);
    newM.setPosition(7, 1, OBSTACLE);
    //newM.setPosition(3, 6, OBSTACLE);
    newM.setPosition(3, 5, OBSTACLE);
    newM.setPosition(3, 4, OBSTACLE);
    newM.setPosition(3, 3, OBSTACLE);

    /*
    newM.setPosition(6, 1, OBSTACLE);
    newM.setPosition(5, 1, OBSTACLE);
    newM.setPosition(4, 1, OBSTACLE);
    newM.setPosition(4, 2, OBSTACLE);
    newM.setPosition(5, 2, OBSTACLE);
    newM.setPosition(4, 4, OBSTACLE);
    newM.setPosition(3, 4, OBSTACLE);
    newM.setPosition(2, 4, OBSTACLE);
    newM.setPosition(1, 4, OBSTACLE);
    newM.setPosition(4, 5, OBSTACLE);
    newM.setPosition(4, 6, OBSTACLE);
    newM.setPosition(3, 6, OBSTACLE);
    newM.setPosition(2, 6, OBSTACLE);
    newM.setPosition(6, 5, OBSTACLE);
    newM.setPosition(2, 7, OBSTACLE);
    newM.setPosition(7, 5, OBSTACLE);
    newM.setPosition(7, 6, OBSTACLE);
    newM.setPosition(7, 7, OBSTACLE);
    newM.setPosition(7, 8, OBSTACLE);
    newM.setPosition(1, 7, OBSTACLE);
    newM.setPosition(1, 6, OBSTACLE);
    newM.setPosition(1, 5, OBSTACLE);
    newM.setPosition(2, 5, OBSTACLE);
    newM.setPosition(3, 5, OBSTACLE);
    newM.setPosition(2, 2, BOX);
    //newM.setPosition(3, 2, BOX);
    newM.setPosition(2, 3, BOX);
    newM.setPosition(1, 1, ROBOT);
    //newM.setPosition(4, 3, OBSTACLE);
    //newM.setPosition(5, 5, OBSTACLE);
    */
    newM.finishMap();
    cout << newM;
    
    //add initial conditions
    vector<Predicate> initiations = newM.map2Predicates(false);
    vector<Predicate> goals;
    goals.push_back(Predicate(Box, newM.cor2ind(2, 1)));
    //goals.push_back(Predicate(Box, newM.cor2ind(2, 1)));
    
    //goals.push_back(Predicate(Box, newM.cor2ind(2, 2)));
    //goals.push_back(Predicate(Robot, newM.cor2ind(2, 1)));
    //goals.push_back(Predicate(Box, newM.cor2ind(6, 1)));
    //goals.push_back(Predicate(Box, newM.cor2ind(3, 2)));
    //goals.push_back(Predicate(Box, newM.cor2ind(6, 3)));
    //goals.push_back(Predicate(Box, newM.cor2ind(5, 7)));
    //goals.push_back(Predicate(Box, newM.cor2ind(5, 6)));
    Action first(START, -3, -2);
    first.addList = initiations;
    Action last(FINISH, -5, -4);
    
    //initiate everything
    VariableTracker vt(newM.getHeight() * newM.getWidth());
    vector<float> momentum = newM.map2Momentum(goals, false);
    Plan p;
    p.connection = 0;
    p.steps.push_back(first);
    p.steps.push_back(last);
    p.realOrder.resize(0);
    p.realOrder.push_back(0);
    p.realOrder.push_back(1);
    for(auto goal: goals)
        p.open.push_back(make_pair(goal, 1));
    p.orderings.insert(Ordering(0,1));
    p.nextVar = vt.getFirstVar();
    //begin search for solution
    int effortLimit = 0;
    try
    {
        auto start = chrono::system_clock::now();

        double delay = omp_get_wtime();
        Plan strategy;
        planSearch(p, vt, momentum, effortLimit, strategy);

        cout << "The delay is " << omp_get_wtime()-delay << endl;

        //Plan strategy = planSearch(p, vt, effortLimit);


        auto end = chrono::system_clock::now();
        chrono::duration<double> elapsed_seconds = end-start;
        time_t end_time = std::chrono::system_clock::to_time_t(end);
        cout << "elapsed time: " << elapsed_seconds.count() << "s" << endl;

        printPlan(strategy, vt, cout);
        for(auto step: strategy.realOrder)
            cout << step << endl;
        cout << "I'm a smart robot and I find your sokoban problem solution in " << strategy.generation << " times' search\n";
    }
    catch(int notfind)
    {
        cout << notfind << endl;
        cerr << "I'm a stupid robot and I cannot find your Sokoban solution. :(\n";
    }
    
}
