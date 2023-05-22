CXX_FLAGS = -Wall -Wextra -fsanitize=address -g -ggdb3 -O3

all: LRU-1 LRU-K

LRU-1: LRU-1/main.c
	gcc $(CXX_FLAGS) -o LRU-1.out LRU-1/main.c


LRU-K-DEMO: LRU-K/obj/main.o-DEMO LRU-K/obj/functions.o-DEMO
	gcc LRU-K/obj/main.o LRU-K/obj/functions.o $(CXX_FLAGS) -o LRU-K.out

LRU-K/obj/functions.o-DEMO: LRU-K/functions.c
	gcc -D DEMO_MODE -c -o LRU-K/obj/functions.o LRU-K/functions.c $(CXX_FLAGS)

LRU-K/obj/main.o-DEMO: LRU-K/main.c
	gcc -D DEMO_MODE -c -o LRU-K/obj/main.o LRU-K/main.c $(CXX_FLAGS)



LRU-K: LRU-K/obj/main.o LRU-K/obj/functions.o
	gcc LRU-K/obj/main.o LRU-K/obj/functions.o $(CXX_FLAGS) -o LRU-K.out

LRU-K/obj/main.o: LRU-K/main.c
	gcc -c -o LRU-K/obj/main.o LRU-K/main.c $(CXX_FLAGS)

LRU-K/obj/functions.o: LRU-K/functions.c
	gcc -c -o LRU-K/obj/functions.o LRU-K/functions.c $(CXX_FLAGS)
