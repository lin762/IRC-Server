#include <time.h>
#include <curses.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "IRCSocket.h"
using namespace std;

char * user;
char * password;
char * host;
char * sport;
int port;

#define MAX_MESSAGES 100
#define MAX_MESSAGE_LEN 300
#define MAX_RESPONSE (20 * 1024)

int lastMessage = 0;

IRCSocket::IRCSocket(char *h, int p){
        IRCSocket soc = open_client_socket(*host, port);
}

int
IRCSocket::open_client_socket(char * host, int port) {
        // Initialize socket address structure
        struct  sockaddr_in socketAddress;

        // Clear sockaddr structure
        memset((char *)&socketAddress,0,sizeof(socketAddress));

        // Set family to Internet 
        socketAddress.sin_family = AF_INET;

        // Set port
        socketAddress.sin_port = htons((u_short)port);

        // Get host table entry for this host
        struct  hostent  *ptrh = gethostbyname(host);
        if ( ptrh == NULL ) {
                perror("gethostbyname");
                exit(1);
        }

        // Copy the host ip address to socket address structure
        memcpy(&socketAddress.sin_addr, ptrh->h_addr, ptrh->h_length);

        // Get TCP transport protocol entry
        struct  protoent *ptrp = getprotobyname("tcp");
        if ( ptrp == NULL ) {
                perror("getprotobyname");
                exit(1);
        }

        // Create a tcp socket
        int sock = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
        if (sock < 0) {
                perror("socket");
                exit(1);
        }

        // Connect the socket to the specified server
        if (connect(sock, (struct sockaddr *)&socketAddress,
                    sizeof(socketAddress)) < 0) {
                perror("connect");
                exit(1);
        }

        return sock;
}

int
IRCSocket::sendCommand(char *host, int port, char *command, char *response){

        int sock = open_client_socket( host, port);

        // Send command
        write(sock, command, strlen(command));
        write(sock, " ", 1);
        write(sock, user, strlen(user));
        write(sock, " ", 1);
        write(sock, password, strlen(password));
        write(sock, " ", 1);
        write(sock, args, strlen(args));
        write(sock, "\r\n",2);

        // Keep reading until connection is closed or MAX_REPONSE
        int n = 0;
        int len = 0;
        while ((n=read(sock, response+len, MAX_RESPONSE - len))>0) {
                len += n;
        }

        //printf("response:%s\n", response);

        close(sock);
}
void
IRCSocket::printUsage(){
        printf("Usage: test-talk-server host port command\n");
        exit(1);
}

bool
IRCSocket::add_user(){
        char response[MAX_RESPONSE];
        sendCommand(host, port, "ADD-USER", user, password, "", response);

        if(!strcmp(response, "OK\r\n")){
                printf("User %s added\n", user);
                return 1;
        }
        return 0;
}

bool
IRCSocket::enter_room() {
        char response[MAX_RESPONSE];
        sendCommand(host, port, "ENTER-ROOM", user, password, "", response);

        if(!strcmp(response, "OK\r\n")){
                printf("User %s entered room\n", user);
                return 1;
        }
        return 0;

bool
IRCSocket::leave_room() {
        char response[MAX_RESPONSE];
        sendCommand(host, port, "LEAVE-ROOM", user, password, "", response);

        if(!strcmp(response, "OK\r\n")){
                printf("User %s left room\n", user);
                return 1;
        }
        return 0;
}

bool
IRCSocket::get_messages() {
        char response[MAX_RESPONSE];
        sendCommand(host, port, "GET-MESSAGES", user, password, "", response);

        if(!strcmp(response, "OK\r\n")){
                printf("Message get\n");
                return 1;
        }
        return 0;
}

bool
IRCSocket::send_message(char * msg) {
        char response[MAX_RESPONSE];
        sendCommand(host, port, "SEND-MESSAGE", user, password, "", response);

        if(!strcmp(response, "OK\r\n")){
                printf("Message sent\n");
                return 1;
        }
        return 0;
}

bool
IRCSocket::print_users_in_room() {
        char response[MAX_RESPONSE];
        sendCommand(host, port, "GET-USERS-IN-ROOM", user, password, "", response);

        if(!strcmp(response, "OK\r\n")){
                printf("All users in room get\n");
                return 1;
        }
        return 0;
}

bool
IRCSocket::print_users() {
        char response[MAX_RESPONSE];
        sendCommand(host, port, "GET-ALL-USERS", user, password, "", response);

        if(!strcmp(response, "OK\r\n")){
                printf("All users get\n");
                return 1;
        }
        return 0;
}

void 
IRCSocket::printPrompt() {
        printf("talk> ");
        fflush(stdout);
}

void 
IRCSocket::printHelp() {
        printf("Commands:\n");
        printf(" -who   - Gets users in room\n");
        printf(" -users - Prints all registered users\n");
        printf(" -help  - Prints this help\n");
        printf(" -quit  - Leaves the room\n");
        printf("Anything that does not start with \"-\" will be a message to the chat room\n");
}

void * 
IRCSocket::getMessagesThread(void * arg) {
        // This code will be executed simultaneously with main()
        // Get messages to get last message number. Discard the initial Messages

        while (1) {
                // Get messages after last message number received.

                // Print messages

                // Sleep for ten seconds
                usleep(2*1000*1000);
        }
}

void 
IRCSocket::startGetMessageThread()
{
        pthread_create(NULL, NULL, getMessagesThread, NULL);
}

