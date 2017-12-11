#include "communicator.hpp"
#include "utilize.hpp"


void plan_communicator::create_mpi_predicate(){
	int             blocklengths[4] = {1,1,1,1};
	MPI_Datatype    types[4] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT};
	MPI_Aint        offsets[4];

	offsets[0] = offsetof(comm_predicate, type);
	offsets[1] = offsetof(comm_predicate, arg_0);
	offsets[2] = offsetof(comm_predicate, arg_1);
	offsets[3] = offsetof(comm_predicate, arg_2);

	MPI_Type_create_struct(4, blocklengths, offsets, types, &mpi_predicate);
	MPI_Type_commit(&mpi_predicate);
}

void plan_communicator::create_mpi_open_pair(){
	int             blocklengths[2] = {1,1};
	MPI_Datatype    types[2] = {mpi_predicate, MPI_INT};
	MPI_Aint        offsets[2];

	offsets[0] = offsetof(comm_open_pair, pred);
	offsets[1] = offsetof(comm_open_pair, actionId);


	MPI_Type_create_struct(2, blocklengths, offsets, types, &mpi_open_pair);
	MPI_Type_commit(&mpi_open_pair);
}


void plan_communicator::create_mpi_link(){
	int             blocklengths[3] = {1,1,1};
	MPI_Datatype    types[3] = {mpi_predicate, MPI_INT, MPI_INT};
	MPI_Aint        offsets[3];

	offsets[0] = offsetof(comm_link, pred);
	offsets[1] = offsetof(comm_link, causalStep);
	offsets[2] = offsetof(comm_link, recipientStep);


	MPI_Type_create_struct(3, blocklengths, offsets, types, &mpi_link);
	MPI_Type_commit(&mpi_link);
}

void plan_communicator::create_mpi_threat(){
	int             blocklengths[2] = {1,1};
	MPI_Datatype    types[2] = {mpi_link, MPI_INT};
	MPI_Aint        offsets[2];

	offsets[0] = offsetof(comm_threat, link);
	offsets[1] = offsetof(comm_threat, actionId);


	MPI_Type_create_struct(2, blocklengths, offsets, types, &mpi_threat);
	MPI_Type_commit(&mpi_threat);
}

void plan_communicator::create_mpi_action(){
	int             blocklengths[4] = {1,1,1,1};
	MPI_Datatype    types[4] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT};
	MPI_Aint        offsets[4];

	offsets[0] = offsetof(comm_action, type);
	offsets[1] = offsetof(comm_action, arg_0);
	offsets[2] = offsetof(comm_action, arg_1);
	offsets[3] = offsetof(comm_action, arg_2);

	MPI_Type_create_struct(4, blocklengths, offsets, types, &mpi_action);
	MPI_Type_commit(&mpi_action);	
}

void plan_communicator::create_mpi_members_info(){
	int             blocklengths[10] = {1,1,1,1,1,1,1,1,1,1};
	MPI_Datatype    types[10] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT};
	MPI_Aint        offsets[10];

	offsets[0] = offsetof(comm_members_info, generation);
	offsets[1] = offsetof(comm_members_info, connection);
	offsets[2] = offsetof(comm_members_info, nextVar);
	offsets[3] = offsetof(comm_members_info, repeat);
	offsets[4] = offsetof(comm_members_info, steps_size);
	offsets[5] = offsetof(comm_members_info, realOrder_size);
	offsets[6] = offsetof(comm_members_info, links_size);
	offsets[7] = offsetof(comm_members_info, threats_size);
	offsets[8] = offsetof(comm_members_info, open_size);
	offsets[9] = offsetof(comm_members_info, orderings_size);

	MPI_Type_create_struct(10, blocklengths, offsets, types, &mpi_members_info);
	MPI_Type_commit(&mpi_members_info);	
}

void plan_communicator::create_mpi_order(){
	int             blocklengths[2] = {1,1};
	MPI_Datatype    types[2] = {MPI_INT, MPI_INT};
	MPI_Aint        offsets[2];

	offsets[0] = offsetof(comm_order, action1);
	offsets[1] = offsetof(comm_order, action2);

	MPI_Type_create_struct(2, blocklengths, offsets, types, &mpi_order);
	MPI_Type_commit(&mpi_order);
}


