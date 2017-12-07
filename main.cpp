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
#include <omp.h>
#include "variables.hpp"
#include <deque>
#include <ctime>
#include "topsort.hpp"
#include <queue>

using namespace std;

void planSearch();
priority_queue<Plan*, vector<Plan*>, planCmp> pq;
//queue<Plan*> pq;
//deque<Plan*> pq;
omp_lock_t queue_l;
omp_lock_t exit_l;

bool whether_exit = false;
Plan* strategy;
int effortLimit = 0;

double push_delay = 0;
double case_delay = 0;
double sort_delay = 0;

int main(int argc, const char * argv[])
{
    //define the map
    Map newM(11,11);
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
    newM.setPosition(9, 9, ROBOT);

    newM.setPosition(7, 1, OBSTACLE);
    newM.setPosition(7, 1, OBSTACLE);
    newM.setPosition(3, 6, OBSTACLE);
    newM.setPosition(3, 5, OBSTACLE);
    newM.setPosition(3, 4, OBSTACLE);
    newM.setPosition(3, 3, OBSTACLE);
     
    /*
    newM.setPosition(8, 1, OBSTACLE);
    newM.setPosition(7, 1, OBSTACLE);
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
    vector<float> momentum = newM.map2Momentum(goals, false);
    Plan* p = new Plan();
    p->connection = 0;
    p->steps.push_back(first);
    p->steps.push_back(last);
    p->realOrder.resize(0);
    p->realOrder.push_back(0);
    p->realOrder.push_back(1);
    for(auto goal: goals)
        p->open.push_back(make_pair(goal, 1));
    p->orderings.insert(Ordering(0,1));
    p->nextVar =121;
    //begin search for solution
    
    try
    {
        omp_init_lock(&queue_l);
        omp_init_lock(&exit_l);


        pq.push(p);
        //pq.push_back(p);

        double delay = omp_get_wtime();

        #pragma omp parallel
        {
            planSearch();
        }
        cout << "The delay is " << omp_get_wtime()-delay << endl;
        cout << "The push_elay is " << push_delay << endl;
        cout << "The case delay is " << case_delay << endl;
        cout << "the sort delay is " << sort_delay << endl;
        
        omp_destroy_lock(&exit_l);
        omp_destroy_lock(&queue_l);


//        printPlan(strategy, vt, cout);
//        for(auto step: strategy.realOrder)
//            cout << step << endl;
        cout << "I'm a smart robot and I find your sokoban problem solution in " << strategy->generation << " times' search\n";
    }
    catch(int notfind)
    {
        cout << notfind << endl;
        cerr << "I'm a stupid robot and I cannot find your Sokoban solution. :(\n";
    }
    
}



void planSearch()
{
    while(true)
    {
        omp_set_lock(&exit_l);
        if(whether_exit){
            omp_unset_lock(&exit_l);
            break;
        }
        omp_unset_lock(&exit_l);

        omp_set_lock(&queue_l);
        Plan* p;
        if(!pq.empty()){
            double t = omp_get_wtime();
            p = pq.top();
            pq.pop();
            push_delay = omp_get_wtime()-t+push_delay;

            //p = pq[0];
            //pq.pop_front();
        }
        else{
            omp_unset_lock(&queue_l);
            continue;
        }
        omp_unset_lock(&queue_l);


       //cout << omp_get_thread_num() << " " << effortLimit << endl;

        double sort_stime = omp_get_wtime();

        if(p->open.empty() && p->threats.empty() && isOrderConsistent(p->orderings, int(p->steps.size())))
        {
            p->realOrder = topSort(p->orderings, int(p->steps.size())).first;
            
            omp_set_lock(&exit_l);
            strategy = p;
            whether_exit = true;
            omp_unset_lock(&exit_l);

            delete p;
            break;
        }

   //     if(effortLimit > MAX_SEARCH_EFFORT) return p;

        effortLimit++;

        if(!isOrderConsistent(p->orderings, int(p->steps.size()))){
            delete p;
            continue;
        }

        p->realOrder = topSort(p->orderings, int(p->steps.size())).first;
        p->connection = 0;
        for(int i = 0; i < p->steps.size() - 1; ++i)
            if(p->steps[p->realOrder[i]].args[1] == p->steps[p->realOrder[i + 1]].args[0])
                p->connection += 1;

        openCmp cp;
        cp.plan = p;
        stable_sort(p->open.begin(), p->open.end(), cp);
        sort_delay += omp_get_wtime() - sort_stime;

        double start_time = omp_get_wtime();
        //case 1 no action adding, only add order based on threats
        if(!p->threats.empty())
        {
            if(p->threats.begin()->threatened.causalStep != 0)
            {
                auto temp = new Plan(*p);
                temp->orderings.insert(make_pair(temp->threats.begin()->actionId, temp->threats.begin()->threatened.causalStep));
                temp->threats.erase(temp->threats.begin());
                temp->generation = effortLimit;

                omp_set_lock(&queue_l);
                double t = omp_get_wtime();
                pq.push(temp);
                push_delay = omp_get_wtime()-t + push_delay;
                //pq.push_back(temp);
                omp_unset_lock(&queue_l);
            }
            if(p->threats.begin()->threatened.recipientStep != 1)
            {
                auto temp = new Plan(*p);
                temp->orderings.insert(make_pair(temp->threats.begin()->threatened.recipientStep, temp->threats.begin()->actionId));
                temp->threats.erase(temp->threats.begin());
                temp->generation = effortLimit;
                
                double t = omp_get_wtime();
                omp_set_lock(&queue_l);
                pq.push(temp);
                //pq.push_back(temp);
                omp_unset_lock(&queue_l);
                push_delay = omp_get_wtime()-t + push_delay;

            }
            delete p;
            case_delay += omp_get_wtime() - start_time;
            continue;
        }//case 1 finish

        //case 2 solve one open condition by unify already existing action
        if(!p->open.empty())
        {
            //case 2.1 use already exist action
            if(p->steps.size() >= 2)
            {
                for(unsigned i = 0; i < p->steps.size(); ++i)
                {

                    VariableTracker vt(121);
                    auto unifyList = p->steps[i].adds(p->open[0].first, vt);

                    if(!unifyList.empty())  
                    {
                        for(auto onePossible: unifyList)
                        {
                            try
                            {
                                auto tempPlan = new Plan(*p);
                                auto tempOpen = p->open[0];
                                tempPlan->links.push_back(Link(tempOpen.first, i, tempOpen.second));
                                tempPlan->open.erase(tempPlan->open.begin());
                                tempPlan->orderings.insert(make_pair(i, tempOpen.second));

                                for(auto bind: onePossible)
                                {
                                    
                                    if(bind.first != bind.second)
                                    {
                                        /*
                                        if(tracker.isLiteral(bind.second))
                                            tempPlan.momentum += momentum[bind.second];*/
                                        for(auto& eachAction: tempPlan->steps)
                                        {
                                            eachAction.substitute(bind.first, bind.second);
                                            if(eachAction.args[0] == eachAction.args[1])
                                            {
                                                throw 1;
                                            }
                                        }
                                        for(auto link: tempPlan->links)
                                            if(tempPlan->steps[link.causalStep].prepost(tempPlan->steps[link.recipientStep]))
                                                throw 3;
                                        for(auto& link: tempPlan->links)
                                        {
                                            link.pred.arg_[0] = (link.pred.arg_[0] == bind.first) ? bind.second : link.pred.arg_[0];
                                            link.pred.arg_[1] = (link.pred.arg_[1] == bind.first) ? bind.second : link.pred.arg_[1];
                                            link.pred.arg_[2] = (link.pred.arg_[2] == bind.first) ? bind.second : link.pred.arg_[2];
                                            
                                        }
                                        for(auto& openP: tempPlan->open)
                                        {
                                            openP.first.arg_[0] = (openP.first.arg_[0] == bind.first) ? bind.second : openP.first.arg_[0];
                                            openP.first.arg_[1] = (openP.first.arg_[1] == bind.first) ? bind.second : openP.first.arg_[1];
                                            openP.first.arg_[2] = (openP.first.arg_[2] == bind.first) ? bind.second : openP.first.arg_[2];
                                        }   
                                    }
                                }

                                //add threats
                                for(unsigned k = 0; k < tempPlan->steps.size(); ++k)
                                {
                                    for(unsigned t = 0; t < tempPlan->links.size(); ++t)
                                    {
                                        auto possible1 = make_pair(int(k), tempPlan->links[t].causalStep);
                                        auto possible2 = make_pair(tempPlan->links[t].recipientStep, int(k));
                                        if(tempPlan->steps[k].deletes(tempPlan->links[t].pred)
                                           && tempPlan->links[t].recipientStep != k
                                           && find(tempPlan->orderings.begin(), tempPlan->orderings.end(), possible1) == tempPlan->orderings.end()
                                           && find(tempPlan->orderings.begin(), tempPlan->orderings.end(), possible2) == tempPlan->orderings.end())
                                            tempPlan->threats.insert(Threat(tempPlan->links[t], k));
                                    }
                                }
                                
                                for(auto threat: tempPlan->threats)
                                {
                                    bool abandon = false;
                                    for(auto order: p->orderings)
                                        if(order.first == threat.actionId && order.second == threat.threatened.recipientStep && threat.threatened.causalStep == 0)
                                        {
                                            abandon = true;
                                            break;
                                        }
                                    if(abandon) throw 9;
                                }
                                
                                tempPlan->repeat = 0;
                                
                                for(auto link: tempPlan->links)
                                {
                                    if(link.causalStep != 0 && link.recipientStep != 1)
                                    {
                                        for(auto l: tempPlan->links)
                                            if(l.causalStep == link.recipientStep && tempPlan->steps[l.recipientStep].reverseEqual(tempPlan->steps[link.causalStep]))
                                            {
                                                throw 5;
                                            }
                                    }
                                }
                                
                                for(unsigned i = 0; i < tempPlan->steps.size(); ++i)
                                {
                                    for(unsigned j = i; j < tempPlan->steps.size(); ++j)
                                    {
                                        if(tempPlan->steps[i].reverseEqual(tempPlan->steps[j]))
                                            tempPlan->repeat++;
                                    }
                                }
                                
                                tempPlan->generation = effortLimit;

                                double t = omp_get_wtime();
                                omp_set_lock(&queue_l);
                                pq.push(tempPlan);
                                //pq.push_back(tempPlan);
                                omp_unset_lock(&queue_l);
                                push_delay = omp_get_wtime()-t + push_delay;
                            }//get rid of not moving action
                            catch(int notMoving)
                            {                                
                                continue;
                            }
                        }//one possible unification
                    }//if can add
                    //only start can possiblly add these predicates
                    if(p->open[0].first.type_ == Linear || p->open[0].first.type_ == Next)
                        break;
                }//for each action see if it can add open[0]
            }//case 2.1 finish

            //case 2.2 add new action
            auto considerOpen = p->open[0];
            auto tempPlan = new Plan(*p);
            auto tempPlan1 = new Plan(*p);
            bool twoActions = false;
            Action newAction(GO, 0, -1);
            Action newAction1(GO, 0, -1);
            switch(considerOpen.first.type_)
            {
                case Robot:
                {
                    newAction = Action(GO, tempPlan->nextVar++, considerOpen.first.arg_[0]);
                    //newAction1 = Action(PUSH, tempPlan1.nextVar++, considerOpen.first.arg_[0], tempPlan1.nextVar++);
                    //twoActions = true;
                    break;
                }
                case Empty:
                {
                    newAction = Action(PUSH, tempPlan->nextVar++, considerOpen.first.arg_[0], tempPlan->nextVar++);
                    //newAction1 = Action(GO, considerOpen.first.arg_[0], tempPlan1.nextVar++); no need this, robot
                    //move shouldn't be a source of Empty predicate
                    //twoActions = true;
                    break;
                }
                case Box:
                {
                    newAction = Action(PUSH, tempPlan->nextVar++, tempPlan->nextVar++, considerOpen.first.arg_[0]);
                    break;
                }
                default:{
                    delete p;
                    continue;
                }
            }
            tempPlan->open.erase(tempPlan->open.begin());
            int numStep = int(tempPlan->steps.size());
            for(auto pre: newAction.getPrereqs())
                tempPlan->open.push_back(make_pair(pre, numStep));
            tempPlan->steps.push_back(newAction);
            for(auto link: tempPlan->links)
            {
                auto possible1 = make_pair(int(tempPlan->steps.size() - 1), link.causalStep);
                auto possible2 = make_pair(link.recipientStep, int(tempPlan->steps.size() - 1));
                if(newAction.deletes(link.pred) && link.recipientStep != tempPlan->steps.size() - 1
                   && find(tempPlan->orderings.begin(), tempPlan->orderings.end(), possible1) == tempPlan->orderings.end()
                   && find(tempPlan->orderings.begin(), tempPlan->orderings.end(), possible2) == tempPlan->orderings.end())
                    tempPlan->threats.insert(Threat(link, int(tempPlan->steps.size() - 1)));
            }
            tempPlan->links.push_back(Link(considerOpen.first, numStep, considerOpen.second));
            for(unsigned k = 0; k < tempPlan->steps.size() - 1; ++k)
            {
                auto possible1 = make_pair(int(k), tempPlan->links.back().causalStep);
                auto possible2 = make_pair(tempPlan->links.back().recipientStep, int(k));
                if(tempPlan->steps[k].deletes(considerOpen.first) && tempPlan->links.back().recipientStep != k
                   && find(tempPlan->orderings.begin(), tempPlan->orderings.end(), possible1) == tempPlan->orderings.end()
                   && find(tempPlan->orderings.begin(), tempPlan->orderings.end(), possible2) == tempPlan->orderings.end())
                    tempPlan->threats.insert(Threat(tempPlan->links.back(), k));
            }
            tempPlan->orderings.insert(make_pair(numStep, considerOpen.second));
            tempPlan->orderings.insert(make_pair(numStep, 1));
            tempPlan->orderings.insert(make_pair(0, numStep));
            tempPlan->generation = effortLimit;
            
            double o = omp_get_wtime();
            omp_set_lock(&queue_l);
            pq.push(tempPlan);
            //pq.push_back(tempPlan);
            omp_unset_lock(&queue_l);
            push_delay = omp_get_wtime() -o + push_delay;

            
            //have two possible actions
            if(twoActions)
            {
                tempPlan1->open.erase(tempPlan1->open.begin());
                    int numStep = int(tempPlan1->steps.size());
                    for(auto pre: newAction1.getPrereqs())
                        tempPlan1->open.push_back(make_pair(pre, numStep));
                    tempPlan1->steps.push_back(newAction1);
                    tempPlan1->orderings.insert(make_pair(numStep, considerOpen.second));
                    tempPlan1->orderings.insert(make_pair(numStep, 1));
                    tempPlan1->orderings.insert(make_pair(0, numStep));
                    for(auto link: tempPlan1->links)
                    {
                        auto possible1 = make_pair(int(tempPlan1->steps.size() - 1), link.causalStep);
                        auto possible2 = make_pair(link.recipientStep, int(tempPlan1->steps.size() - 1));
                        if(newAction1.deletes(link.pred) && link.recipientStep != tempPlan1->steps.size() - 1
                           && find(tempPlan1->orderings.begin(), tempPlan1->orderings.end(), possible1) == tempPlan1->orderings.end()
                           && find(tempPlan1->orderings.begin(), tempPlan1->orderings.end(), possible2) == tempPlan1->orderings.end())
                            tempPlan1->threats.insert(Threat(link, int(tempPlan1->steps.size() - 1)));
                    }
                    tempPlan1->links.push_back(Link(considerOpen.first, numStep, considerOpen.second));
                    for(unsigned k = 0; k < tempPlan1->steps.size() - 1; ++k)
                    {
                        auto possible1 = make_pair(int(k), tempPlan1->links.back().causalStep);
                        auto possible2 = make_pair(tempPlan1->links.back().recipientStep, int(k));
                        if(tempPlan1->steps[k].deletes(considerOpen.first) && tempPlan1->links.back().recipientStep != k
                           && find(tempPlan1->orderings.begin(), tempPlan1->orderings.end(), possible1) == tempPlan1->orderings.end()
                           && find(tempPlan1->orderings.begin(), tempPlan1->orderings.end(), possible2) == tempPlan1->orderings.end())
                            tempPlan1->threats.insert(Threat(tempPlan1->links.back(), k));
                    }
                    tempPlan1->generation = effortLimit;

                    double t = omp_get_wtime();
                    omp_set_lock(&queue_l);
                    pq.push(tempPlan1);
                    //pq.push_back(tempPlan1);
                    omp_unset_lock(&queue_l);
                    push_delay = push_delay + omp_get_wtime() - t;
            }
        }//case 2 finish
        case_delay += omp_get_wtime() - start_time;

        // omp_set_lock(&queue_l);
        // planCmp pCmp;
        // stable_sort(pq.begin(), pq.end(), pCmp);
        // omp_unset_lock(&queue_l);
    }//priority depth first search
}

