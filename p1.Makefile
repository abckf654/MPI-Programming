main: p1.c
	mpicc -o p1 p1.c
	mpiexec -n 8 -ppn 8 ./p1