comm_predicate plan_communicator::toCommPredicate(Predicate pred){
	comm_predicate res;
	res.type = pred.type_;
	res.arg_0 = pred.arg_[0];
	res.arg_1 = pred.arg_[1];
	res.arg_2 = pred.arg_[2];

	return res;
}

Link plan_communicator::toPlannerLink(comm_link l){
	Predicate p(static_cast<Predicates>(l.pred.type), l.pred.arg_0, l.pred.arg_1, l.pred.arg_2);
	return Link(p, l.causalStep, l.recipientStep);
}

Threat plan_communicator::toPlannerThreat(comm_threat t){
	Link l = toPlannerLink(t.link);
	return Threat(l, t.actionId);
}

plan_communicator::plan_communicator(){
	create_mpi_action();
	create_mpi_members_info();
	create_mpi_predicate();
	create_mpi_open_pair();
	create_mpi_link();
	create_mpi_threat();
	create_mpi_order();
}

plan_communicator::~plan_communicator(){
	MPI_Type_free(&mpi_action);
	MPI_Type_free(&mpi_members_info);
	MPI_Type_free(&mpi_threat);
	MPI_Type_free(&mpi_order);
	MPI_Type_free(&mpi_open_pair);
	MPI_Type_free(&mpi_link);
	MPI_Type_free(&mpi_predicate);
}

void plan_communicator::sendPlan(int destination, Plan& p, int tag){
	MPI_Request sendrequest[7];

	comm_members_info members_info;
	members_info.generation = p.generation;
	members_info.connection = p.connection;
	members_info.nextVar = p.nextVar;
	members_info.repeat = p.repeat;
	members_info.steps_size = p.steps.size();
	members_info.realOrder_size = p.realOrder.size();
	members_info.links_size = p.links.size();
	members_info.threats_size = p.threats.size();
	members_info.open_size = p.open.size();
	members_info.orderings_size = p.orderings.size();
	MPI_Isend(&members_info, 1, mpi_members_info, destination, tag, MPI_COMM_WORLD, &sendrequest[0]);


	comm_action* comm_action_array = new comm_action[members_info.steps_size];
	for(int i=0; i<members_info.steps_size; ++i){
		comm_action_array[i].type = p.steps[i].type;
		comm_action_array[i].arg_0 = p.steps[i].args[0];
		comm_action_array[i].arg_1 = p.steps[i].args[1];
		comm_action_array[i].arg_2 = p.steps[i].args[2];
	}
	MPI_Isend(comm_action_array, members_info.steps_size, mpi_action, destination, tag, MPI_COMM_WORLD, &sendrequest[1]);
	delete[] comm_action_array;


	int* realOrder_array = new int[members_info.realOrder_size];
	for(int i=0; i<members_info.realOrder_size; ++i){
		realOrder_array[i] = p.realOrder[i];
	}
	MPI_Isend(realOrder_array, members_info.realOrder_size, MPI_INT, destination, tag, MPI_COMM_WORLD, &sendrequest[2]);
	delete[] realOrder_array;


	comm_link* comm_link_array = new comm_link[members_info.links_size];
	for(int i=0; i<members_info.links_size; ++i){
		comm_predicate cp = toCommPredicate(p.links[i].pred);
		comm_link_array[i].pred = cp;
		comm_link_array[i].causalStep = p.links[i].causalStep;
		comm_link_array[i].recipientStep = p.links[i].recipientStep;
	}
	MPI_Isend(comm_link_array, members_info.links_size, mpi_link, destination, tag, MPI_COMM_WORLD, &sendrequest[3]);
	delete[] comm_link_array;

	comm_threat* comm_threat_array = new comm_threat[members_info.threats_size];
	int j=0;
	for(auto it=p.threats.begin(); it!=p.threats.end(); it++){
		comm_threat_array[j].link.pred = toCommPredicate(it->threatened.pred);
		comm_threat_array[j].link.causalStep = it->threatened.causalStep;
		comm_threat_array[j].link.recipientStep = it->threatened.recipientStep;
		comm_threat_array[j].actionId = it->actionId;
		j++;
	}
	MPI_Isend(comm_threat_array, members_info.threats_size, mpi_threat, destination, tag, MPI_COMM_WORLD, &sendrequest[4]);
	delete[] comm_threat_array;



	comm_open_pair* comm_open_pair_array = new comm_open_pair[members_info.open_size];
	for(int i=0; i<members_info.open_size; i++){
		comm_open_pair_array[i].pred = toCommPredicate(p.open[i].first);
		comm_open_pair_array[i].actionId = p.open[i].second;
	}
	MPI_Isend(comm_open_pair_array, members_info.open_size, mpi_open_pair, destination, tag, MPI_COMM_WORLD, &sendrequest[5]);
	delete[] comm_open_pair_array;


	comm_order* comm_order_array = new comm_order[members_info.orderings_size];
	j=0;
	for(auto it=p.orderings.begin(); it!=p.orderings.end(); ++it){
		comm_order_array[j].action1 = it->first;
		comm_order_array[j].action2 = it->second;
		++j;
	}
	MPI_Isend(comm_order_array, members_info.orderings_size, mpi_order, destination, tag, MPI_COMM_WORLD, &sendrequest[6]);
	delete[] comm_order_array; 
}

