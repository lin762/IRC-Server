
const char * usage =
"                                                               \n"
"IRCServer:                                                   \n"
"                                                               \n"
"Simple server program used to communicate multiple users       \n"
"                                                               \n"
"To use it in one window type:                                  \n"
"                                                               \n"
"   IRCServer <port>                                          \n"
"                                                               \n"
"Where 1024 < port < 65536.                                     \n"
"                                                               \n"
"In another window type:                                        \n"
"                                                               \n"
"   telnet <host> <port>                                        \n"
"                                                               \n"
"where <host> is the name of the machine where talk-server      \n"
"is running. <port> is the port number you used when you run    \n"
"daytime-server.                                                \n"
"                                                               \n";

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "IRCServer.h"

int QueueLength = 5;

//test

void userlist_init(UserList * list)
{
	list->head = NULL;
}

void userlist_add(UserList * list, char *user, char *password, int writeToFile) {
	// Create new node
	ListUserNode * n = (ListUserNode *) malloc(sizeof(ListUserNode));
	n->username = strdup(user);
	n->password = strdup(password);
	//printf("added user: %s with password: %s\n", n->username, n->password);
	// Add at the beginning of the list
	n->next = list->head;
	list->head = n;

	if(writeToFile) {
		FILE *userFile;
		userFile = fopen("password.txt", "w");
		ListUserNode *e;
		e = list->head;
		while(e != NULL) {
			fprintf(userFile, "%s %s\n", e->username, e->password);
			//printf("password read: %s\n", e->password);
			e = e->next;
		}
		fclose(userFile);
	}
	userlist_sort(list);
}

int userlist_exists(UserList * list, char *user) {
	ListUserNode * e;
	e = list->head;
	while(e != NULL) {
		//printf("%s | %s", e->username, user);
		if(strcmp(e->username, user) == 0) {
			return 1;
		}
		e = e->next;
	}
	return 0;
}

int userlist_remove(UserList * list, char *username) {
	ListUserNode * e;
	e = list->head;
	if(strcmp(e->username, username) == 0) {
		list->head = e->next;
		return 1;
	}
	while(e->next != NULL) {
		if(strcmp(e->next->username, username) == 0) {
			e->next = e->next->next;
			return 1;
		}
		e = e->next;
	}
	return 0;
}

void userlist_sort(UserList * list) {
	ListUserNode *first;
	ListUserNode *second;
	int temp, flag;
	first = list->head;
	flag = 1;
	second = first->next;
	while(flag == 1) {
		//llist_print(list);
		flag = 0;
		first = list->head;
		second = first->next;
		while(second != NULL) {
			//printf("%d", first->value);
			if(strcmp(first->username, second->username) > 0) {
				char *temp_username;
				char *temp_password;

				temp = first->value;
				first->value = second->value;
				second->value = temp;

				temp_username = strdup(first->username);
				first->username = strdup(second->username);
				second->username = strdup(temp_username);

				temp_password = strdup(first->password);
				first->password = strdup(second->password);
				second->password = strdup(temp_password);

				first = second->next;
				flag = 1;
				break;
			}
			else {
				first = second;
				second = second->next;
			}
		}
	}
}

void roomlist_init(RoomList * list)
{
	list->head = NULL;
}

void roomlist_add(RoomList * list, char *name) {
	ListRoomNode * n = (ListRoomNode *) malloc(sizeof(ListRoomNode));
	UserList *ulist = (UserList*)malloc(sizeof(UserList));
	userlist_init(ulist);
	n->messageCounter = 0;
	n->name = name;
	n->users_in_room = ulist;
	
	n->next = list->head;
	list->head = n;
}

int roomlist_exists(RoomList * list, char *name) {
	ListRoomNode * e;
	e = list->head;
	while(e != NULL) {
		if(strcmp(e->name, name) == 0) {
			return 1;
		}
		e = e->next;
	}
	return 0;
}

