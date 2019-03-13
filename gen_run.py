import itertools as it
import numpy as np
import scipy.special
import sys

def gen(N, K, H):

	vertex_set = range(1, N + 1)
	edge_set = list(it.combinations(vertex_set, H))
	clique_set = list(it.combinations(vertex_set, K))

	var_num = len(edge_set)

	pos_clause_num = len(clique_set)
	pos_clause_len = int(scipy.special.binom(K, H) + 0.01)
	pos_var_appearance = pos_clause_num * pos_clause_len / var_num

	print var_num, pos_clause_num, pos_clause_len, pos_var_appearance
	
	## map (u, v) to a positive integer as variable index in CNF
	dict_var_id = dict()
	for ee in edge_set:
		dict_var_id[ee] = len(dict_var_id) + 1
		#print uv, dict_var_id[uv]

	neighbor = []
	for i in range(var_num + 1):
		neighbor.append(set())

	# generate all positive clauses	
	pos_var_ap = dict()
	for cs in clique_set:
		clique_edges = list(it.combinations(cs, H))
		clause_var = []
		for clique_edge in clique_edges:
			var_id = dict_var_id[clique_edge]
			print var_id,
			if var_id not in pos_var_ap:
				pos_var_ap[var_id] = 1
			else:
				pos_var_ap[var_id] += 1
			clause_var.append(var_id)
		print ''
		for i in range(len(clause_var) - 1):
			for j in range(i + 1, len(clause_var)):
				neighbor[clause_var[i]].add(clause_var[j])
				neighbor[clause_var[j]].add(clause_var[i])
	## verify the variable appearance
	for v in pos_var_ap:
		assert(pos_var_ap[v] == pos_var_appearance)

	## output neighborhood
	neighbor_num = len(neighbor[1])
	for v in range(1, var_num + 1):
		assert(len(neighbor[v]) == neighbor_num)
		print len(neighbor[v])
		for j in neighbor[v]:
			print j,
		print ''

i = int(sys.argv[1])
gen(i, 6, 5)
