#include "structures.hpp"

//at most return one pair, because our predicates only take one argument
vector<Binding> Unify(const Predicate& p1, const Predicate& p2, const VariableTracker& vt)
{
    vector<Binding> theta(0);
    if(p1.type_ != p2.type_) return theta;
    if(p1.type_ != Next && p1.type_ != Linear)
    {
        //first literal is different
        if(vt.isLiteral(p1.arg_[0])
           && vt.isLiteral(p2.arg_[0])
           && p1.arg_[0] != p2.arg_[0])
            return theta;
        else
        {
            Binding temp = make_pair(max(p1.arg_[0], p2.arg_[0]), min(p1.arg_[0], p2.arg_[0]));
            theta.push_back(temp);
            return theta;
        }
    }
    else if(p1.type_ == Next)
    {
        //bind in first order
        if(!(vt.isLiteral(p1.arg_[0]) && vt.isLiteral(p2.arg_[0]) && p1.arg_[0] != p2.arg_[0])
           && !(vt.isLiteral(p1.arg_[1]) && vt.isLiteral(p2.arg_[1]) && p1.arg_[1] != p2.arg_[1]))
        {
            Binding temp1 = make_pair(max(p1.arg_[0], p2.arg_[0]), min(p1.arg_[0], p2.arg_[0]));
            Binding temp2 = make_pair(max(p1.arg_[1], p2.arg_[1]), min(p1.arg_[1], p2.arg_[1]));
            theta.push_back(temp1);
            theta.push_back(temp2);
        }
        //bind in second order
        if(!(vt.isLiteral(p1.arg_[0]) && vt.isLiteral(p2.arg_[1]) && p1.arg_[0] != p2.arg_[1])
           && !(vt.isLiteral(p1.arg_[1]) && vt.isLiteral(p2.arg_[0]) && p1.arg_[1] != p2.arg_[0]))
        {
            Binding temp1 = make_pair(max(p1.arg_[0], p2.arg_[1]), min(p1.arg_[0], p2.arg_[1]));
            Binding temp2 = make_pair(max(p1.arg_[1], p2.arg_[0]), min(p1.arg_[1], p2.arg_[0]));
            theta.push_back(temp1);
            theta.push_back(temp2);
        }
        return theta;
    }
    else//linear
    {
        if(vt.isLiteral(p1.arg_[1]) && vt.isLiteral(p2.arg_[1]) && p1.arg_[1] != p2.arg_[1])
            return theta;
        //bind in first order
        if(!(vt.isLiteral(p1.arg_[0]) && vt.isLiteral(p2.arg_[0]) && p1.arg_[0] != p2.arg_[0])
           && !(vt.isLiteral(p1.arg_[2]) && vt.isLiteral(p2.arg_[2]) && p1.arg_[2] != p2.arg_[2]))
        {
            Binding temp1 = make_pair(max(p1.arg_[0], p2.arg_[0]), min(p1.arg_[0], p2.arg_[0]));
            Binding temp2 = make_pair(max(p1.arg_[2], p2.arg_[2]), min(p1.arg_[2], p2.arg_[2]));
            theta.push_back(temp1);
            theta.push_back(temp2);
            theta.push_back(make_pair(max(p1.arg_[1], p2.arg_[1]), min(p1.arg_[1], p2.arg_[1])));
        }
        //bind in second order
        if(!(vt.isLiteral(p1.arg_[0]) && vt.isLiteral(p2.arg_[2]) && p1.arg_[0] != p2.arg_[2])
           && !(vt.isLiteral(p1.arg_[2]) && vt.isLiteral(p2.arg_[0]) && p1.arg_[2] != p2.arg_[0]))
        {
            Binding temp1 = make_pair(max(p1.arg_[0], p2.arg_[2]), min(p1.arg_[0], p2.arg_[2]));
            Binding temp2 = make_pair(max(p1.arg_[2], p2.arg_[0]), min(p1.arg_[2], p2.arg_[0]));
            theta.push_back(temp1);
            theta.push_back(temp2);
            theta.push_back(make_pair(max(p1.arg_[1], p2.arg_[1]), min(p1.arg_[1], p2.arg_[1])));
        }
        return theta;
    }
}

