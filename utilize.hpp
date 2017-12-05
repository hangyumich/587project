//
//  utilize.hpp
//  Sokoban
//
//  Created by Luyao Yuan on 2016/3/22.
//  Copyright © 2016年 Luyao Yuan. All rights reserved.
//

#ifndef utilize_hpp
#define utilize_hpp

#include "structures.hpp"
#include "Map.hpp"
#include "variables.hpp"
#include <iostream>

void printPredicate( Predicate& p,  VariableTracker& vt, ostream& os);
void printAction( Action& a,  VariableTracker& vt, ostream& os);
void printPlan( Plan& p,  VariableTracker& vt, ostream& os);

#endif /* utilize_hpp */
