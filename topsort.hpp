/*
 topsort.h
 ---------
 Contains functions "topSort" and "isOrderConsistent"
 which are useful for checking order consistency and
 making partially ordered plans into linear plans (for debugging)
 */
#ifndef TOPSORT_H
#define TOPSORT_H

#include <queue>
#include <stack>
#include <fstream>
#include <cctype>

using namespace std;

/*
 Suppose you have a graph with whose vertex count is given by "numVertices",
 with the vertices labeled 0, 1, 2, ..., numVertices-1
 You are given a container "orderings" of ordered pairs of integer (v1, v2),
 representing directed edges v1 --> v2.
 
 The function "topSort" takes orderings and numVertices as inputs,
 and returns a pair of things. The first element of the pair,
 a vector of integers, is a topological sort of the input graph.
 This means that it is a linearly ordered list of integers
 s_0, s_1, ... s_numVertices such that every edge (s_i, s_j) is such that i < j
 
 The second element of the pair is a boolean that is true if the sort was
 successful, and false if unsuccessful. A sort is unsuccessful if the input
 graph contains a cycle.
 
 A plan is order consistent if and only if it can be successfully
 topologically sorted - hence the usefulness of this function in our project.
 A topological sort is also useful for taking a partially ordered plan
 and making it into a linear sequence of steps, for ease of reading.
 */

template <typename T>
pair<vector<int>, bool> topSort(const T& orderings, int numVertices)
{
    vector<int> sorted; //Container for topological sort of vertices
    bool successful = true; //Whether or not the sort was successful
    vector<int> incomingEdges(numVertices,0); //The number of incoming edges of all vertices
    vector<vector<int> > outgoingEdges(numVertices); //List of outgoing edges per vertex
    queue<int> S; // Queue of vertices with no incoming edges
    int remainingEdges = int(orderings.size()); //Number of edges remaining in graph
    
    // For each vertex, create a list of children
    // Also, keep track of the number of incoming edges for each vertex
    for (typename T::const_iterator i = orderings.cbegin(); i != orderings.cend(); ++i)
    {
        outgoingEdges[i->first].push_back(i->second);
        ++incomingEdges[i->second]; //Number of incoming edges
    }
    // Add all vertices with zero incoming edges to a queue
    for (int i = int(incomingEdges.size() - 1); i >= 0; --i)
    {
        if (incomingEdges[i] == 0)
        {
            S.push(i);
        }
    }
    
    // Perform topological sort:
    // While the queue is not empty, take out a vertex, put it in sorted list,
    // remove all edges emanating from it,
    // and add to the queue any children who now have no incoming edges
    while (!S.empty())
    {
        int vertex = S.front();
        S.pop();
        sorted.push_back(vertex);
        const vector<int>& outgoing = outgoingEdges[vertex];
        for (int i = 0; i < outgoing.size(); ++i)
        {
            int o = outgoing[i];
            int& income = incomingEdges[o];
            --income; //Remove an edge
            --remainingEdges;
            if (income == 0) S.push(o); // Insert a vertex if no more incoming edges
        }
    }
    if (remainingEdges > 0) successful = false; //If edges are left, a cycle exists
    
    return make_pair(sorted, successful);
}

/*
 The following function takes a graph, specified in the same way as above,
 and returns true if it is order consistent. Otherwise, it returns false.
 This function can be used to check for consistency of plans in your algorithm.
 */
template <typename T>
bool isOrderConsistent(const T& orderings, int numVertices)
{
    pair<vector<int>, bool> result = topSort(orderings, numVertices);
    return result.second;
}


#endif