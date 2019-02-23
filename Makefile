filename = *.cpp
output   = sum_of_arrays.out \
		   partial_sum_busy.out \
		   partial_sum_mutex.out \
		   barriers.out \
		   multithreaded_linked_list.out
CC	     = g++
FLAGS    = -Wall -pthread
compile  = $(CC) $< -o $@ $(FLAGS)

all : $(output)

sum_of_arrays.out 		: sum_of_arrays.cpp
							${compile}

partial_sum_busy.out 	: partial_sum_busy.cpp
					   		${compile}

partial_sum_mutex.out 	: partial_sum_mutex.cpp
					   		${compile}

barriers.out 			: barriers.cpp	
							${compile}

multithreaded_linked_list.out : multithreaded_linked_list.cpp
								${compile}

clean 					: 
							rm *.out