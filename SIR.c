/******************************************************************************

Welcome to GDB Online.
GDB online is an online compiler and debugger tool for C, C++, Python, Java, PHP, Ruby, Perl,
C#, OCaml, VB, Swift, Pascal, Fortran, Haskell, Objective-C, Assembly, HTML, CSS, JS, SQLite, Prolog.
Code, Compile, Run and Debug online from anywhere in world.

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

// Constant Parameters
#define MAX_VERTICES 10000
#define MAX_EDGES 3000
#define DAYS 500
#define TAU 0.90
#define GAMMA 0.02
#define CAPACITY 50000

/*
	The event capacity was decided seeing the result of multiple
	simulations. The Value never exceeded 30k (or 3 X Vertices in general) 
	hence 50k (or 5 X Vertices) was assumed to be a safe 
	upper bound 
*/

/*
	Output format
	<Day>,<Num of S>,<Num of I>,<Num of R>
	Day Start from 1 till 300.
*/


// Some variables as global parameters for the simulation.
int curr_day = 0;
int event_length = 0;
int susceptible;
int infected = 0;
int recovered = 0;

// A boolean matrix to store adjacent nodes
bool neighbour[MAX_VERTICES][MAX_VERTICES];

// An event node.
struct q_node
{
	int id;
	int day;
	char action;
	/*
		T - Transmit
		R - Recover
	*/
}
events[CAPACITY];


// Structure of a graph node
struct vertex
{
	int id;
	char status;
	int pred_inf_time; // Pedicted infection time
	int rec_time;  // Recovery time

}
graph[MAX_VERTICES];


/*

	Functions start here.

*/


// Function to swap the values of two q_nodes
void q_swap(struct q_node *a,struct q_node *b)
{
	int t_id = a->id;
	int t_day = a->day;
	char t_action = a->action;
	a->id = b->id;
	a->day = b->day;
	a->action = b->action;
	b->id = t_id;
	b->day = t_day;
	b->action = t_action;	
}

// Function to make a correct heap
void q_heapify(int position)
{
	// l,r are the children : smallest is the most prior event  
	int l = 2*position +1;
	int r = 2*position +2;
	int smallest = position;

	// Checking if swap is needed
	if((l < event_length) && (events[l].day < events[smallest].day))
		smallest = l;
	if((r < event_length) && (events[r].day < events[smallest].day))
		smallest = r;
	if(smallest != position)
	{
		q_swap(&events[position],&events[smallest]);
		q_heapify(smallest);
	}
}


// Function to add event to the priority queue.
void enqueue(int id, int day, char action)
{
	if(event_length == CAPACITY)
	{
		printf("Event Overflow.\n");
		exit(1);
	}
	event_length++;
	
	// Iterative variable required for bubbling.
	int i = event_length-1;

	// Initializing Values Of the Event (Node)
	events[i].id = id;
	events[i].day = day;
	events[i].action = action;

	// Placing the node in the queue
	while(i>0 && (events[((i-1)/2)].day > events[i].day))
	{
		q_swap(&events[((i-1)/2)], &events[i]);
		i = (i-1)/2;
	}
}

// Function to remove event from the front of queue
void dequeue()
{
	// Error handling
	if(event_length == 0)
	{
		printf("Length of queue is already zero.\n");
		return;
	}

	// Providing bad values
	events[0].id = -1;
	events[0].day = 302;
	events[0].action = 'e';
	
	if(event_length != 1)
		q_swap(&events[0], &events[event_length-1]);
	event_length--;
	q_heapify(0);
}


// Function to create the graph.
void create_graph(int vertices)
{
	// Making the nodes disjoint.
	for (int i = 0; i < vertices; ++i)
	{
		for (int j = 0; j < vertices; ++j)
		{
			neighbour[i][j] = false;
		}
	}


	for (int i = 0; i < vertices; ++i)
	{
		// Initialising the node values 
		graph[i].id = i;
		graph[i].status = 'S';
		graph[i].pred_inf_time = DAYS+1;
		graph[i].rec_time = DAYS+1;

		// Making a directed graph by joining the links
		int node_neighbours[MAX_EDGES];
		for (int j = 0; j < MAX_EDGES; ++j)
		{
			node_neighbours[j] = rand() % (vertices);
			if(node_neighbours[j] == i) j--;
		}
		for (int j = 0; j < MAX_EDGES; ++j)
		{
			neighbour[i][node_neighbours[j]] =true;
		}
	}

	// Global Variable Initialised
	susceptible = vertices;

	// Converting directed graph to undirected.
	// Using symmetry.
	for (int i = 0; i < vertices; ++i)
	{
		for (int j = i; j < vertices; ++j)
		{
			neighbour[i][j] = neighbour[j][i];
		}
	}
}

