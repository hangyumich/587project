#ifndef COMMUNICATOR_HPP
#define COMMUNICATOR_HPP

#include <mpi.h>
#include <vector>
#include "Map.hpp"
#include "structures.hpp"

using namespace std;

typedef struct predicate_t {
	int type;
	int arg_0;
	int arg_1;
	int arg_2;
} comm_predicate;

typedef struct open_pair_t {
	comm_predicate pred;
	int actionId;
} comm_open_pair;

typedef struct link_t {
	comm_predicate pred;
	int causalStep;
	int recipientStep;
} comm_link;

typedef struct threat_t {
	comm_link link;
	int actionId;
} comm_threat;


typedef struct action_t {
	int type;
	int arg_0;
	int arg_1;
	int arg_2;
} comm_action;

typedef struct members_information_t {
	int generation;
	int connection;
	int nextVar;
	int repeat;
	int steps_size;
	int realOrder_size;
	int links_size;
	int threats_size;
	int open_size;
	int orderings_size;
} comm_members_info;


typedef struct order_t {
	int action1;
	int action2;
}  comm_order;


class plan_communicator{

 	MPI_Datatype mpi_action;
 	MPI_Datatype mpi_open_pair;
	MPI_Datatype mpi_predicate;
	MPI_Datatype mpi_link;
	MPI_Datatype mpi_threat;
	MPI_Datatype mpi_members_info;
	MPI_Datatype mpi_order;


	void create_mpi_predicate();

	void create_mpi_open_pair();
	void create_mpi_order();

	//must be called after create_mpi_predicate()
	void create_mpi_link();

	//must be called after create_mpi_link()
	void create_mpi_threat();

	void create_mpi_action();
	void create_mpi_members_info();

	comm_predicate toCommPredicate(Predicate pred);

	Link toPlannerLink(comm_link l);
	Threat toPlannerThreat(comm_threat t);

public:
	plan_communicator();
	~plan_communicator();

	void sendPlan(int destination, Plan& p, int tag);

	Plan* recvPlan();

};


#endif