int
IRCServer::open_server_socket(int port) {

	// Set the IP address and port for this server
	struct sockaddr_in serverIPAddress; 
	memset( &serverIPAddress, 0, sizeof(serverIPAddress) );
	serverIPAddress.sin_family = AF_INET;
	serverIPAddress.sin_addr.s_addr = INADDR_ANY;
	serverIPAddress.sin_port = htons((u_short) port);
  
	// Allocate a socket
	int masterSocket =  socket(PF_INET, SOCK_STREAM, 0);
	if ( masterSocket < 0) {
		perror("socket");
		exit( -1 );
	}

	// Set socket options to reuse port. Otherwise we will
	// have to wait about 2 minutes before reusing the sae port number
	int optval = 1; 
	int err = setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR, 
			     (char *) &optval, sizeof( int ) );
	
	// Bind the socket to the IP address and port
	int error = bind( masterSocket,
			  (struct sockaddr *)&serverIPAddress,
			  sizeof(serverIPAddress) );
	if ( error ) {
		perror("bind");
		exit( -1 );
	}
	
	// Put socket in listening mode and set the 
	// size of the queue of unprocessed connections
	error = listen( masterSocket, QueueLength);
	if ( error ) {
		perror("listen");
		exit( -1 );
	}

	return masterSocket;
}

void
IRCServer::runServer(int port)
{
	int masterSocket = open_server_socket(port);

	initialize();
	
	while ( 1 ) {
		
		// Accept incoming connections
		struct sockaddr_in clientIPAddress;
		int alen = sizeof( clientIPAddress );
		int slaveSocket = accept( masterSocket,
					  (struct sockaddr *)&clientIPAddress,
					  (socklen_t*)&alen);
		
		if ( slaveSocket < 0 ) {
			perror( "accept" );
			exit( -1 );
		}
		
		// Process request.
		processRequest( slaveSocket );		
	}
}

int
main( int argc, char ** argv )
{
	// Print usage if not enough arguments
	if ( argc < 2 ) {
		fprintf( stderr, "%s", usage );
		exit( -1 );
	}
	
	// Get the port from the arguments
	int port = atoi( argv[1] );

	IRCServer ircServer;

	// It will never return
	ircServer.runServer(port);
	
}

//
// Commands:
//   Commands are started y the client.
//
//   Request: ADD-USER <USER> <PASSWD>\r\n
//   Answer: OK\r\n or DENIED\r\n
//
//   REQUEST: GET-ALL-USERS <USER> <PASSWD>\r\n
//   Answer: USER1\r\n
//            USER2\r\n
//            ...
//            \r\n
//
//   REQUEST: CREATE-ROOM <USER> <PASSWD> <ROOM>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: LIST-ROOMS <USER> <PASSWD>\r\n
//   Answer: room1\r\n
//           room2\r\n
//           ...
//           \r\n
//
//   Request: ENTER-ROOM <USER> <PASSWD> <ROOM>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: LEAVE-ROOM <USER> <PASSWD>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: SEND-MESSAGE <USER> <PASSWD> <MESSAGE> <ROOM>\n
//   Answer: OK\n or DENIED\n
//
//   Request: GET-MESSAGES <USER> <PASSWD> <LAST-MESSAGE-NUM> <ROOM>\r\n
//   Answer: MSGNUM1 USER1 MESSAGE1\r\n
//           MSGNUM2 USER2 MESSAGE2\r\n
//           MSGNUM3 USER2 MESSAGE2\r\n
//           ...\r\n
//           \r\n
//
//    REQUEST: GET-USERS-IN-ROOM <USER> <PASSWD> <ROOM>\r\n
//    Answer: USER1\r\n
//            USER2\r\n
//            ...
//            \r\n
//

