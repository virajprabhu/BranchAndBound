# temporary make file. will be improved

all:
	gcc -o bnb -fopenmp bnb_openmp_final.c

run:
	./bnb

debug:
	lldb-3.4 bnb

gdebug:
	gdb bnb