Plan* plan_communicator::recvPlan(){
	Plan* p_ptr = new Plan();

	comm_members_info members_info;
	MPI_Status status;
	MPI_Recv(&members_info, 1, mpi_members_info, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
	int source = status.MPI_SOURCE;
	int tag = status.MPI_TAG;

	p_ptr->generation = members_info.generation;
	p_ptr->connection = members_info.connection;
	p_ptr->nextVar    = members_info.nextVar;
	p_ptr->repeat     = members_info.repeat;

	comm_action* comm_action_array = new comm_action[members_info.steps_size];
	MPI_Recv(comm_action_array, members_info.steps_size, mpi_action, source, tag, MPI_COMM_WORLD, &status);
	for(int i=0; i<members_info.steps_size; ++i){
		Action a(static_cast<Actions>(comm_action_array[i].type), comm_action_array[i].arg_0, comm_action_array[i].arg_1, comm_action_array[i].arg_2);
		p_ptr->steps.push_back(a);
	}
	delete[] comm_action_array;


	int* realOrder_array = new int[members_info.realOrder_size];
	MPI_Recv(realOrder_array, members_info.realOrder_size, MPI_INT, source, tag, MPI_COMM_WORLD, &status);
	for(int i=0; i<members_info.realOrder_size; ++i){
		p_ptr->realOrder.push_back(realOrder_array[i]);
	}
	delete[] realOrder_array;


	comm_link* comm_link_array = new comm_link[members_info.links_size];
	MPI_Recv(comm_link_array, members_info.links_size, mpi_link, source, tag, MPI_COMM_WORLD, &status);
	for(int i=0; i<members_info.links_size; ++i){
		Link l = toPlannerLink(comm_link_array[i]);
		p_ptr->links.push_back(l);
	}
	delete[] comm_link_array;


	comm_threat* comm_threat_array = new comm_threat[members_info.threats_size];
	MPI_Recv(comm_threat_array, members_info.threats_size, mpi_threat, source, tag, MPI_COMM_WORLD, &status);
	for(int i=0; i<members_info.threats_size; ++i){
		Threat t = toPlannerThreat(comm_threat_array[i]);
		p_ptr->threats.insert(t);

	}
	delete[] comm_threat_array;


	comm_open_pair* comm_open_pair_array = new comm_open_pair[members_info.open_size];
	MPI_Recv(comm_open_pair_array, members_info.open_size, mpi_open_pair, source, tag, MPI_COMM_WORLD, &status);
	for(int i=0; i<members_info.open_size; ++i){
		Predicate p(static_cast<Predicates>(comm_open_pair_array[i].pred.type), comm_open_pair_array[i].pred.arg_0,
					comm_open_pair_array[i].pred.arg_1,comm_open_pair_array[i].pred.arg_2);
		p_ptr->open.push_back(make_pair(p, comm_open_pair_array[i].actionId));
	}
	delete[] comm_open_pair_array;

	comm_order* comm_order_array = new comm_order[members_info.orderings_size];
	MPI_Recv(comm_order_array, members_info.orderings_size, mpi_order, source, tag, MPI_COMM_WORLD, &status);
	for(int i=0; i<members_info.orderings_size; ++i){
		p_ptr->orderings.insert( Ordering(comm_order_array[i].action1, comm_order_array[i].action2) );
	}
	delete[] comm_order_array;

	return p_ptr;
}



