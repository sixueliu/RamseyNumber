#include <assert.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <algorithm>

using namespace std;

/* limits on the size of the problem. */
#define MAX_VARS	50000000
//#define MAX_CLAUSES	50063861

/*parameters of the instance*/
int     num_vars;			//var index from 1 to num_vars
int     num_clauses;		//clause index from 1 to num_clauses
//int	clause_len;		//length of clause, all identical
#define	clause_len	6
int	var_ap;			//variable appearance, all identical

/* literal arrays */				
int	*var_poslit[MAX_VARS];				//var_lit[v][j]: the clause number of the clause that the j'th positive literal of v occurs in.
int	var_poslit_count[MAX_VARS];
int	*var_neighbor[MAX_VARS];
int	neighbor_num;

//int	clause_var[MAX_CLAUSES][clause_len];
int	(*clause_var)[clause_len];
			
/* Information about the clauses */
//int     sat_count[MAX_CLAUSES];			
int     *sat_count;			

//unsat clauses stack
//int	unsat_stack[MAX_CLAUSES];		//store the unsat clause number
int	*unsat_stack;		//store the unsat clause number
int	unsat_stack_fill_pointer;
//int	index_in_unsat_stack[MAX_CLAUSES];	//which position is a clause in the unsat_stack
int	*index_in_unsat_stack;	//which position is a clause in the unsat_stack

/* Information about solution */
bool     cur_soln[MAX_VARS];	//the current solution, with 1's for True variables, and 0's for False variables
// Information during search
bool	conf_change[MAX_VARS];
bool	cscc[MAX_VARS];
int	score[MAX_VARS];
int	score2[MAX_VARS];
int	step, time_stamp[MAX_VARS];


// all vars with conf_change = 1 and score > 0
int	goodvar[MAX_VARS];
int	goodvar_fill_pointer = 0;
bool	already_in_goodvar[MAX_VARS];

int	lastvar;

void (*pick_greedy)();
void (*pick_random)();

int build_instance(/*char *filename*/)
{
	//char    line[1024];
	int     cur_var;
	int     i,j;
	int		v,c;//var, clause
	
	//ifstream infile(filename);
	//if(infile==NULL) {
	//	cout<<"c Invalid filename: "<< filename<<endl;
	//	return 0;
	//}

	/*** build problem data structures of the instance ***/
	//infile.getline(line,1024);

	//sscanf(line, "%d %d %d %d", &num_vars, &num_clauses, &i, &var_ap);
	scanf("%d %d %d %d", &num_vars, &num_clauses, &i, &var_ap);
	if (num_vars >= MAX_VARS) {
		cout << "MAX_VARS reached, please enlarge!\n";
		exit(-1);
	}
	assert(i == clause_len);
	/* new memory: static allocation limitation*/
	clause_var = (int (*)[clause_len]) new int[(num_clauses + 1) * clause_len];
	sat_count = new int[num_clauses + 1];
	unsat_stack = new int[num_clauses + 1];
	index_in_unsat_stack = new int[num_clauses + 1];

	/////////////////////////////////////////////////////////////////////////////
	//Now, read the clauses, one at a time.
	for (c = 1; c <= num_clauses; ++c) {
		//clause_var[c] = new int[clause_len];
		for (i = 0; i < clause_len; ++i) {
			//infile >> cur_var;
			cin >> cur_var;
			clause_var[c][i] = cur_var;
		}
	}
	/*** build neighborhood relationship from input	***/
	for (v = 1; v <= num_vars; ++v) {
		//infile >> neighbor_num;
		cin >> neighbor_num;
		var_neighbor[v] = new int[neighbor_num];
		for (i = 0; i < neighbor_num; ++i) {
			//infile >> cur_var;
			cin >> cur_var;
			var_neighbor[v][i] = cur_var;
		}
	}
	//infile.close();
	
	//creat var literal arrays
	for (v = 1; v <= num_vars; ++v) {
		var_poslit[v] = new int[var_ap];
		// reset to store
		var_poslit_count[v] = 0;
	}
	//scan all clauses to build up var literal arrays
	for (c = 1; c <= num_clauses; ++c) {
		for(i = 0; i < clause_len; ++i) {
			v = clause_var[c][i];
			var_poslit[v][var_poslit_count[v]] = c;
			++var_poslit_count[v];
		}
	}
	// virtual variable 0: worse than any other var
	// to avoid lscore overflow
	score[0] = -100000000;
	score2[0] = 100000000;
	time_stamp[0] = 2100000000;
	conf_change[0] = 0;
	cscc[0] = 0;
}


