CXX_FLAGS = -Wall -Wextra -fsanitize=address -g -ggdb3

all: LRU-1 LRU-K

LRU-1: LRU-1/main.c
	g++ $(CXX_FLAGS) -o LRU-1.out LRU-1/main.c


LRU-K: LRU-K/obj/main.o LRU-K/obj/functions.o
	g++ LRU-K/obj/main.o LRU-K/obj/functions.o $(CXX_FLAGS) -o LRU-K.out

LRU-K/obj/main.o: LRU-K/main.c
	g++ -c -o LRU-K/obj/main.o LRU-K/main.c $(CXX_FLAGS)

LRU-K/obj/functions.o: LRU-K/functions.c
	g++ -c -o LRU-K/obj/functions.o LRU-K/functions.c	$(CXX_FLAGS)
