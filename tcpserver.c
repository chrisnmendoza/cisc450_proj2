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
   char* text;
};

struct Visitor {
   unsigned short int currentStep;
   unsigned short int currentClientPort;
   char currentClientName[80];
};

struct Message message; //holds data from received or sent messages
struct Visitor entry; //holds data for an entry in Visitors.txt

void updateVisitors() {
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
         writeLineToTemp(tempFile, 1);
      }
      else {
         writeLineToTemp(tempFile, 0);
      }
   }
   printf("scan gave value of %d, which is EOF\n",scanResult);
   if(!foundMatch) {
      entry.currentStep = message.step;
      entry.currentClientPort = message.clientPort;
      strncpy(entry.currentClientName,message.text,80);
      printf("no existing entries match, make new entry\n");
      fputs("\n",tempFile);
      writeLineToTemp(tempFile, 0);
   }
   fclose(fp);
   fclose(tempFile);
   system("mv ./tempVisitors.txt ./Visitors.txt ");
}


int getVisitorData(FILE *fp) {
   int scanResult;
   char visitorBuffer[BUFF_LEN];
   fscanf(fp,"%s",visitorBuffer);
   sscanf(visitorBuffer, "%hu", &(entry.currentStep));
   printf("step: %hu\n",entry.currentStep);
   fscanf(fp,"%s",visitorBuffer);
   sscanf(visitorBuffer, "%hu", &(entry.currentClientPort));
   printf("client port: %hu\n",entry.currentClientPort);
   scanResult = fscanf(fp,"%s",visitorBuffer);
   strncpy(entry.currentClientName,visitorBuffer,BUFF_LEN-1);
   printf("client name: %s\n",entry.currentClientName);
   return scanResult;
}


void writeLineToTemp(FILE *dst, int incrementStep) {
   char tempFileBuffer[ENTRY_LEN];
   if(incrementStep) { //means found entry match
      printf("validating info sent\n");
      if((entry.currentStep == 1 && message.serverPort == SERV_TCP_PORT)
         || (entry.currentStep == 2 && message.serverPort == SERV_TCP_PORT && message.secretCode == serverSecretCode)) 
      {
         printf("validated!\n");
         printf("increment step\n");
         sprintf(tempFileBuffer,"%hu, %hu, %s",entry.currentStep + 1,message.clientPort,message.text);
         fputs(tempFileBuffer,dst);
      }
      else {
         printf("INVALID AUTHENTICATION\n");
         sprintf(tempFileBuffer,"%hu, %hu, %s",entry.currentStep,entry.currentClientPort,entry.currentClientName);
         fputs(tempFileBuffer,dst);
      }
   }
   else {
      printf("just copy info as is\n");
      sprintf(tempFileBuffer,"%hu, %hu, %s",entry.currentStep,entry.currentClientPort,entry.currentClientName);
      fputs(tempFileBuffer,dst);
   }
}


void determineSendConfig(void) {
   switch(entry.currentStep) {
      case 1:
         printf("current step is step 1, go to step 2\n");
         printf("server should send step = 2, client and server port numbers, server secret code\n");
         break;
      case 2:
         printf("current step is step 2, go to step 3\n");
         printf("server should send step = 3, client and server port numbers, secret code, and server's travel location\n");
         break;
      default:
         printf("idk a default\n");
   }
}


int main(void) {
   //for testing purposes, hard code what a message WOULD send
   //assume sending first message on the port
   char clientText[] = "*";

   //new entry
   message.step = 1;
   message.clientPort = 19624;
   message.serverPort = 0;
   message.secretCode = 0;
   message.text = clientText;
   updateVisitors();
   
   //existing entry
   message.step = 2;
   message.clientPort = 12345;
   message.serverPort = SERV_TCP_PORT;
   message.secretCode = 0;
   message.text = clientText;
   updateVisitors();

   //existing entry bad secret code
   message.step = 0;
   message.clientPort = 12345;
   message.serverPort = SERV_TCP_PORT;
   message.secretCode = serverSecretCode - 5;
   message.text = clientText;
   updateVisitors();

   char clientText2[] = "fortnites-atfreddys";
   //existing entry final step
   //2, 25813, abc
   message.step = 0;
   message.clientPort = 25813;
   message.serverPort = SERV_TCP_PORT;
   message.secretCode = serverSecretCode;
   message.text = clientText2;
   updateVisitors();

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

      bytes_recd = recv(sock_connection, sentence, STRING_SIZE, 0);

      if (bytes_recd > 0){
         printf("Received Sentence is:\n");
         printf("%s", sentence);
         printf("\nwith length %d\n\n", bytes_recd);

        /* prepare the message to send */

         msg_len = bytes_recd;

         for (i=0; i<msg_len; i++)
            modifiedSentence[i] = toupper (sentence[i]);

         /* send message */
 
         bytes_sent = send(sock_connection, modifiedSentence, msg_len, 0);
      }

      /* close the socket */

      close(sock_connection);
   } 
}
