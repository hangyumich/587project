#include "variables.hpp"
#include <sstream>
#include <iostream>
using namespace std;

VariableTracker::VariableTracker()
:gridEnd(0), groundEnd(0)
{}

// Construct a literal / variable tracker
VariableTracker::VariableTracker(int numGrids)
{
    gridEnd = numGrids;
    groundEnd = gridEnd;
}

// Get the unique integer id associated with name of variable or literal
int VariableTracker::getId(string var) const
{
    istringstream instr(var);
    char ch;
    int num;
    instr >> ch >> num;
    switch(ch){
        case 'g': //grid
            return num;
        case 'x': //variable
            return groundEnd + num;
        default:
            cerr << "Invalid literal\n";
            return -1;
    };
}

// Get the name of the variable or literal associated with id
string VariableTracker::getName(int n) const {
    ostringstream varName;
    if (n < 0) varName << "InvalidVariable";
    else if (n < gridEnd) varName << 'g' << n;
    else varName << 'x' << n - groundEnd;
    return varName.str();
}