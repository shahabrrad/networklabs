all: audiostreams audiostreamc

audiostreamc: audiostreamc.o socket_utils.o congestion_control.o queue.o concurrency_utils.o ../lib/constants.h
	@echo "Linking files and creating the client executable"
	gcc audiostreamc.o congestion_control.o socket_utils.o queue.o concurrency_utils.o -o audiostreamc.bin -lasound

audiostreams: audiostreams.o socket_utils.o congestion_control.o ../lib/constants.h
	@echo "Linking files and creating the server executable"
	gcc audiostreams.o congestion_control.o socket_utils.o -o audiostreams.bin

audiostreamc.o: audiostreamc.c ../lib/socket_utils.h ../lib/concurrency_utils.h ../lib/queue.h ../lib/constants.h
	@echo "Compiling the client audiostreamc file"
	gcc -c audiostreamc.c -lasound

audiostreams.o: audiostreams.c ../lib/socket_utils.h ../lib/constants.h
	@echo "Compiling the client audiostreams file"
	gcc -c audiostreams.c 

socket_utils.o: ../lib/socket_utils.c ../lib/socket_utils.h
	@echo "Compiling the socket util files"
	gcc -c ../lib/socket_utils.c

queue.o: ../lib/queue.c ../lib/queue.h
	@echo "Compiling the queue files"
	gcc -c ../lib/queue.c

congestion_control.o: ../lib/congestion_control.c ../lib/socket_utils.h
	@echo "Compiling the congestion_control files"
	gcc -c ../lib/congestion_control.c

concurrency_utils.o: ../lib/concurrency_utils.c ../lib/concurrency_utils.h
	@echo "Compiling the concurrency_utils files"
	gcc -c ../lib/concurrency_utils.c


clean:
	@echo "Removing everything but the source files"
	rm audiostreamc.o audiostreams.o socket_utils.o congestion_control.o queue.o concurrency_utils.o  audiostreamc.bin audiostreams.bin
