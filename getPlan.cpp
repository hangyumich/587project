#include "getPlan.hpp"

void managerPlanSearch(priority_queue<Plan*, vector<Plan*>, planCmp>& pq, int expected_size)
{

	int effortLimit = 0;
    while(pq.size() < expected_size)
    {
        Plan* p;
        p = pq.top();
        pq.pop();

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
    
        //case 1 no action adding, only add order based on threats
        if(!p->threats.empty())
        {
            if(p->threats.begin()->threatened.causalStep != 0)
            {
                auto temp = new Plan(*p);
                temp->orderings.insert(make_pair(temp->threats.begin()->actionId, temp->threats.begin()->threatened.causalStep));
                temp->threats.erase(temp->threats.begin());
                temp->generation = effortLimit;

                pq.push(temp);
            }
            if(p->threats.begin()->threatened.recipientStep != 1)
            {
                auto temp = new Plan(*p);
                temp->orderings.insert(make_pair(temp->threats.begin()->threatened.recipientStep, temp->threats.begin()->actionId));
                temp->threats.erase(temp->threats.begin());
                temp->generation = effortLimit;
                
                pq.push(temp);
            }
            delete p;
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

                    VariableTracker vt(81);
                    auto unifyList = p->steps[i].adds(p->open[0].first, vt);

                    if(!unifyList.empty())  
                    {
                        for(auto onePossible: unifyList)
                        {
                            Plan* tempPlan;
                            try
                            {
                                tempPlan = new Plan(*p);
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

                                pq.push(tempPlan);
                            }//get rid of not moving action
                            catch(int notMoving)
                            {
                                delete tempPlan;                                
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
                    delete tempPlan;
                    delete tempPlan1;
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
            
            pq.push(tempPlan);
          
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

                    pq.push(tempPlan1);

            }
        }//case 2 finish
    }//priority depth first search
}






















bool planSearch(Plan& plan, int rank)
{
	priority_queue<Plan*, vector<Plan*>, planCmp> pq;
	Plan* p_ptr = new Plan(plan);
	pq.push(p_ptr);

	int effortLimit = plan.generation;

    while(!pq.empty())
    {

cout <<rank << " "<<effortLimit <<  " " <<pq.size()<<endl;

        Plan* p;
        p = pq.top();
        pq.pop();

        if(p->open.empty() && p->threats.empty() && isOrderConsistent(p->orderings, int(p->steps.size())))
        {
            cout << "plan found" << endl;
            // p->realOrder = topSort(p->orderings, int(p->steps.size())).first;
            // delete p;
            // while(!pq.empty()){
            // 	delete pq.top();
            // }
            return true;
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
    
        //case 1 no action adding, only add order based on threats
        if(!p->threats.empty())
        {
            if(p->threats.begin()->threatened.causalStep != 0)
            {
                auto temp = new Plan(*p);
                temp->orderings.insert(make_pair(temp->threats.begin()->actionId, temp->threats.begin()->threatened.causalStep));
                temp->threats.erase(temp->threats.begin());
                temp->generation = effortLimit;

                pq.push(temp);
            }
            if(p->threats.begin()->threatened.recipientStep != 1)
            {
                auto temp = new Plan(*p);
                temp->orderings.insert(make_pair(temp->threats.begin()->threatened.recipientStep, temp->threats.begin()->actionId));
                temp->threats.erase(temp->threats.begin());
                temp->generation = effortLimit;
                
                pq.push(temp);
            }
            delete p;
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

                    VariableTracker vt(81);
                    auto unifyList = p->steps[i].adds(p->open[0].first, vt);

                    if(!unifyList.empty())  
                    {
                        for(auto onePossible: unifyList)
                        {
                            Plan* tempPlan;
                            try
                            {
                                tempPlan = new Plan(*p);
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

                                pq.push(tempPlan);
                            }//get rid of not moving action
                            catch(int notMoving)
                            {
                                delete tempPlan;                             
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
                    delete tempPlan;
                    delete tempPlan1;
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
            
            pq.push(tempPlan);
          
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

                    pq.push(tempPlan1);

            }
        }//case 2 finish

    }//priority depth first search

    return false;
}


bool planSearchStep(Plan& plan, int rank, vector<Plan>& result)
{
    int effortLimit = plan.generation;

    Plan *p = new Plan(plan);

    if(p->open.empty() && p->threats.empty() && isOrderConsistent(p->orderings, int(p->steps.size())))
    {
        cout << "plan found" << endl;
        return true;
    }

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

    //case 1 no action adding, only add order based on threats
    if(!p->threats.empty())
    {
        if(p->threats.begin()->threatened.causalStep != 0)
        {
            auto temp = new Plan(*p);
            temp->orderings.insert(make_pair(temp->threats.begin()->actionId, temp->threats.begin()->threatened.causalStep));
            temp->threats.erase(temp->threats.begin());
            temp->generation = effortLimit;

            result.push_back(temp);
        }
        if(p->threats.begin()->threatened.recipientStep != 1)
        {
            auto temp = new Plan(*p);
            temp->orderings.insert(make_pair(temp->threats.begin()->threatened.recipientStep, temp->threats.begin()->actionId));
            temp->threats.erase(temp->threats.begin());
            temp->generation = effortLimit;
            
            result.push_back(temp);
        }
        delete p;
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

                VariableTracker vt(81);
                auto unifyList = p->steps[i].adds(p->open[0].first, vt);

                if(!unifyList.empty())  
                {
                    for(auto onePossible: unifyList)
                    {
                        Plan* tempPlan;
                        try
                        {
                            tempPlan = new Plan(*p);
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

                            result.push_back(tempPlan);
                        }//get rid of not moving action
                        catch(int notMoving)
                        {
                            delete tempPlan;                             
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
                delete tempPlan;
                delete tempPlan1;
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
        
        result.push_back(tempPlan);
      
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

            result.push_back(tempPlan1);
        }
    }//case 2 finish

    // }//priority depth first search

    return false;
}