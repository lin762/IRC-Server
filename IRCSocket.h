#ifndef IRCSOCKET_H
#define IRCSOCKET_H

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

class IRCSocket;

class IRCSocket{
public:
        char * user;
        char * password;
        char * host;
        char * sport;
        int port;
        int lastMessage;

        IRCSocket();
        int open_client_socket(char *host, int port);
        int sendCommand(char *host, int port, char *command, char *user, char *password, char *args, char *response);
        void printUsage();
        bool add_user();
        bool enter_room();
        bool leave_room();
        bool get_message();
        bool send_message(char *msg);
        bool print_users_in_room();
        void printPrompt();
        void printHelp();
        void *getMessagesThread(void *arg);
        void startGetMessageThread();


};

#endif //IRCSOCKET_H

