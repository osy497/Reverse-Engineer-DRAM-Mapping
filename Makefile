map : HP_map.o
	gcc -o map HP_map.o
HP_map.o : HP_map.c
	gcc -c -o HP_map.o HP_map.c

clean :
	rm *.o map
