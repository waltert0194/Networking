# Makefile for simple tcp client and server

# on solaris we need to explicitly include the socket and name server
# libraries, this because sockets are not supported by the O.S. directly,
# but are a library that calls the native (XTI) network API. 
# Leave this commented out unless you are on a Sun running Solaris

LIBS= -lsocket -lnsl

# on solaris we also need to avoid inet_aton, this flag will take care of it
CFLAG= -DSUN


# This rule describes how to build a .o file from a .c file
# -Wall tells gcc to print all warnings!

.c.o:	gcc  -c -Wall ${CFLAG} $<



# by default Make will build the first target - so here we define the
# first target "all", which depends on targets "client" and "server"

all: server client

# Here are the rules that tell make how to build the executables
# named "client" and "server". If LIBS is not defined, including it
# does no harm!

server: server.o 
	gcc -o server server.o -lpthread

client: client.o 
	gcc -o client client.o 

clean: 
	rm *.o server client



