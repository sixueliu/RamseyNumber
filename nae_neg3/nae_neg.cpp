#include "nae_neg.h"
#include <sys/times.h>
#include <unistd.h>
struct 	tms start, stop;
int	cutoff_time;
int	s_id;
int	tries, max_flips;
int	noise;

void pick_greedy_cscc_no()
{
	int	i, v;
	int	best_var = goodvar[0];
	for (i = 1; i < goodvar_fill_pointer; ++i) {
		v = goodvar[i];
		if (cscc[v] && !cscc[best_var])
			best_var = v;
		else if (cscc[v] == cscc[best_var]) {
			if (score[v] > score[best_var])
				best_var = v;
			else if (score[v] == score[best_var]) {
				if (time_stamp[v] < time_stamp[best_var])
					best_var = v;
			}
		}
	}
	flip_update(best_var);
}
void pick_greedy_cscc_neg()
{
	int	i, v;
	int	best_var = goodvar[0];
	for (i = 1; i < goodvar_fill_pointer; ++i) {
		v = goodvar[i];
		if (cscc[v] && !cscc[best_var])
			best_var = v;
		else if (cscc[v] == cscc[best_var]) {
			if (score[v] > score[best_var])
				best_var = v;
			else if (score[v] == score[best_var]) {
				if (score2[v] < score2[best_var])
					best_var = v;
				else if (score2[v] == score2[best_var]) {
					if (time_stamp[v] < time_stamp[best_var])
						best_var = v;
				}
			}
		}
	}
	flip_update(best_var);
}
void pick_greedy_no()
{
	int	i, v;
	int	best_var = goodvar[0];
	for (i = 1; i < goodvar_fill_pointer; ++i) {
		v = goodvar[i];
		if (score[v] > score[best_var])
			best_var = v;
		else if (score[v] == score[best_var]) {
			if (time_stamp[v] < time_stamp[best_var])
				best_var = v;
		}
	}
	flip_update(best_var);
}
void pick_greedy_neg()
{
	int	i, v;
	int	best_var = goodvar[0];
	for (i = 1; i < goodvar_fill_pointer; ++i) {
		v = goodvar[i];
		if (score[v] > score[best_var])
			best_var = v;
		else if (score[v] == score[best_var]) {
			if (score2[v] < score2[best_var])
				best_var = v;
			else if (score2[v] == score2[best_var]) {
				if (time_stamp[v] < time_stamp[best_var])
					best_var = v;
			}
		}
	}
	flip_update(best_var);
}
////////////////////////////////////////////////
void pick_random_no()
{
	int	i, v;
	int	best_var = 0;
	int	c = unsat_stack[rand() % unsat_stack_fill_pointer];
	for (i = 0; i < clause_len; ++i) {
		v = clause_var[c][i];
		if (v == lastvar)
			continue;
		if (score[v] > score[best_var])
			best_var = v;
		else if (score[v] == score[best_var]) {
			if (time_stamp[v] < time_stamp[best_var])
				best_var = v;
		}
	}
	flip_update(best_var);
}
void pick_random_neg()
{
	int	i, v;
	int	best_var = 0;
	int	c = unsat_stack[rand() % unsat_stack_fill_pointer];
	for (i = 0; i < clause_len; ++i) {
		v = clause_var[c][i];
		if (v == lastvar)
			continue;
		if (score[v] > score[best_var])
			best_var = v;
		else if (score[v] == score[best_var]) {
			if (score2[v] < score2[best_var])
				best_var = v;
			else if (score2[v] == score2[best_var]) {
				if (time_stamp[v] < time_stamp[best_var])
					best_var = v;
			}
		}
	}
	flip_update(best_var);
}

///////////////////////////////////////////////////////////////////////////////////
void pickandflip()
{
	//greedy
	if (goodvar_fill_pointer > 0) {
		pick_greedy();
	}
	//random
	else {
		if (rand() < noise) {
			int	i, v;
			int	best_var = 0;
			int	c = unsat_stack[rand() % unsat_stack_fill_pointer];
			for (i = 0; i < clause_len; ++i) {
				v = clause_var[c][i];
				if (v == lastvar)
					continue;
				if (time_stamp[v] < time_stamp[best_var])
					best_var = v;
			}
			flip_update(best_var);
		}
		else
			pick_random();
	}
}

void set_fun_par()
{
	//max_flips = 2000000000;
	max_flips = 20000000;
	pick_random = pick_random_no;
	int	x = s_id % 4;
	int	y = s_id / 4;
	if (x == 0) {
		pick_greedy = pick_greedy_no;
	}
	else if (x == 1) {
		pick_greedy = pick_greedy_neg;
	}
	else if (x == 2) {
		pick_greedy = pick_greedy_cscc_no;
	}
	else if (x == 3) {
		pick_greedy = pick_greedy_cscc_neg;
	}
	//double p_noise = y * 0.03 + 0.5;
	double p_noise = y * 0.03 + 0.53;
	noise = RAND_MAX * p_noise;
}

int main(int argc, char* argv[])
{
	int     seed = time(0); 
	double	comp_time;	
	
	times(&start);
	build_instance();
	//if (build_instance(argv[1])==0) return -1;
	
	//if (argc > 2)	
	//	sscanf(argv[2],"%d",&cutoff_time);
	//if (argc > 3)	
	//	sscanf(argv[3],"%d",&seed);
	//if (argc > 4)	
	//	sscanf(argv[4],"%d",&s_id);
    	//cout << "seed:\t" << seed << endl;
	srand(seed);


	for (tries = 0; ; ++tries) {
		s_id = tries % 16;
		set_fun_par();
		init();
		// local search begins!
		for (/*step = 1*/; step < max_flips; ++step) {
			if (unsat_stack_fill_pointer == 0) { //solution found!
				times(&stop);
				comp_time = double(stop.tms_utime - start.tms_utime +stop.tms_stime - start.tms_stime) / sysconf(_SC_CLK_TCK);
				verify_sol();
				printf("%.2lf\n", comp_time);
				printf("Total steps:\t%lld\n", (long long)(max_flips) * tries + step);
				print_solution();
				free_memory();
				return 0;
			}
			/*
			if (step % 1000000 == 0) {
				//cout << unsat_stack_fill_pointer << endl;
				times(&stop);
				comp_time = double(stop.tms_utime - start.tms_utime +stop.tms_stime - start.tms_stime) / sysconf(_SC_CLK_TCK);
				//printf("current time:\t%.2lf\n", comp_time);
				if (comp_time > cutoff_time) {
					cout << -1 << endl;
					free_memory();
					return 0;
				}
			}*/
			pickandflip();
		}
		printf("Tries:\t%d\n", tries + 1);
	}
}

