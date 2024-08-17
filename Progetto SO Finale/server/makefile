server: server.o utility.o log.o connection.o
	gcc -o ./server server.o utility.o log.o connection.o -lssl -lcrypto
	rm *.o

server.o: src/server.c include/utility.h include/log.h include/connection.h
	gcc -c src/server.c

utility.o: src/utility.c include/utility.h
	gcc -c src/utility.c

log.o: src/log.c include/log.h
	gcc -c src/log.c

connection.o: src/connection.c include/connection.h
	gcc -c src/connection.c