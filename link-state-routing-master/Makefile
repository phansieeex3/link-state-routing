all: vec ls manager

manager: manager_send.c
	gcc -g -pthread -o manager_send manager_send.c

vec: main.c monitor_neighbors.c pqueue.c data_structure.c
	gcc -g -pthread -o vec_router main.c monitor_neighbors.c pqueue.c data_structure.c

ls: main.c monitor_neighbors.c pqueue.c data_structure.c
	gcc -g -pthread -o ls_router main.c monitor_neighbors.c pqueue.c data_structure.c

.PHONY: clean
clean:
	rm *.o vec_router ls_router