void
IRCServer::processRequest( int fd )
{
	// Buffer used to store the comand received from the client
	const int MaxCommandLine = 1024;
	char commandLine[ MaxCommandLine + 1 ];
	int commandLineLength = 0;
	int n;
	
	// Currently character read
	unsigned char prevChar = 0;
	unsigned char newChar = 0;
	
	//
	// The client should send COMMAND-LINE\n
	// Read the name of the client character by character until a
	// \n is found.
	//

	// Read character by character until a \n is found or the command string is full.
	while ( commandLineLength < MaxCommandLine &&
		read( fd, &newChar, 1) > 0 ) {

		if (newChar == '\n' && prevChar == '\r') {
			break;
		}
		
		commandLine[ commandLineLength ] = newChar;
		commandLineLength++;

		prevChar = newChar;
	}
	
	// Add null character at the end of the string
	// Eliminate last \r
	commandLineLength--;
        commandLine[ commandLineLength ] = 0;

	printf("RECEIVED: %s\n", commandLine);

	printf("The commandLine has the following format:\n");
	printf("COMMAND <user> <password> <arguments>. See below.\n");
	printf("You need to separate the commandLine into those components\n");
	printf("For now, command, user, and password are hardwired.\n");

	char * command = "ADD-USER";
	char * user = "peter";
	char * password = "spider";
	char * args = "";

	printf("command=%s\n", command);
	printf("user=%s\n", user);
	printf( "password=%s\n", password );
	printf("args=%s\n", args);

	if (!strcmp(command, "ADD-USER")) {
		addUser(fd, user, password, args);
	}
	else if (!strcmp(command, "ENTER-ROOM")) {
		enterRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "LEAVE-ROOM")) {
		leaveRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "SEND-MESSAGE")) {
		sendMessage(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-MESSAGES")) {
		getMessages(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-USERS-IN-ROOM")) {
		getUsersInRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-ALL-USERS")) {
		getAllUsers(fd, user, password, args);
	}
	else {
		const char * msg =  "UNKNOWN COMMAND\r\n";
		write(fd, msg, strlen(msg));
	}

	// Send OK answer
	//const char * msg =  "OK\n";
	//write(fd, msg, strlen(msg));

	close(fd);	
}

UserList *userList;
RoomList *roomList;
int maxMessages = 100;

void
IRCServer::initialize()
{
	userList = (UserList*)malloc(sizeof(UserList));
	initUserList(userList);

	roomList = (RoomList*)malloc(sizeof(RoomList));
	initRoomList(roomList);
	
	FILE*fd = fopen("password.txt", "r");
	char fileuser[100];
	char filepassword[100];
	if(fd == NULL){
		return;
	}
	while(fscanf(fd, "%s %s\n", fileuser, filepassword) == 2){
		addUserList(userList, fileuser, filepassword,1);
	}
	fclose(fd);

}

bool
IRCServer::checkPassword(int fd, char * user, char * password) {
	// Here check the password
	UserNode *n;
	n = userList -> head;
	if(strcmp(n -> username, user) == 0){
		if(strcmp(n -> password, password) == 0){
			return true;
		}
	}else{
		return false;
	}	
	while(n != NULL){
		if(strcmp(n -> username, user) == 0){
			if(strcmp(n -> password, password) == 0){
				return true;
			}else{
				return false;
			}
		}
		n = n -> next;
	}
	return true;
}

void
IRCServer::addUser(int fd, char * user, char * password, char * args)
{
	// Here add a new user. For now always return OK.
	if(userExists(userList,user) == 0){
		addUserList(userList, user, password, 1);
		const char * msg =  "OK\r\n";
		write(fd, msg, strlen(msg));
	}else{
		const char *msg = "DENIED\r\n";
		write(fd, msg, strlen(msg));
	}
	return;		
}

void
IRCServer::enterRoom(int fd, char * user, char * password, char * args)
{
	if(userExists(userList, user) == 1){
		if(checkPassword(fd, user, password) == 1){
			if(roomExists(roomList, args) == 1){
				RoomNode *r;
				r = roomList -> head;
				while(r != NULL){
					if(strcmp(r -> name, args) == 0){
						break;
					}
					r = r -> next;
				}
				char empty[1] = "";
				if(userExists(r -> users, user) == 0){
					addUserList(r -> users, user, empty, 0);
				}
				const char *msg = "OK\r\n";
				write(fd, msg, strlen(msg));
			}
			else{
				const char *msg = "ERROR (No room)\r\n";
				write(fd,msg,strlen(msg));
			}
		}else{
			const char *msg = "ERROR (Wrong password)\r\n";	
			write(fd, msg, strlen(msg));
		}
	}else{
		const char *msg = "ERROR (Wrong password)\r\n";
		write(fd, msg, strlen(msg));
	}
}

void
IRCServer::leaveRoom(int fd, char * user, char * password, char * args)
{
	if(userExists(userList, user) == 1){
		if(checkPassword(fd, user, password) == 1){
			if(roomExists(roomList, args) ==  1){
				int user_entered_room = 0;
				RoomNode *n;
				n = roomList -> head;
				while(n != NULL){
					if(strcmp(n -> name, args) == 0){
						break;
					}
					n = n -> next;
				}
				UserNode *u;
				u = n -> users -> head;
				const char *msg;
				while(u != NULL){
					if(strcmp(u -> username, user) == 0){
						user_entered_room = 1;
						break;
					}
					u = u -> next;
				}
				if(user_entered_room == 1){
					if(userRemove(n -> users, user) == 1){
						const char *msg = "OK\r\n";
						write(fd, msg, strlen(msg));
					}else{
						const char *msg = "DENIED\r\n";
						write(fd, msg, strlen(msg));
					}
				}else{
					const char *msg = "ERROR (No user in room)\r\n";	
					write(fd, msg, strlen(msg));
				}
			}else{
				const char *msg = "DENIED\r\n";
				write(fd, msg, strlen(msg));
			}
		}else{
			const char *msg = "ERROR (Wrong password)\r\n";
			write(fd, msg, strlen(msg));
		}
	}else{
		const char *msg = "ERROR (Wrong password)\r\n";
		write(fd, msg, strlen(msg));
	}
}

void IRCServer::sendMessage(int fd, char * user, char * password, char * args) {
	if(userExists(userList, user) == 1) { // If user exists
		if(checkPassword(fd, user, password) == 1) { // If password is correct
			char *splitted;
			char *originalMessage;
			char *roomName;
			int roomNameLength = 0;
			int entireMessageLength = 0;
			int realMessageLength = 0;
			originalMessage = strdup(args);
			entireMessageLength = strlen(args);
			roomName = strtok (args," ");
			printf("%s\n", roomName);
		  	roomNameLength = strlen(roomName);	  	
		  	realMessageLength = entireMessageLength - roomNameLength;
		  	if(roomExists(roomList, roomName) == 1) { 
				RoomNode * n;
				n = roomList -> head;
				while(n != NULL) {
					if(strcmp(n -> name, roomName) == 0) {
						break;
					}
					n = n -> next;
				}
				if(userExists(n -> users, user) == 1) {
					originalMessage += strlen(roomName) + 1;
					if(n -> messageCounter >= maxMessages) {
						n -> m[n->messageCounter % maxMessages].message = strdup(originalMessage);
						n -> m[n->messageCounter % maxMessages].username = strdup(user);
					}
					else {
						n -> m[n->messageCounter].message = strdup(originalMessage);
						n -> m[n->messageCounter].username = strdup(user);
					}
					n -> messageCounter = n -> messageCounter + 1;
					const char *msg = "OK\r\n";
					write(fd, msg, strlen(msg));				
				}
			  	else {
					const char *msg = "ERROR (user not in room)\r\n";
					write(fd, msg, strlen(msg));
				}				
		  	}
		  	else {
				const char *msg = "DENIED\r\n";
				write(fd, msg, strlen(msg));
			}
		}
		else {
			const char *msg = "ERROR (Wrong password)\r\n";
			write(fd, msg, strlen(msg));
		}
	}
	else {
		const char *msg = "ERROR (Wrong password)\r\n";
		write(fd, msg, strlen(msg));
	}	
}

void
IRCServer::getMessages(int fd, char * user, char * password, char * args)
{
	if(userExists(userList, user) == 1) { 
		if(checkPassword(fd, user, password) == 1) { 
			char *splitted;
			char *temp_num;
			int lastMessageNum;
			char *roomName;
			splitted = strtok (args," ");
			temp_num = strdup(splitted);
			lastMessageNum = atoi(temp_num);
			splitted = strtok (NULL, " ");
			roomName = strdup(splitted);
			int count;
			count = 0;
			
			if(roomExists(roomList, roomName) == 1) {
				RoomNode * n;
				n = roomList -> head;
				while(n != NULL) {
					if(strcmp(n -> name, roomName) == 0) {
						break;
					}
					n = n -> next;
				}
				if(userExists(n -> users, user) == 1) {
					int i;
					char *msg;
					if(maxMessages < n -> messageCounter) {
						lastMessageNum = 0;
					}
					if(lastMessageNum >= n -> messageCounter) {
						const char *msg = "NO-NEW-MESSAGES\r\n";
						write(fd, msg, strlen(msg));
						return;
					}
					if(n -> messageCounter >= maxMessages) {
						int begin;
						begin = (n -> messageCounter % maxMessages)+lastMessageNum;
						for(i = begin; i < maxMessages; i++, count++) {
							char buffer[500];
							sprintf(buffer, "%d %s %s\r\n", n -> messageCounter - maxMessages + count + 1, n -> m[i].username, n -> m[i].message);
							const char *msg = buffer;
							printf("%s\n", buffer);
							write(fd, msg, strlen(msg));
						}
						for(i = 0; i < begin; i++) {
							char buffer[500];
							sprintf(buffer, "%d %s %s\r\n", n -> messageCounter - maxMessages + count + i + 1, n -> m[i].username, n -> m[i].message);
							printf("%s\n", buffer);
							const char *msg = buffer;
							write(fd, msg, strlen(msg));
						}					
					}
					else {
						for(i = lastMessageNum; i < n -> messageCounter; i++) {
							char buffer[500];
							sprintf(buffer, "%d %s %s\r\n", i, n -> m[i].username, n -> m[i].message);
							printf("%s\n", buffer);
							const char *msg = buffer;
							write(fd, msg, strlen(msg));
						}
					}
					write(fd, "\r\n", 2);
				}
				else {
					const char *msg = "ERROR (User not in room)\r\n";
					write(fd, msg, strlen(msg));
				}
			}
			else {
				const char *msg = "DENIED\r\n";
				write(fd, msg, strlen(msg));
			}			
		}
		else {
			const char *msg = "ERROR (Wrong password)\r\n";
			write(fd, msg, strlen(msg));
		}
	}
	else {
		const char *msg = "ERROR (Wrong password)\r\n";
		write(fd, msg, strlen(msg));
	}	
}

void
IRCServer::getUsersInRoom(int fd, char * user, char * password, char * args)
{
	if(userExists(userList, user) == 1){
		if(checkPassword(fd, user, password) == 1){
			if(roomExists(roomList, args) == 1){
				RoomNode *n;
				n = roomList -> head;
				while(n != NULL){
					if(strcmp(n -> name, args) == 0){
						break;
					}
					n = n -> next;
				}
				UserNode *u;
				u = n -> users -> head;
				while(u != NULL){
					char buffer[500];
					sprintf(buffer, "%s\r\n", u -> username);
					const char *msg = buffer;
					write(fd, msg, strlen(msg));
					u = u -> next;
				}
				write(fd, "\r\n", 2);
			}
		}else{
			const char *msg = "ERROR (Wrong password)\r\n";
			write(fd, msg, strlen(msg));
		}
	}else{
		const char *msg = "ERROR (Wrong password)\r\n";
		write(fd, msg, strlen(msg));
	}
}

void
IRCServer::getAllUsers(int fd, char * user, char * password, char * args)
{
	if(userExists(userList, user) == 1){
		if(checkPassword(fd, user, password) == 1){
			UserNode *n;
			n = userList -> head;
			if(n != NULL){
				const char *msg;
				while(n != NULL){
					write(fd, n -> username, strlen(n -> username));
					write(fd, "\r\n", 2);
					n = n -> next;
				}
				write(fd, "\r\n", 2);
			}
		}else{
			const char *msg = "ERROR (Wrong password)\r\n";
			write(fd, msg, strlen(msg));
		}
	}else{
		const char *msg = "ERROR (Wrong password)\r\n";
		write(fd, msg, strlen(msg));
	}
}

