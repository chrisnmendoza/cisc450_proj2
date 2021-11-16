/* tcp_ client.c */ 
/* Programmed by Christopher-Neil Mendoza and David Lizotte */
/* 16 November, 2021 */       

#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset, memcpy, and strlen */
#include <netdb.h>          /* for struct hostent and gethostbyname */
#include <sys/socket.h>     /* for socket, connect, send, and recv */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */

#define STRING_SIZE 1024
#define MESSAGE_SIZE 90

//hardcoding the following quantities
const char clientVisitorName[] = "Mendoza-Lizotte"; //size = 16 with null-terminating character
const unsigned short int serverSecretCode = 54321;
const char serverTravelLocation[] = "Long-Island"; //size = 12 with null-terminating character

//unsigned short int == 2 bytes
//unsigned int == 4 bytes
//struct of all messages
struct Message {
   unsigned short int step;
   unsigned short int clientPort;
   unsigned short int serverPort;
   unsigned short int secretCode;
   char text[81];
};

struct Message message;


void messageHton(void) {
   message.step = htons(message.step);
   message.clientPort = htons(message.clientPort);
   message.serverPort = htons(message.serverPort);
   message.secretCode = htons(message.secretCode);
}


void messageNtoh(void) {
   message.step = ntohs(message.step);
   message.clientPort = ntohs(message.clientPort);
   message.serverPort = ntohs(message.serverPort);
   message.secretCode = ntohs(message.secretCode);
}


int main(void) {

   char clientText2[] = "fortnites-atfreddys";
   //existing entry final step
   //2, 25813, abc
   message.step = 0;
   message.clientPort = 25813;
   message.serverPort = 46298;
   message.secretCode = serverSecretCode;
   strncpy(message.text,clientText2,80);

   int sock_client;  /* Socket used by client */

   struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
   struct hostent * server_hp;      /* Structure to store server's IP
                                        address */
   char server_hostname[STRING_SIZE]; /* Server's hostname */
   unsigned short server_port;  /* Port number used by server (remote port) */

   char sentence[STRING_SIZE];  /* send message */
   char modifiedSentence[STRING_SIZE]; /* receive message */
   unsigned int msg_len;  /* length of message */                      
   int bytes_sent, bytes_recd; /* number of bytes sent or received */
  
   /* open a socket */

   if ((sock_client = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
      perror("Client: can't open stream socket");
      exit(1);
   }

   /* Note: there is no need to initialize local client address information 
            unless you want to specify a specific local port
            (in which case, do it the same way as in udpclient.c).
            The local address initialization and binding is done automatically
            when the connect function is called later, if the socket has not
            already been bound. */

   /* initialize server address information */

   printf("Enter hostname of server: ");
   printf("hardcoding localhost for now\n");
   //scanf("%s", server_hostname); PUT BACK IN WHEN NOT HARD CODING
   sprintf(server_hostname,"localhost");//REMOVE WHEN NOT HARDCODING
   if ((server_hp = gethostbyname(server_hostname)) == NULL) {
      perror("Client: invalid server hostname");
      close(sock_client);
      exit(1);
   }

   printf("Enter port number for server: ");
   printf("hardcoding 46298 for now\n");
   //scanf("%hu", &server_port); PUT BACK IN WHEN NOT HARD CODING
   server_port = (unsigned short int)46298;//REMOVE WHEN NOT HARDCODING

   /* Clear server address structure and initialize with server address */
   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   memcpy((char *)&server_addr.sin_addr, server_hp->h_addr,
                                    server_hp->h_length);
   server_addr.sin_port = htons(server_port);

    /* connect to the server */
 		
   if (connect(sock_client, (struct sockaddr *) &server_addr, 
                                    sizeof (server_addr)) < 0) {
      perror("Client: can't connect to server");
      close(sock_client);
      exit(1);
   }
  
   /* user interface */

   printf("Please input a sentence:\n");
   scanf("%s", sentence);
   msg_len = strlen(sentence) + 1;

   /* send message */
   messageHton();
   bytes_sent = send(sock_client, &message, MESSAGE_SIZE, 0);

   /* get response from server */
  
   bytes_recd = recv(sock_client, &message, MESSAGE_SIZE, 0); 
   messageNtoh();
   printf("\nThe response from server is:\n");
   printf("step: %hu \t text: %s\n\n",message.step, message.text);

   /* close the socket */

   close (sock_client);
}