bool Action::operator==(const Action& rhs) const
{
    if(this->type != rhs.type)
        return false;
    for(int i = 0; i < 3; ++i)
        if(this->args[i] != rhs.args[i])
           return false;
    return true;
}

bool Action::reverseEqual(const Action& rhs) const
{
    if(rhs.type != type)
        return false;
    if(rhs.type == GO && rhs.args[0] == args[1] && rhs.args[1] == args[0])
        return true;
    if(rhs.type == PUSH && rhs.args[0] == args[2] && rhs.args[2] == args[0])
        return true;
    return false;
}

bool Action::prepost(const Action& rhs) const
{
    return args[0] == rhs.args[1] && args[1] == rhs.args[0];
}

// Return a vector of vectors of variable bindings
// Each vector of bindings is one possible substitution that
// would cause the action to add predicate p.
vector<vector<Binding> > Action::adds(const Predicate& p,
                                      const VariableTracker& tracker) const
{
    vector<vector<Binding> > returnList;
    for(auto x: addList)
    {
        vector<Binding> temp = Unify(x, p, tracker);
        if(!temp.empty())
            returnList.push_back(temp);
    }
    
    return returnList;
}

//******The rest of these functions are implemented for you******

// Returns true if the action deletes the given predicate
// Unification should not be done here
bool Action::deletes(const Predicate& p) const
{
    for (int i = 0; i < deleteList.size(); ++i)
    {
        if (p == deleteList[i]) return true;
    }
    return false;
}

// This is called either during construction, or during a
// variable rebinding.
// Clears the old lists and inserts predicates into them based on the
// action arguments
// Does nothing if action is start or finish
void Action::fillPredicates()
{
    addList.clear();
    deleteList.clear();
    assert(type == START ||type == FINISH || args[0] != args[1]);
    assert(type == START ||type == FINISH || args[0] != args[2]);
    switch(type)
    {
        case GO:
            addList.push_back(Predicate(Robot, args[1]));
            addList.push_back(Predicate(Empty, args[0]));
            deleteList.push_back(Predicate(Robot, args[0]));
            //deleteList.push_back(Predicate(Empty, args[1]));
            break;
        case PUSH:
            addList.push_back(Predicate(Box, args[2]));
            addList.push_back(Predicate(Empty, args[0]));
            addList.push_back(Predicate(Robot, args[1]));
            deleteList.push_back(Predicate(Box, args[1]));
            deleteList.push_back(Predicate(Empty, args[2]));
            deleteList.push_back(Predicate(Robot, args[0]));
            break;
        default:
            break;
    };
}

vector<Predicate> Action::getPrereqs() const
{
    assert(args[0] != args[1]);
    vector<Predicate> prereqList;
    prereqList.push_back(Predicate(Robot, args[0]));
    switch(type)
    {
        case GO:
            prereqList.push_back(Predicate(Next, args[0], args[1]));
            prereqList.push_back(Predicate(Empty, args[1]));
            break;
        case PUSH:
            prereqList.push_back(Predicate(Linear, args[0], args[1], args[2]));
            prereqList.push_back(Predicate(Box, args[1]));
            prereqList.push_back(Predicate(Empty, args[2]));
            break;
        default:
            break;
    };
    return prereqList;
}


// Substitutes all instances of "former" with instances of "newval"
void Action::substitute(int former, int newval)
{
    bool subst = false;
    for (int i = 0; i < 3; ++i)
    {
        if (args[i] == former)
        {
            args[i] = newval;
            subst = true;
        }
    }
    if (subst) fillPredicates();
}

// Constructs an action
Action::Action(Actions t, int arg1, int arg2, int arg3)
{
    type = t;
    args[0] = arg1;
    args[1] = arg2;
    args[2] = arg3;
    
    fillPredicates();
}




