//
//  utilize.cpp
//  Sokoban
//
//  Created by Luyao Yuan on 2016/3/22.
//  Copyright © 2016年 Luyao Yuan. All rights reserved.
//

#include "utilize.hpp"

void printPredicate(Predicate& p, VariableTracker& vt, ostream& os)
{
    os << Predicate2Name.at(p.type_) << " ";
    for(int i = 0; i < 3; ++i)
    {
        if(p.arg_[i] == -1)
            break;
        os << vt.getName(p.arg_[i]) << " ";
    }
}
void printAction(Action& a, VariableTracker& vt, ostream& os)
{
    if(a.type == START || a.type == FINISH)
    {
        os << Action2Name.at(a.type);
        return;
    }
    if(a.type == GO)
    {
        os << Action2Name.at(a.type) << " from ";
        os << vt.getName(a.args[0]) << " to ";
        os << vt.getName(a.args[1]);
    }
    else
    {
        os << Action2Name.at(a.type) << " from ";
        os << vt.getName(a.args[0]) << " through " << vt.getName(a.args[1]) << " to ";
        os << vt.getName(a.args[2]);
    }
}

void printPlan(Plan& plan, VariableTracker& m, ostream& out)
{
    out << "$$$$This is a Plan$$$$\n";
    out << "momentum: " << plan.momentum << "\n";
    out << "#Steps\n";
    for (int i = 0; i < plan.steps.size(); ++i)
    {
        Action& act = plan.steps[i];
        out << i << " ";
        printAction(act, m, out);
        out << endl;
    }
    out << "\n#Orderings\n";
    for (auto i = plan.orderings.begin(); i != plan.orderings.end(); ++i)
    {
        out << i->first << " < " << i->second << '\n';
    }
    out << "\n\n#Causal Links\n";
    for (int i = 0; i < plan.links.size(); ++i)
    {
        out << plan.links[i].causalStep << ", ";
        printPredicate(plan.links[i].pred, m, out);
        out << ", " << plan.links[i].recipientStep << '\n';
    }
    out << "\n\n#Threats\n";
    for (auto i = plan.threats.begin(); i != plan.threats.end(); ++i)
    {
        out << i->actionId << ", (";
        out << i->threatened.causalStep << ", ";
        Predicate temp = i->threatened.pred;
        printPredicate(temp, m, out);
        out << ", " << i->threatened.recipientStep;
        out << ")\n";
    }
    out << "\n\n#Open Preconditions\n";
    for (int i = 0; i < plan.open.size(); ++i)
    {
        printPredicate(plan.open[i].first, m, out);
        out << " of " << plan.open[i].second;
        out << '\n';
    }
    out << '\n';
}