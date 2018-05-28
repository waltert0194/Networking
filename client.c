/*
  Name:Walter Thompson
  File:client.c
  Class: CPSC 3600 HW4


*/

#include <stdio.h>	/* standard C i/o facilities */
#include <stdlib.h>	/* needed for atoi() */
#include <unistd.h>	/* Unix System Calls */
#include <sys/types.h>	/* system data type definitions */
#include <sys/socket.h> /* socket specific definitions */
#include <netinet/in.h> /* INET constants and stuff */
#include <arpa/inet.h>	/* IP address conversion stuff */
#include <string.h> 

char *message = "Networking Rules !!!\n";


int main( int argc, char **argv ) {

  FILE *file;
  int sk;
  struct sockaddr_in skaddr;
  char buff[100];


  /* first - check to make sure there are 2 command line parameters
     (argc=3 since the program name is argv[0]) 
  */
  if (argc!= 4) {
    printf("Usage: client <server name> <server port> <filename>\n");
    exit(0);
  }


  /*Attempting to open data file*/
  if((file = fopen(argv[3], "r+")) == NULL) {
    printf("Could not open data file.\n");
    exit(1);
  }

     
  /* create a socket 
     IP protocol family (PF_INET) 
     TCP protocol (SOCK_STREAM)
  */
  if ((sk = socket( PF_INET, SOCK_STREAM, 0)) < 0) {
    printf("Problem creating socket\n");
    exit(1);
  }

  /* fill in an address structure that will be used to specify
     the address of the server we want to connect to

     address family is IP  (AF_INET)

     server IP address is found by calling gethostbyname with the
     name of the server (entered on the command line)

     server port number is argv[2] (entered on the command line)
  */
  skaddr.sin_family = AF_INET;

  #ifndef SUN
    if (inet_aton(argv[1],&skaddr.sin_addr)==0) {
      printf("Invalid IP address: %s\n",argv[1]);
      exit(1);
    }
  #else
    /*inet_aton is missing on Solaris - you need to use inet_addr! */
    /*inet_addr is not as nice, the return value is -1 if it fails
      (so we need to assume that is not the right address !)
    */
    skaddr.sin_addr.s_addr = inet_addr(argv[1]);
    if (skaddr.sin_addr.s_addr ==-1) {
      printf("Invalid IP address: %s\n",argv[1]);
      exit(1);
    }
  #endif

  skaddr.sin_port = htons(atoi(argv[2]));


  /* attempt to establish a connection with the server */
  /* will not work if port and address are different than the server */
  if (connect(sk,(struct sockaddr *) &skaddr,sizeof(skaddr)) < 0 ) {
    printf("Problem connecting skt\n");
    exit(1);
  }

  //send filename
  write(sk, argv[3], strlen(argv[3]));

  //get ACK from server
  read(sk, &buff, 1);
  if(buff[0] != (char)0x04){
    printf("Error, ACK not received.\n");
    close(sk);
    exit(1);
  }

  //Aquire timestamped filename
  read(sk, &buff, 100);
  if (buff == argv[3]) {
    printf("Error! The filename was not timestampted.\n");
    close(sk);
    exit(1);
  }


  /*  Send a line
      Wait until transmission character is received
      repeat
  */
  while(fgets(buff, 100, file))
  {
       write(sk, buff ,strlen(buff));
       read(sk, &buff, 1);

       if (buff[0] != (char)0x06) 
       {
            printf("Transmission error\n");
            exit(1);
            close(sk);
       }
  }

  buff[0] = (char) 0x04;
  write(sk, buff, strlen(buff));

  fclose(file);
  close(sk);
  return 0;
}
