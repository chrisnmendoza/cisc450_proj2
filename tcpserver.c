/* tcpserver.c */
/* Programmed by Christopher-Neil Mendoza and David Lizotte */
/* 16 November, 2021 */    

#include <ctype.h>          /* for toupper */
#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset */
#include <sys/socket.h>     /* for socket, bind, listen, accept */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */

#define STRING_SIZE 1024   
#define BUFF_LEN 84 //visitor name 80 digits, null character, commas, spaces, EOF
#define ENTRY_LEN 94 //step number 1 digit, client port number 5 digits, visitor name 80 digits, commas, spaces, EOF, newline
/* SERV_TCP_PORT is the port number on which the server listens for
   incoming requests from clients. You should change this to a different
   number to prevent conflicts with others in the class. */
#define MESSAGE_SIZE 90
#define SERV_TCP_PORT 46298

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

struct Visitor {
   unsigned short int currentStep;
   unsigned short int currentClientPort;
   char currentClientName[81];
};

struct Message message; //holds data from received or sent messages
struct Visitor entry; //holds data for an entry in Visitors.txt

int validateInfo(unsigned short int, unsigned short int, unsigned short int);


//updates entries in Visitors.txt
int updateVisitors() {
   int returnStep = 1;
   FILE *fp = fopen("./Visitors.txt", "r");
   FILE *tempFile = fopen("./tempVisitors.txt","w+");
   int foundMatch = 0;
   int scanResult;
   int firstLine = 1;
   while((scanResult = getVisitorData(fp)) != EOF) {
      if(!firstLine) { //puts newline between entries
         fputs("\n",tempFile);
      }
      firstLine = 0;
      if(entry.currentClientPort == message.clientPort) {
         printf("found match!\n");
         foundMatch = 1;
         returnStep = validateInfo(message.step, message.serverPort, message.secretCode);
         writeLineToTemp(tempFile,returnStep);
      }
      else {
         writeLineToTemp(tempFile, entry.currentStep);
      }
   }
   if(!foundMatch) {
      entry.currentStep = message.step;
      entry.currentClientPort = message.clientPort;
      strncpy(entry.currentClientName,message.text,81);
      printf("no existing entries match, make new entry\n");
      fputs("\n",tempFile);
      writeLineToTemp(tempFile, 0);
   }
   //if we just validated the entry going to step 3, returnStep == -3, must turn to regular 3
   if(returnStep < 0) {
      returnStep *= -1;
   }
   fclose(fp);
   fclose(tempFile);
   system("mv ./tempVisitors.txt ./Visitors.txt ");
   printf("returnstep: %d\n",returnStep);
   return returnStep;
}


//puts entry data into global var entry
int getVisitorData(FILE *fp) {
   int scanResult;
   char visitorBuffer[BUFF_LEN];
   fscanf(fp,"%s",visitorBuffer);
   sscanf(visitorBuffer, "%hu", &(entry.currentStep));
   fscanf(fp,"%s",visitorBuffer);
   sscanf(visitorBuffer, "%hu", &(entry.currentClientPort));
   scanResult = fscanf(fp,"%s",visitorBuffer);
   strncpy(entry.currentClientName,visitorBuffer,BUFF_LEN);
   return scanResult;
}


//returns the corresponding step code that the entry should get
int validateInfo(unsigned short int givenStep, unsigned short int givenServerPort, unsigned short int givenSecret) {
   if(givenSecret != 0) {
      if(givenSecret == serverSecretCode && givenServerPort == SERV_TCP_PORT) {
         printf("validated for step 3 **need to include message\n");
         return -3;
      }
      printf("NOT VALID for step 3\n");
   }
   //step 2 attempt
   else if(givenServerPort != 0) {
      if(givenServerPort == SERV_TCP_PORT) {
         printf("validated for step 2\n");
         return 2;
      }
      printf("NOT VALID for step 2\n");
   }
   return 1;
}


//changeStep is the step to write to the file, with exception of 0
void writeLineToTemp(FILE *dst, int changeStep) {
   char tempFileBuffer[ENTRY_LEN];
   int stepToWrite = changeStep;
   int clientPortToWrite = entry.currentClientPort;
   char textToWrite[81];
   strncpy(textToWrite,"*",81);
   switch(changeStep) {
      //case where the entry got the secret code correct, and this is the matching entry
      case -3:
         stepToWrite = 3;
         strncpy(textToWrite,message.text,81);
         printf("in case -3\n");
         break;
      //case where no entry was found -> load in only the client port number
      case 0:
         stepToWrite = 1;
         clientPortToWrite = message.clientPort;
         break;
      //cases where this is not the matching entry or where we're not updating the name
      case 1:
      case 2:
      case 3:
         strncpy(textToWrite,entry.currentClientName,81);
         printf("at case 1, 2, or 3\n");
         break;
      default:
         printf("put a default here\n");
         break;
   }
   sprintf(tempFileBuffer,"%hu, %hu, %s",stepToWrite,clientPortToWrite,textToWrite);
   fputs(tempFileBuffer,dst);
}


