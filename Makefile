all: vec ls

vec: main.c monitor_neighbors.h
	gcc -pthread -o vec_router main.c monitor_neighbors.h

ls: main.c monitor_neighbors.h
	gcc -pthread -o ls_router main.c monitor_neighbors.h

lss: main.c monitor_neighbors.c monitor_neighbors.c
	gcc -g -pthread -o ls_router main.c monitor_neighbors.c 
	

.PHONY: clean
clean:
	rm *.o vec_router ls_router