void free_memory()
{
	int i;
	//for (i = 1; i <= num_clauses; ++i) 
	//	delete[] clause_var[i];
	
	for(i = 1; i <= num_vars; ++i) {
		delete[] var_poslit[i];
		delete[] var_neighbor[i];
	}
}

//initiation of the algorithm
void init()
{
	step = 1;
	//last time no var was flipped 
	lastvar = -1;
	goodvar_fill_pointer = 0;
	unsat_stack_fill_pointer = 0;

	int	v, c;
	int	i, j;
	
	//init var: including uniform random assignment
	for (v = 1; v <= num_vars; v++) {
		cur_soln[v] = rand() & 1;
		time_stamp[v] = 0; // never been flipped
		conf_change[v] = 1;
		cscc[v] = 1;
	}

	/* figure out sat_count, and init unsat_stack */
	for (c = 1; c <= num_clauses; ++c) {
		sat_count[c] = 0;
		for(j = 0; j < clause_len; ++j) {
			if (cur_soln[clause_var[c][j]]) {
				++sat_count[c];
			}
		}
		// all false or all true
		if (sat_count[c] == 0 || sat_count[c] == clause_len) {
			unsat_stack[unsat_stack_fill_pointer] = c;
			index_in_unsat_stack[c] = unsat_stack_fill_pointer++;
		}
	}
	// calculate score
	for (v = 1; v <= num_vars; ++v) {
		score[v] = 0;
		score2[v] = 0;
		for (i = 0; i < var_ap; ++i) {
			c = var_poslit[v][i];
			if (cur_soln[v]) {
				if (sat_count[c] == 1) {
					--score[v];
				}
				else if (sat_count[c] == 2) {
					--score2[v];
				}
				else if (sat_count[c] == clause_len - 1) {
					++score2[v];
				}
				else if (sat_count[c] == clause_len) {
					++score[v];
				}
			}
			else { // if (cur_soln[v] == 0)
				if (sat_count[c] == clause_len - 1) {
					--score[v];
				}
				else if (sat_count[c] == clause_len - 2) {
					--score2[v];
				}
				else if (sat_count[c] == 1) {
					++score2[v];
				}
				else if (sat_count[c] == 0) {
					++score[v];
				}
			}
		}
		// add goodvar
		if (score[v] > 0) {
			goodvar[goodvar_fill_pointer++] = v;
			already_in_goodvar[v] = 1;
		}
		else
			already_in_goodvar[v] = 0;
	}
}

