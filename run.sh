#!/bin/sh


for((i=72;i<100;i++));
do
	python gen_run.py $i | nae_neg3/nae_neg > result/r$i
done
