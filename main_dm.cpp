#include <mpi.h>
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
#include "communicator.hpp"
#include "getPlan.hpp"

using namespace std;

bool planSearch(Plan&);

int main(int argc, char** argv)
{
    int rank , size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    plan_communicator communicator;


    
    //manager
    if (rank == 0) {
        //define the map
        Map newM(9,9);
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

        newM.setPosition(7, 1, OBSTACLE);
        newM.setPosition(7, 1, OBSTACLE);
        //newM.setPosition(3, 6, OBSTACLE);
        newM.setPosition(3, 5, OBSTACLE);
        newM.setPosition(3, 4, OBSTACLE);
        newM.setPosition(3, 3, OBSTACLE);
         
     
        newM.finishMap();
        cout << newM;
        
        //add initial conditions
        vector<Predicate> initiations = newM.map2Predicates(false);
        vector<Predicate> goals;
        goals.push_back(Predicate(Box, newM.cor2ind(2, 1)));
        

        Action first(START, -3, -2);
        first.addList = initiations;
        Action last(FINISH, -5, -4);
        
        //initiate everything
        vector<float> momentum = newM.map2Momentum(goals, false);
        Plan* p = new Plan();
        p->generation = 0;
        p->connection = 0;
        p->steps.push_back(first);
        p->steps.push_back(last);
        p->realOrder.resize(0);
        p->realOrder.push_back(0);
        p->realOrder.push_back(1);
        for(auto goal: goals)
            p->open.push_back(make_pair(goal, 1));
        p->orderings.insert(Ordering(0,1));
        p->nextVar = 81;

        priority_queue<Plan*, vector<Plan*>, planCmp> pq;
        pq.push(p);

        double start_time = MPI_Wtime();

        // create (size-1)*1000 tasks
        managerPlanSearch(pq, 20);
        //planSearch(*p, 0);

        MPI_Status status;
        for (int i = 1; i < size; ++i) {
            Plan* plan = pq.top();
            pq.pop();

            // VariableTracker vt(81);
            // cout << "sendPlan 0" << endl;
            // printPlan(*plan, vt, cout);

            communicator.sendPlan(i, i, *plan);
        }

        bool is_found = false;
        while (true) {
            MPI_Recv(&is_found, 1, MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (is_found || pq.empty()) {
                break;
            }

            Plan* plan = pq.top();
            pq.pop();

            // VariableTracker vt(81);
            // cout << "sendPlan " << 21-pq.size()-1 << endl;
            // printPlan(*plan, vt, cout);

            communicator.sendPlan(status.MPI_SOURCE, status.MPI_TAG, *plan);
        }

        double end_time = MPI_Wtime();
        if (is_found) {
            cout << "I'm a smart robot and I find your sokoban problem solution in "  << " times' search\n";
        } else {
            cout << "I'm a stupid robot and I cannot find your Sokoban solution. :(" << endl;
        }
        cout << "elapsed time: " << end_time - start_time << endl;
    } 
    // worker
    else {
        MPI_Status status;
        Plan* plan;
        int i=0;
        while (1) {
            plan = communicator.recvPlan(0, rank);

            // VariableTracker vt(81);
            // cout << "recvPlan " << i << endl;
            // printPlan(*plan, vt, cout);

            bool is_found = planSearch(*plan, rank);
            delete plan;
            MPI_Send(&is_found, 1, MPI_BYTE, 0, rank, MPI_COMM_WORLD);
            i++;
        }
    }
}

