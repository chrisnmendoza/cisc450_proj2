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
#define DESTINATION_LEN 100 //step number 1 digit, server port number 5 digits, secret code 5 digits, visitor name 80 digits, commas, spaces, EOF, newline
#define BUFF_LEN 84 //visitor name 80 digits, null character, commas, spaces, EOF

//hardcoding the following quantities
const char clientVisitorName[] = "Mendoza-Lizotte"; //size = 16 with null-terminating character
const unsigned short int serverSecretCode = 54321;
const char serverTravelLocation[] = "Long-Island"; //size = 12 with null-terminating character
const unsigned short int clientPort = 12345;

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

struct Destination {
   unsigned short int currentStep;
   unsigned short int currentServerPort;
   unsigned short int secretCode;
   char location[81];
};

struct Message message;
struct Destination destination;


int constructMessage() {
   int returnStep = 1;
   FILE *fp = fopen("./Travel.txt", "r");
   int foundMatch = 0;
   int scanResult;
   message.clientPort = clientPort;
   strncpy(message.text,"*",80);
   while((scanResult = getDestinationData(fp)) != EOF) {
      if(destination.currentServerPort == message.serverPort) {
         printf("found match! Using travel data to configure message\n");
         if(destination.currentStep == 3) {
            printf("already did step 3, ignoring!\n");
            return -1;
         }
         else {
            message.step = (destination.currentStep + 1) % 3;
            if(message.step == 0) {
               strncpy(message.text,clientVisitorName,80);
            }
         }
         message.serverPort = destination.currentServerPort;
         message.secretCode = destination.secretCode;
         return 1;
      }
   }
   //if there is no entry, do a default step 1 approach
   message.step = 1;
   message.serverPort = 0;
   message.secretCode = 0;
   strncpy(message.text,"*",80);
   return 0;
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


int updateDestinations() {
   int returnStep = 1;
   FILE *fp = fopen("./Travel.txt", "r");
   FILE *tempFile = fopen("./tempTravel.txt","w+");
   int foundMatch = 0;
   int scanResult;
   int firstLine = 1;
   while((scanResult = getDestinationData(fp)) != EOF) {
      if(!firstLine) { //puts newline between entries
         fputs("\n",tempFile);
      }
      firstLine = 0;
      if(destination.currentServerPort == message.serverPort) {
         printf("found match!\n");
         foundMatch = 1;
         writeLineToTemp(tempFile, 1);
      }
      else {
         printf("not a match\n");
         writeLineToTemp(tempFile, 0);
      }
   }
   if(!foundMatch) {
      destination.currentStep = message.step;
      destination.currentServerPort = message.serverPort;
      destination.secretCode = message.secretCode;
      strncpy(destination.location,message.text,80);
      printf("no existing entries match, make new entry\n");
      fputs("\n",tempFile);
      writeLineToTemp(tempFile, 0);
   }
   fclose(fp);
   fclose(tempFile);
   system("mv ./tempTravel.txt ./Travel.txt ");
   printf("returnstep: %d\n",returnStep);
   return returnStep;
}


int getDestinationData(FILE *fp) {
   int scanResult;
   char destinationBuffer[BUFF_LEN];
   fscanf(fp,"%s",destinationBuffer);
   printf("fortnite: %s\n",destinationBuffer);
   sscanf(destinationBuffer, "%hu", &(destination.currentStep));
   fscanf(fp,"%s",destinationBuffer);
   sscanf(destinationBuffer, "%hu", &(destination.currentServerPort));
   fscanf(fp,"%s",destinationBuffer);
   sscanf(destinationBuffer, "%hu", &(destination.secretCode));
   scanResult = fscanf(fp,"%s",destinationBuffer);
   strncpy(destination.location,destinationBuffer,BUFF_LEN-1);
   return scanResult;
}


void writeLineToTemp(FILE *dst, int changeStep) {
   char tempFileBuffer[DESTINATION_LEN];
   if(changeStep) { //means found entry match
      sprintf(tempFileBuffer,"%hu, %hu, %hu, %s", message.step, message.serverPort, message.secretCode, message.text);
   }
   else {
      sprintf(tempFileBuffer,"%hu, %hu, %hu, %s", destination.currentStep, destination.currentServerPort, destination.secretCode, destination.location);
   }
   fputs(tempFileBuffer,dst);
}

//TODO: SENDING
/*
void determineSendConfig(int returnStep) {
   switch(returnStep) {
      case 1:
         printf("current step is step 1, go to step 2\n");
         printf("server should send step = 2, client and server port numbers, server secret code\n");
         message.step = 1;
         message.secretCode = 0;
         message.serverPort = SERV_TCP_PORT;
         strncpy(message.text,"*",1);
         break;
      case 2:
         printf("current step is step 2, go to step 3\n");
         printf("server should send step = 3, client and server port numbers, secret code, and server's travel location\n");
         message.step = 2;
         message.serverPort = SERV_TCP_PORT;
         message.secretCode = serverSecretCode;
         strncpy(message.text,"*",1);
         break;
      case 0:
         printf("current step is step 2, go to step 3\n");
         printf("server should send step = 3, client and server port numbers, secret code, and server's travel location\n");
         message.step = 3;
         message.serverPort = SERV_TCP_PORT;
         message.secretCode = serverSecretCode;
         strncpy(message.text,serverTravelLocation,80);
         break;
      default:
         printf("idk a default\n");
         message.step = 1;
         message.secretCode = 0;
         message.serverPort = SERV_TCP_PORT;
         strncpy(message.text,"*",80);
         break;
   }
}*/


int main(void) {

   char clientText2[] = "fortnites-atfreddys";
   //existing entry final step
   //2, 25813, abc
   message.step = 0;
   message.clientPort = clientPort;
   message.serverPort = 46298;
   message.secretCode = serverSecretCode;
   strncpy(message.text,"eek",80);

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
   constructMessage();//hardcoded for own server

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
   printf("step: %hu \t code: %hu \t text: %s\n\n",message.step, message.secretCode, message.text);
   updateDestinations();
   /* close the socket */

   close (sock_client);
}