// Prints the graph
// ##Not a mandatory funtion as per the project, but for user convenience.
void printgraph(int vertices)
{
	for (int i = 0; i < vertices; ++i)
	{
		printf("\n%d %c %d %d:", i ,graph[i].status,graph[i].pred_inf_time,graph[i].rec_time);
		
		for (int j = 0; j < vertices; ++j)
		{
			if(neighbour[i][j])
			printf(" %d",j);
		}
	}
}

// Minimum Function
int min(int a, int b)
{
	if(a>b) return b;
	else return a;
}

// The Coin-Flip Experiment.
int exponent_variate(float lambda)
{
	int tries = 1;
	int norm_lam = (int)(lambda*100);
	int ran = rand()%100;
	while(ran >= norm_lam)
	{
		tries++;
		ran = rand()%100;
	}
	return tries;
}

// Funtion as Given in Project PDF
// Function for predicting the infection time of targets
void find_trans_SIR( struct vertex *target, struct vertex *source)
{
	if((target->status) == 'S')
	{
		int inf_time = curr_day + exponent_variate(TAU);
		if(inf_time <  min( (source->rec_time), min((target->pred_inf_time),DAYS) ) )
		{
			enqueue((target->id), inf_time, 'T');
			target->pred_inf_time = inf_time;
		}
	}
}

// Funtion as Given in Project PDF
// Function for infecting the targets.
void proc_trans_SIR( struct vertex *target, int vertices)
{
	if((target->status) == 'S')
	{
		susceptible--;
		infected++;
		target->status = 'I';
		target->rec_time = curr_day + exponent_variate(GAMMA) + 1; 
		// The +1 is for minimal recovery time condition
		// A sort of transformation in probability.
		if((target->rec_time) < DAYS)
		{
			enqueue((target->id), (target->rec_time),'R');
		}

		for (int i = 0; i < vertices; ++i)
		{
			if(neighbour[(target->id)][i])
			{
				find_trans_SIR(&graph[i], target);
			}
		}
	}
	dequeue();
}

// Funtion as Given in Project PDF
// Function for recovering the infected people.
void proc_rec_SIR(struct vertex *target)
{
	if((target->status) == 'I')
	{
		infected--;
		recovered++;
		target->status = 'R';
	}
	dequeue();
}

// Funtion as Given in Project PDF
// Processing the event queue/ running the simulation.
void simulation(int vertices)
{
	// The first persont to be infected.
	int first_infect = rand()%vertices;
	printf("First infected vertex is: %d\n",first_infect);
	enqueue(first_infect, 1, 'T');
	graph[first_infect].pred_inf_time = 1;
	curr_day++;

	// Event Processing
	while((event_length) && curr_day < (DAYS+1))
	{
		if((events[0].day) > curr_day)
		{
		  //  printf("")
			printf("On day =  %d, number of suspected = %d, infected = %d, and recovered = %d\n",curr_day,susceptible,infected,recovered);
			curr_day++;
		}
		else if((events[0].action) == 'T')
		{
			proc_trans_SIR(&graph[(events[0].id)], vertices);
		}
		else if((events[0].action) == 'R')
		{
			proc_rec_SIR(&graph[(events[0].id)]);
		}
	}
	// Printing the parameters of remaining days.
	while(curr_day < (DAYS+1))
	{
		printf("On day =  %d, number of suspected = %d, infected = %d, and recovered = %d\n",curr_day,susceptible,infected,recovered);
		curr_day++;
	}
}

int main()
{
	// Seeding the rand();
	srand(time(NULL));
	// Random number of vertices.
	int num_nodes = rand()%MAX_VERTICES;
	create_graph(num_nodes);
	printf("********************************************Covid19-Simulation-Start**************************************************************\n");

	simulation(num_nodes);
	return 0;
}