//configures what to send in global var message
void determineSendConfig(int returnStep) {
   switch(returnStep) {
      case 1:
         printf("server should send step = 1, client and server port numbers, secret code, and server's travel location\n");
         message.step = 1;
         message.secretCode = 0;
         message.serverPort = SERV_TCP_PORT;
         strncpy(message.text,"*",81);
         break;
      case 2:
         printf("server should send step = 2, client and server port numbers, server secret code\n");
         message.step = 2;
         message.secretCode = serverSecretCode;
         message.serverPort = SERV_TCP_PORT;
         strncpy(message.text,"*",1);
         break;
      case 3:
         printf("server should send step = 3, client and server port numbers, secret code, and server's travel location\n");
         message.step = 3;
         message.serverPort = SERV_TCP_PORT;
         message.secretCode = serverSecretCode;
         strncpy(message.text,serverTravelLocation,81);
         break;
      case 0:
         printf("current step is step 2, go to step 3\n");
         printf("server should send step = 3, client and server port numbers, secret code, and server's travel location\n");
         message.step = 3;
         message.serverPort = SERV_TCP_PORT;
         message.secretCode = serverSecretCode;
         strncpy(message.text,serverTravelLocation,81);
         break;
      default:
         printf("idk a default\n");
         break;
   }
}


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

   int sock_server;  /* Socket on which server listens to clients */
   int sock_connection;  /* Socket on which server exchanges data with client */

   struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
   unsigned int server_addr_len;  /* Length of server address structure */
   unsigned short server_port;  /* Port number used by server (local port) */

   struct sockaddr_in client_addr;  /* Internet address structure that
                                        stores client address */
   unsigned int client_addr_len;  /* Length of client address structure */

   char sentence[STRING_SIZE];  /* receive message */
   char modifiedSentence[STRING_SIZE]; /* send message */
   unsigned int msg_len;  /* length of message */
   int bytes_sent, bytes_recd; /* number of bytes sent or received */
   unsigned int i;  /* temporary loop variable */

   /* open a socket */
   if ((sock_server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
      perror("Server: can't open stream socket");
      exit(1);                                                
   }

   /* initialize server address information */
   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   server_addr.sin_addr.s_addr = htonl (INADDR_ANY);  /* This allows choice of
                                        any host interface, if more than one
                                        are present */ 
   server_port = SERV_TCP_PORT; /* Server will listen on this port */
   server_addr.sin_port = htons(server_port);

   /* bind the socket to the local server port */
   if (bind(sock_server, (struct sockaddr *) &server_addr,
                                    sizeof (server_addr)) < 0) {
      perror("Server: can't bind to local address");
      close(sock_server);
      exit(1);
   }                     

   /* listen for incoming requests from clients */
   if (listen(sock_server, 50) < 0) {    /* 50 is the max number of pending */
      perror("Server: error on listen"); /* requests that will be queued */
      close(sock_server);
      exit(1);
   }
   printf("I am here to listen ... on port %hu\n\n", server_port);
  
   client_addr_len = sizeof (client_addr);

   /* wait for incoming connection requests in an indefinite loop */
   for (;;) {

      sock_connection = accept(sock_server, (struct sockaddr *) &client_addr, 
                                         &client_addr_len);
                     /* The accept function blocks the server until a
                        connection request comes from a client */
      if (sock_connection < 0) {
         perror("Server: accept() error\n"); 
         close(sock_server);
         exit(1);
      }
 
      /* receive the message */
      bytes_recd = recv(sock_connection, &message, MESSAGE_SIZE, 0);
      printf("bytes received: %d\n",bytes_recd);

      if (bytes_recd > 0){
         messageNtoh();
         printf("Received Sentence is:\n");
         printf("step : %hu \t text: %s", message.step, message.text);
         printf("\nwith length %d\n\n", bytes_recd);

         /* prepare the message to send */
         int returnStep = updateVisitors();
         determineSendConfig(returnStep);

         messageHton();

         /* send message */
         bytes_sent = send(sock_connection, &message, MESSAGE_SIZE, 0);
      }

      /* close the socket */
      close(sock_connection);
   } 
}
