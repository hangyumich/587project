/*
 variables.h
 -----------
 Implements a class that keeps track of variable names
 */
#pragma once

#include <string>
using namespace std;

class VariableTracker {
private:
    int gridEnd;
    //int robotEnd;
    //int boxEnd;
    int groundEnd;
    
public:
    // Construct a VariableTracker object
    VariableTracker(int numRobots);
    
    VariableTracker();
    
    // Get the unique integer id of a variable or literal
    int getId(string var) const;
    
    // Get the name of the variable associated with id
    string getName(int id) const ;
    
    // Get the integer value of the first variable
    int getFirstVar() const { return groundEnd; }
    
    // Returns true if id is a valid literal or variable
    bool isValid(int id) const { return (id >= 0); }
    
    // Returns true if v is a variable
    bool isVariable(int v) const { return v >= groundEnd; }
    
    // Returns true if l is a literal
    bool isLiteral(int l) const { return l < groundEnd && l >= 0; }
};
