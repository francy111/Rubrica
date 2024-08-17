client: client.o connection.o utility.o
	gcc -o ./client client.o utility.o connection.o
	rm *.o

client.o: src/client.c include/utility.h include/connection.h 
	gcc -c src/client.c

utility.o: src/utility.c include/utility.h 
	gcc -c src/utility.c
	
connection.o: src/connection.c include/utility.h include/utility.h
	gcc -c src/connection.c