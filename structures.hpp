//
//  structures.h
//  Planning
//
//  Created by Luyao Yuan on 15/10/7.
//  Copyright © 2015年 Luyao Yuan. All rights reserved.
//

#pragma once
/*
 structures.h
 -------------
 Contains the following structs and typedefs:
 Predicate // A predicate, with a type and arguments
 Link // A causal link
 Threat // A threat to a causal link
 thrtCmp // A comparator allowing threat objects to be sorted
 Ordering // An ordering constraint (typedef of std::pair<int,int>)
 Binding // A variable binding (typedef of std::pair<int,int>)
 Action // An action, with type, arguments, add list, delete list, and others
 Plan // An object for representing partially ordered plans
 plan_not_found // an exception class
 */
#include "types.hpp"
#include "variables.hpp"
#include "Map.hpp"
#include <algorithm>
#include <vector>
#include <set>
using namespace std;

// A causal link
// Consists of a predicate, a "causal step", and a "recipient step"
// "causalStep" is an integer referring to a step in a plan which
//		adds predicate "pred" in fulfillment of the preconditions of
//		of the step "recipientStep"
//	In the paper's notation:
//	causalStep -----pred-----> recipientStep
struct Link
{
    Predicate pred;
    int causalStep;
    int recipientStep;
    
    Link() {};
    
    Link(Predicate p, int cstep, int rstep)
    {
        pred = p;
        causalStep = cstep;
        recipientStep = rstep;
    }
};

// A threat to a causal link
// Represents the fact that the step identified by "actionId"
// 	deletes the predicate of the causal link "threatened"
struct Threat
{
    Link threatened;
    int actionId;
    
    Threat(Link thrt, int act)
    {
        threatened = thrt;
        actionId = act;
    }
};

struct threat_cmp
{
    bool operator()(Threat t1, Threat t2)
    {
        return t1.actionId < t2.actionId;
    }
};

// Pairs representing ordering contraints and binding constraints
// The typedef is for ease of understanding
typedef pair<int, int> Ordering;
typedef pair<int, int> Binding;

// An object representing an action
// Actions have at least three arguments, and at most five
struct Action
{
    int args[3]; //arguments
    Actions type; //type of action
    vector<Predicate> addList; //List of added predicates
    vector<Predicate> deleteList; //List of deleted predicates
    
    /*
     Action::adds
     ----
     *** You should implement this function in structures.cpp *****
     
     Takes a Predicate p and a VariableTracker tracker
     Returns a vector of vectors of bindings
     
     Each vector of bindings unifies p with one of the things
     in the add list of the action.
     For example, "start" could add both "in c0 p0" and "in c1 p1"
     If you have a predicate "in x1 x2", then start could add this
     predicate in two different ways.
     The return value would be (({x1,c0},{x2,p0}),({x1,c1},{x2,c2}))
     Where (,) is the vector and {,} is a binding.
     
     If nothing unifies, returns an empty vector of vectors.
     */
    vector<vector<Binding> > adds(const Predicate& p,
                                  const VariableTracker& tracker) const;
    
    // ******* The rest are implemented for you ***********
    
    // Constructor
    Action(Actions t, int arg1, int arg2, int arg3 = -1);
    
    // A utility that fills addList and deleteList based on args
    // Called in constructor, and again during variable substitution
    // Implemented for you.
    void fillPredicates();
    
    // Perform a substitution, substituting all instances of
    // variable "former" with "newval"
    // Implemented for you
    void substitute(int former, int newval);
    
    // Returns a vector of predicates consisting of the prerequisites
    // for this particular action
    vector<Predicate> getPrereqs() const;
    
    // Returns true if there is a variable binding in which this action deletes the predicate
    // Probably should also return the binding itself
    // Calls the unification algorithm
    bool deletes(const Predicate& p) const;
    
    bool operator==(const Action&) const;
    bool reverseEqual(const Action&)const;
    bool prepost(const Action&) const;
};

// A plan object
// We put ordering constraints into an std::set for easy element search
// We put threats into a set for easy insertion and deletion
// Everything else comes in vectors
// You may want to modify this data structure, depending on how your algorithm works
// Just note that if you do so, you may also have to modify the read function
struct Plan
{
    int generation;
    int connection;
    float momentum;
    // Actions are uniquely identified by their index in the steps vector
    vector<Action> steps;
    vector<int> realOrder;
    vector<Link> links; // Causal links
    set<Threat, threat_cmp> threats; // All threats to causal links
    
    // For open conditions, we use a vector of pair<Predicate, int>
    // because we need to store the "parent" action of each open condition,
    // That is, the action that requires this Predicate as a precondition.
    // Our solution is to simply pair predicates with integer identifiers
    vector<pair<Predicate, int> > open;
    
    set<Ordering> orderings; // All ordering constraints
    
    //The integer id of the next variable to be allocated
    //This is important for creating actions "with fresh variables"
    int nextVar;
    int repeat;

    Plan(){
        
    }

    Plan (const Plan& p){
        generation = p.generation;
        connection = p.connection;
        momentum = p.momentum;
        steps = p.steps;
        realOrder = p.realOrder;
        links = p.links;
        threats = p.threats;
        open = p.open;
        orderings = p.orderings;
        nextVar = p.nextVar;
        repeat = p.repeat;
    }
};

struct openCmp
{
    openCmp() {};
    Plan* plan;
    bool operator()(const pair<Predicate, int>& a, const pair<Predicate, int>& b)
    {
        if(a.first.type_ != b.first.type_)
            return a.first.type_ < b.first.type_;
        else if (a.first.type_ == Robot)
        {
            return find(plan->realOrder.begin(), plan->realOrder.end(), a.second) < find(plan->realOrder.begin(), plan->realOrder.end(), b.second);
        }
        else
        {
            return find(plan->realOrder.begin(), plan->realOrder.end(), a.second) > find(plan->realOrder.begin(), plan->realOrder.end(), b.second);
        }
    }
};

struct planCmp
{
    planCmp() {}
    bool operator()(Plan* p1, Plan*  p2) const
    {
        float v1 = p1->repeat + p1->threats.size() + p1->steps.size() + 1.5 * p1->open.size();
        float v2 = 1.5 * p2->open.size() + p2->steps.size() + p2->threats.size() + p1->repeat;
        if(v1 != v2)
            return  v1 > v2;
        return v1 - p1->connection > v2 - p2->connection;
    }
};

struct bindingVectorCmp
{
    vector<float> momentum;
    bool operator()(const vector<Binding>& v1, const vector<Binding>& v2)
    {
        float sum1 = 0;
        float sum2 = 0;
        for(auto binding: v1)
            sum1 += momentum[binding.second];
        for(auto binding: v2)
            sum2 += momentum[binding.second];
        return sum1 / v1.size() > sum2 / v2.size();
    }
};

// An exception class, to be thrown if your search could not find a plan
class plan_not_found {};


