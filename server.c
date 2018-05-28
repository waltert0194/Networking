/*
  Name:Walter Thompson
  File:server.c
  Class: CPSC 3600 HW4


*/
#include <stdio.h>	/* standard C i/o facilities */
#include <unistd.h>	/* Unix System Calls */
#include <sys/types.h>	/* system data type definitions */
#include <sys/socket.h> /* socket specific definitions */
#include <netinet/in.h> /* INET constants and stuff */
#include <arpa/inet.h>	/* IP address conversion stuff */
#include <time.h> /*For appending time to file name*/
#include <stdlib.h> /*standard library*/
#include <string.h> /*Strings*/
#include <pthread.h> /*PThreads*/


pthread_mutex_t control_mtx  = PTHREAD_MUTEX_INITIALIZER;

int clientHandler (int sd);

static void * threaded(void *input){

  pthread_mutex_lock(&control_mtx);
  int sd = * ((int *) input);
  pthread_mutex_unlock(&control_mtx);

  pthread_detach(pthread_self());

  if(clientHandler(sd) == -1) 
  {
       close(sd);
       pthread_exit((void *) -1);
  }

  close(sd);
  return (NULL);
}


int main (int argc, char**argv) {

  int ld, sd;
  int addrlen;
  struct sockaddr_in skaddr;
  struct sockaddr_in from;
  int portnum = atoi(argv[1]);
  pid_t childpid;
  char buffer[100];
  char filename[100];
  char reverse[100];
  int n;
  pthread_t id;
  
 

  if (argc != 2) {
    printf("Usage: server <server port>\n");
    exit(1);
  }

  /* create a socket 
       IP protocol family (PF_INET) 
       TCP protocol (SOCK_STREAM)
  */
  if ((ld = socket( PF_INET, SOCK_STREAM, 0 )) < 0) {
    perror("Problem creating socket\n");
    exit(1);
  }

  /* establish our address 
     address family is AF_INET
     our IP address is INADDR_ANY (any of our IP addresses)
     the port number is assigned by the kernel 
  */
  skaddr.sin_family = AF_INET;
  skaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  skaddr.sin_port = htons(portnum);

  if (bind(ld, (struct sockaddr *) &skaddr, sizeof(skaddr)) < 0) {
    perror("Problem binding\n");
    exit(0);
  }

  //printing port
  printf("The Server port number is %d\n",ntohs(skaddr.sin_port));

  // put the socket into passive mode (waiting for connections) 
  if (listen(ld, 5) < 0 ) {
    perror("Error calling listen\n");
    exit(1);
  }

  // now process incoming connections forever ...
  for ( ; ; ) 
  {

      printf("Ready for a connection...\n");

      addrlen = sizeof(from);

      if ((sd = accept(ld, (struct sockaddr*) &from, &addrlen)) < 0) 
      {
           perror("Problem with accept call\n");
           exit(1);
      }
      
      pthread_mutex_lock(&control_mtx);
      pthread_create(&id, NULL, &threaded, &sd);
      pthread_mutex_unlock(&control_mtx);
   }
   return 0;
}


int clientHandler (int sd){

           char buffer[100];
           char filename[100];
           char reverse[100];
           int n;

           //Acquire filename
           if(read(sd, filename, 100) == -1) 
           {
                printf("Error! Could not acquire filename from client.\n");
                return -1;
           }
            
           //Timestamp filename
           time_t t;
           struct tm *tmp;
           char localt[100];
           time(&t);
           tmp = localtime(&t);

           strftime(localt, sizeof(localt)-1, "%H:%M:%S%p", tmp);
           localt[49] = 0;
           
           strncat(filename, "-", 99 - sizeof(filename));
           strncat(filename, localt, 99 - sizeof(filename));
           strncat(filename, ".txt\0", 99 - sizeof(filename));

           FILE *file = fopen(filename, "w");

           if (file == NULL) 
           {
               printf("Error! Could not timestamp filename.\n");
               return -1;
           }

           //ACK: filename was read and send timestamped filename
           //     back to client.
           buffer[0] = (char)0x04;
           write(sd, buffer, 1);


           write (sd, filename, strlen(filename));

           //read client message line by line and reverse it
           while ((n = read(sd, buffer, 100)) > 0)
           {
                
                for(int i = 0; i < n; i++)
                {
                     reverse[i] = buffer[n - i - 1];
                     
                }
                if(file != NULL)
                {
                     fwrite(reverse, 1, n, file);
                }

                //ACK: line successfully reversed
                buffer[0] = (char)0x06;
                write(sd, buffer, 1);
           }
           fclose(file);
           return 0;
}



