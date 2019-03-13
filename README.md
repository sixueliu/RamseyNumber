# RamseyNumber
Ramsey Number on Hypergraphs

Please run "./run.sh" to generate a CNF for proving r_k(k+1,k+1)>=N and to solve it by the algorithm.

The file "result71/r71" is a 71-vertex 5-hypergraph with no 6-clique nor 6-independent-set, i.e., a proof for r_5(6,6)>=72. 

It takes about 138 hours to find this graph using the algorithm in our machine (Intel(R) Xeon(R) CPU E5-2680 v4 @ 2.40GHz).

Other helpful statistics:
a. r_2(5,5)>=43: 1500 seconds.
b. r_3(5,4)>=33: 20 seconds.
c. r_4(5,5)>=43: 300 seconds.