void flip_update(int flipvar)
{
	int c, i, j;
	int v;
	cur_soln[flipvar] = !cur_soln[flipvar];
	int	org_score = score[flipvar];
	int	org_score2 = score2[flipvar];
	
	if (cur_soln[flipvar]) {
		for (i = 0; i < var_ap; ++i) {
			c = var_poslit[flipvar][i];
			++sat_count[c];
			if (sat_count[c] == 1) {
				// new sat c
				unsat_stack[index_in_unsat_stack[c]] = unsat_stack[--unsat_stack_fill_pointer];
				index_in_unsat_stack[unsat_stack[index_in_unsat_stack[c]]] = index_in_unsat_stack[c];
				for (j = 0; j < clause_len; ++j) {
					--score[clause_var[c][j]];
					++score2[clause_var[c][j]];
					cscc[clause_var[c][j]] = 1;
				}
			}
			else if (sat_count[c] == 2) {
				for (j = 0; j < clause_len; ++j) {
					--score2[clause_var[c][j]];
					if (cur_soln[clause_var[c][j]]) {
						++score[clause_var[c][j]];
					}
				}
			}
			else if (sat_count[c] == 3) {
				for (j = 0; j < clause_len; ++j) {
					if (cur_soln[clause_var[c][j]])
						++score2[clause_var[c][j]];
					else
						--score2[clause_var[c][j]];
				}
			}
			/*else if (sat_count[c] == 3) {
				for (j = 0; j < clause_len; ++j) {
					if (cur_soln[clause_var[c][j]])
						++score2[clause_var[c][j]];
				}
			}
			else if (sat_count[c] == clause_len - 2) {
				for (j = 0; j < clause_len; ++j) {
					if (!cur_soln[clause_var[c][j]])
						--score2[clause_var[c][j]];
				}
			}*/
			else if (sat_count[c] == clause_len - 1) {
				for (j = 0; j < clause_len; ++j) {
					++score2[clause_var[c][j]];
					if (!cur_soln[clause_var[c][j]]) {
						--score[clause_var[c][j]];
					}
				}
			}
			else if (sat_count[c] == clause_len) {
				// new unsat c
				unsat_stack[unsat_stack_fill_pointer] = c;
				index_in_unsat_stack[c] = unsat_stack_fill_pointer++;
				for (j = 0; j < clause_len; ++j) {
					++score[clause_var[c][j]];
					--score2[clause_var[c][j]];
					cscc[clause_var[c][j]] = 1;
				}
			}
		}
	}
	else {// if (cur_soln[flipvar] == 0) {
		for (i = 0; i < var_ap; ++i) {
			c = var_poslit[flipvar][i];
			--sat_count[c];
			if (sat_count[c] == clause_len - 1) {
				// new sat c
				unsat_stack[index_in_unsat_stack[c]] = unsat_stack[--unsat_stack_fill_pointer];
				index_in_unsat_stack[unsat_stack[index_in_unsat_stack[c]]] = index_in_unsat_stack[c];
				for (j = 0; j < clause_len; ++j) {
					--score[clause_var[c][j]];
					++score2[clause_var[c][j]];
					cscc[clause_var[c][j]] = 1;
				}
			}
			else if (sat_count[c] == clause_len - 2) {
				for (j = 0; j < clause_len; ++j) {
					--score2[clause_var[c][j]];
					if (!cur_soln[clause_var[c][j]]) {
						++score[clause_var[c][j]];
					}
				}
			}
			else if (sat_count[c] == clause_len - 3) {
				for (j = 0; j < clause_len; ++j) {
					if (!cur_soln[clause_var[c][j]]) {
						++score2[clause_var[c][j]];
					}
					else
						--score2[clause_var[c][j]];
				}
			}
			/*else if (sat_count[c] == clause_len - 3) {
				for (j = 0; j < clause_len; ++j) {
					if (!cur_soln[clause_var[c][j]]) {
						++score2[clause_var[c][j]];
					}
				}
			}
			else if (sat_count[c] == 2) {
				for (j = 0; j < clause_len; ++j) {
					if (cur_soln[clause_var[c][j]]) {
						--score2[clause_var[c][j]];
					}
				}
			}*/
			else if (sat_count[c] == 1) {
				for (j = 0; j < clause_len; ++j) {
					++score2[clause_var[c][j]];
					if (cur_soln[clause_var[c][j]]) {
						--score[clause_var[c][j]];
					}
				}
			}
			else if (sat_count[c] == 0) {
				// new unsat c
				unsat_stack[unsat_stack_fill_pointer] = c;
				index_in_unsat_stack[c] = unsat_stack_fill_pointer++;
				for (j = 0; j < clause_len; ++j) {
					++score[clause_var[c][j]];
					--score2[clause_var[c][j]];
					cscc[clause_var[c][j]] = 1;
				}
			}
		}
	}
	score[flipvar] = -org_score; // have to update before updating goodvar
	score2[flipvar] = -org_score2;
	conf_change[flipvar] = 0;
	cscc[flipvar] = 0;
	// update goodvar
	// have to be reverse order
	for (i = goodvar_fill_pointer - 1; i >= 0; --i) {
		v = goodvar[i];
		if (score[v] <= 0) {
			// remove it from goodvar
			goodvar[i] = goodvar[--goodvar_fill_pointer];
			already_in_goodvar[v] = 0;
		}
	}
	// update conf_change
	for (i = 0; i < neighbor_num; ++i) {
		v = var_neighbor[flipvar][i];
		conf_change[v] = 1;
		if (score[v] > 0 && !already_in_goodvar[v]) {
			goodvar[goodvar_fill_pointer++] = v;
			already_in_goodvar[v] = 1;
		}
	}
	time_stamp[flipvar] = step; // have to update after updating conf_change
	lastvar = flipvar;
}

/*the following functions are non-algorithmic*/

void print_solution()
{
	cout<<"solutions are:"<<endl;
	for (int v = 1; v <= num_vars; ++v)
		cout << cur_soln[v];
	cout << endl;
	/*
	int    i;
	cout<<"v ";
    	for (i = 1; i <= num_vars; ++i) {
		if(cur_soln[i]==0) cout<<"-";
		cout<<i;
		
		if(i%10==0) 
		{
			cout<<endl;
			cout<<"v ";
		}
		else cout<<' ';
     	}
     	cout<<"0"<<endl;
	*/
}

int verify_sol()
{
	int	c, i; 
	int	sc;
	
	for (c = 1; c <= num_clauses; ++c) {
		sc = 0;
		for (i = 0; i < clause_len; ++i) {
			if (cur_soln[clause_var[c][i]])
				++sc;
		}
		if (sc == 0 || sc == clause_len) {
			printf("Something is wrong!\n");
			printf("the %d clause is unsat", c);
			return 0;
		}
	}
	printf("verify successful!\n");
	return 1;
}
