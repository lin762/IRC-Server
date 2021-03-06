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


void userlist_init(UserList * list)
{
	list->head = NULL;
}

void userlist_print(UserList * list) {
	
	ListUserNode * e;

	if (list->head == NULL) {
		printf("{EMPTY}\n");
		return;
	}

	printf("{");

	e = list->head;
	while (e != NULL) {
		printf("%d", e->value);
		e = e->next;
		if (e!=NULL) {
			printf(", ");
		}
	}
	printf("}\n");
}
void userlist_add(UserList * list, char *user, char *password, int writeToFile) {
	ListUserNode * n = (ListUserNode *) malloc(sizeof(ListUserNode));
	n->username = strdup(user);
	n->password = strdup(password);
	n->next = list->head;
	list->head = n;

	if(writeToFile) {
		FILE *userFile;
		userFile = fopen("password.txt", "w");
		ListUserNode *e;
		e = list->head;
		while(e != NULL) {
			fprintf(userFile, "%s %s\n", e->username, e->password);
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


int userlist_number_elements(UserList * list) {
	ListUserNode * e;
	e = list->head;
	int counter;
	counter = 0;
	while(e != NULL) {
		counter++;
		e = e->next;
	}
	return counter;
}


int userlist_save(UserList * list, char * file_name) {
	ListUserNode *e;
	e = list->head;
	FILE *fptr;
	fptr = fopen(file_name, "w");
	if(fptr == NULL) {
		return 0;
	}
	while(e != NULL) {
		fprintf(fptr, "%d\n",e->value);
	 	e = e->next;
	}
	fclose(fptr);
	return 0;
}
void userlist_clear(UserList *list)
{
	ListUserNode *e = list->head;
	ListUserNode *temp;
	while(e != NULL) {
		temp = e->next;
		free(e);
		e = temp;
	}
	list->head = NULL;
}

void userlist_sort(UserList * list) {
	ListUserNode *first;
	ListUserNode *second;
	int temp, flag;
	first = list->head;
	flag = 1;
	second = first->next;
	while(flag == 1) {
		flag = 0;
		first = list->head;
		second = first->next;
		while(second != NULL) {
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
ListRoomNode* roomlist_get_ith(RoomList * list, int ith) {
	ListRoomNode * e;
	int counter;
	counter = 0;
	e = list->head;
	while(e != NULL) {
		if(counter == ith) {
			return e;
		}
		counter++;
		e = e->next;
	}
}


int QueueLength = 5;

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
	char *commandLine = (char*)malloc(sizeof(char) * (MaxCommandLine + 1));
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

	// printf("The commandLine has the following format:\n");
	// printf("COMMAND <user> <password> <arguments>. See below.\n");
	// printf("You need to separate the commandLine into those components\n");
	// printf("For now, command, user, and password are hardwired.\n");

	char *temp_commandLine = commandLine;
	char *command = (char*)malloc(sizeof(char) * 100);
	char *user = (char*)malloc(sizeof(char) * 100);
	char *password = (char*)malloc(sizeof(char) * 100);
	char *args = (char*)malloc(sizeof(char) * 100);

	command = strtok (commandLine," ");
	user = strtok (NULL, " ");
	password = strtok (NULL, " ");
	args = strtok (NULL, "\n");
	

	if (!strcmp(command, "ADD-USER")) {
		addUser(fd, user, password, args);
	}
	else if (!strcmp(command, "ENTER-ROOM")) {
		enterRoom(fd, user, password, args);
	}
	else if (!strcmp(command, "CREATE-ROOM")) {
		createRoom(fd, user, password, args);
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
	else if (!strcmp(command, "LIST-ROOMS")) {
		listRooms(fd, user, password, args);
	}
	else if (!strcmp(command, "GET-ALL-USERS")) {
		getAllUsers(fd, user, password, args);
	}
	else if (!strcmp(command, "CHECK-AUTH")) {
		checkAuth(fd, user, password, args);
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
UserList *usersList;
RoomList * roomsList;
int maxMessages = 100;
void
IRCServer::initialize()
{
	usersList = (UserList*)malloc(sizeof(UserList));
	userlist_init(usersList);

	roomsList = (RoomList*)malloc(sizeof(RoomList));
	roomlist_init(roomsList);

	FILE *userFile;
	char fileuser[100];
	char filepassword[100];
	userFile = fopen("password.txt", "r");
	if(userFile == NULL) {
		return;
	}
	while(fscanf(userFile, "%s %s\n", fileuser, filepassword) == 2) {
		userlist_add(usersList, fileuser, filepassword, 1);
	}
	fclose(userFile);
}

bool
IRCServer::checkPassword(char * user, char * password) {
	ListUserNode *e;
	e = usersList->head;
	if(strcmp(e->username, user) == 0) {
		if(strcmp(e->password, password) == 0) {
			return 1;
		}
		else {
			return 0;
		}
	}
	while(e != NULL) {
		if(strcmp(e->username, user) == 0) {
			if(strcmp(e->password, password) == 0) {
				return 1;
			}
			else {
				return 0;
			}
		}		
		e = e->next;
	}
}

void
IRCServer::addUser(int fd, char * user, char * password, char * args)
{
	if(userlist_exists(usersList, user) == 0) {
		userlist_add(usersList, user, password, 1);
		//printf("~%d~\n", userlist_number_elements(usersList));
		const char * msg =  "OK\r\n";
		write(fd, msg, strlen(msg));
	}
	else {
		const char * msg =  "DENIED\r\n";
		write(fd, msg, strlen(msg));
	}
	return;		
}

void
IRCServer::checkAuth(int fd, char * user, char * password, char * args)
{
	//printf("%s %s\n", user, password);
	if(userlist_exists(usersList, user) == 1) {
		if(checkPassword(user, password) == 1) {
			const char * msg =  "OK\r\n";
			write(fd, msg, strlen(msg));
		}
		else {
			const char * msg =  "NO\r\n";
			write(fd, msg, strlen(msg));
		}
	}
	else {
		const char * msg =  "NO\r\n";
		write(fd, msg, strlen(msg));
	}
	return;		
}

void IRCServer::enterRoom(int fd, char * user, char * password, char * args) {
	if(userlist_exists(usersList, user) == 1) { // If user exists
		if(checkPassword(user, password) == 1) { // If password is correct
			if(roomlist_exists(roomsList, args) == 1) {
				//printf("room exists!\n");
				ListRoomNode * e;
				e = roomsList->head;
				while(e != NULL) {
					if(strcmp(e->name, args) == 0) {
						break;
					}
					e = e->next;
				}
				char empty[1] = "";
				if(userlist_exists(e->users_in_room, user) == 0) {
					userlist_add(e->users_in_room, user, empty, 0);
				}
				const char *msg = "OK\r\n";
				write(fd, msg, strlen(msg));
			}
			else {
				const char *msg = "ERROR (No room)\r\n";
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

void IRCServer::leaveRoom(int fd, char * user, char * password, char * args) {
	if(userlist_exists(usersList, user) == 1) { // If user exists
		if(checkPassword(user, password) == 1) { // If password is correct
			if(roomlist_exists(roomsList, args) == 1) {
				int user_entered_room = 0;
				//printf("room exists!\n");
				ListRoomNode * e;
				e = roomsList->head;
				while(e != NULL) {
					if(strcmp(e->name, args) == 0) {
						break;
					}
					e = e->next;
				}
				ListUserNode * u;
				u = e->users_in_room->head;
				const char *msg;
				while(u != NULL) {
					if(strcmp(u->username, user) == 0) {
						user_entered_room = 1;
						break;
					}
					u = u->next;	
				}
				if(user_entered_room == 1) {
					if(userlist_remove(e->users_in_room, user) == 1) {
						const char *msg = "OK\r\n";
						write(fd, msg, strlen(msg));
					}
					else {
						const char *msg = "DENIED\r\n";
						write(fd, msg, strlen(msg));					
					}
				}
				else {
					const char *msg = "ERROR (No user in room)\r\n";
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

void IRCServer::sendMessage(int fd, char * user, char * password, char * args) {
	if(userlist_exists(usersList, user) == 1) { // If user exists
		if(checkPassword(user, password) == 1) { // If password is correct
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
		  	// while (splitted != NULL) {
		  	// 	roomName = strdup(splitted);
		   //  	splitted = strtok (NULL, " ");
		  	// }
		  	roomNameLength = strlen(roomName);	  	
		  	realMessageLength = entireMessageLength - roomNameLength;
		  	if(roomlist_exists(roomsList, roomName) == 1) { // room exists
		  		// Get pointer to the room
				ListRoomNode * e;
				e = roomsList->head;
				while(e != NULL) {
					if(strcmp(e->name, roomName) == 0) {
						break;
					}
					e = e->next;
				}
				if(userlist_exists(e->users_in_room, user) == 1) {
					// Clip original message to remove the name of the room (and the extra space)
					originalMessage += strlen(roomName) + 1;
					if(e->messageCounter >= maxMessages) {
						e->m[e->messageCounter % maxMessages].message = strdup(originalMessage);
						e->m[e->messageCounter % maxMessages].username = strdup(user);
						//printf("(%d) %s: %s\n", e->messageCounter, e->m[e->messageCounter % maxMessages].username, e->m[e->messageCounter - maxMessages].message);
					}
					else {
						// Set the username and message in the MessageContainer
						e->m[e->messageCounter].message = strdup(originalMessage);
						e->m[e->messageCounter].username = strdup(user);
						//printf("(~%d) %s: %s\n", e->messageCounter, e->m[e->messageCounter].username, e->m[e->messageCounter].message);
					}
					e->messageCounter = e->messageCounter + 1;
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

void IRCServer::getMessages(int fd, char * user, char * password, char * args) {
	if(userlist_exists(usersList, user) == 1) { // If user exists
		if(checkPassword(user, password) == 1) { // If password is correct
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
			//printf("%d, %s\n", lastMessageNum, roomName);
			
			if(roomlist_exists(roomsList, roomName) == 1) { // room exists
		  		// Get pointer to the room
				ListRoomNode * e;
				e = roomsList->head;
				while(e != NULL) {
					if(strcmp(e->name, roomName) == 0) {
						break;
					}
					e = e->next;
				}
				if(userlist_exists(e->users_in_room, user) == 1) {
					int i;
					char *msg;
					if(maxMessages < e->messageCounter) {
						lastMessageNum = 0;
					}
					if(lastMessageNum >= e->messageCounter) {
						const char *msg = "NO-NEW-MESSAGES\r\n";
						write(fd, msg, strlen(msg));
						return;
					}
					if(e->messageCounter >= maxMessages) {
						//printf("Total Messages: %d, maxMessages: %d\n", e->messageCounter, maxMessages);
						int begin;
						begin = (e->messageCounter % maxMessages)+lastMessageNum;
						for(i = begin + 1; i < maxMessages; i++, count++) {
							char buffer[500];
							sprintf(buffer, "%d %s %s\r\n", e->messageCounter - maxMessages + count + 1, e->m[i].username, e->m[i].message);
							const char *msg = buffer;
							printf("%s\n", buffer);
							write(fd, msg, strlen(msg));
						}
						for(i = 0; i < begin + 1; i++) {
							char buffer[500];
							sprintf(buffer, "%d %s %s\r\n", e->messageCounter - maxMessages + count + i + 1, e->m[i].username, e->m[i].message);
							printf("%s\n", buffer);
							const char *msg = buffer;
							write(fd, msg, strlen(msg));
						}					
					}
					else {
						for(i = lastMessageNum + 1; i < e->messageCounter; i++) {
							char buffer[500];
							sprintf(buffer, "%d %s %s\r\n", i, e->m[i].username, e->m[i].message);
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

void IRCServer::getUsersInRoom(int fd, char * user, char * password, char * args) {
	if(userlist_exists(usersList, user) == 1) { // If user exists
		if(checkPassword(user, password) == 1) { // If password is correct
			if(roomlist_exists(roomsList, args) == 1) {
				// int i = 0;
				//printf("room exists!\n");
				ListRoomNode * e;
				e = roomsList->head;
				while(e != NULL) {
					if(strcmp(e->name, args) == 0) {
						break;
					}
					e = e->next;
				}
				ListUserNode * u;
				u = e->users_in_room->head;
				while(u != NULL) {
					char buffer[500];
					sprintf(buffer, "%s\r\n", u->username);
					const char *msg = buffer;
					write(fd, msg, strlen(msg));
					u = u->next;
					// i++;
				}
				write(fd, "\r\n", 2);
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

void IRCServer::getAllUsers(int fd, char * user, char * password, char * args) {
	if(userlist_exists(usersList, user) == 1) { // If user exists
		//printf("user exists\n");
		if(checkPassword(user, password) == 1) { // If password is correct
			//printf("password is right!\n");
			ListUserNode *e;
			e = usersList->head;
			if(e != NULL) {
				//printf("head is not null!\n");
				const char *msg;
				while(e != NULL) {
					// printf("found user\n");
					// printf("%s\n", e->username);
					write(fd, e->username, strlen(e->username));
					write(fd, "\r\n", 2);
					e = e->next;
				}
				write(fd, "\r\n", 2);
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

void IRCServer::listRooms(int fd, char * user, char * password, char * args) {
	if(userlist_exists(usersList, user) == 1) { // If user exists
		//printf("user exists\n");
		if(checkPassword(user, password) == 1) { // If password is correct
			//printf("password is right!\n");
			ListRoomNode *e;
			e = roomsList->head;
			if(e != NULL) {
				//printf("head is not null!\n");
				const char *msg;
				while(e != NULL) {
					//printf("%s\n", e->username);
					write(fd, e->name, strlen(e->name));
					write(fd, "\r\n", 2);
					e = e->next;
				}
			}
		}
		else {
			const char *msg = "ERROR (Wrong password)\r\n";
			write(fd, msg, strlen(msg));
		}
	}
	else {
		const char *msg = "DENIED\r\n";
		write(fd, msg, strlen(msg));
	}
}

void IRCServer::createRoom(int fd, char * user, char * password, char * args){
	if(userlist_exists(usersList, user) == 1) { // If user exists
		if(checkPassword(user, password) == 1) { // If password is correct
			if(roomlist_exists(roomsList, args) == 0) { // If room does not already exist
				roomlist_add(roomsList, args); // Add args (name of room) to RoomsList
				//printf("~Rooms: %d~\n", roomlist_number_elements(roomsList));
				const char *msg = "OK\r\n";
				write(fd, msg, strlen(msg));
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
		const char *msg = "DENIED\r\n";
		write(fd, msg, strlen(msg));
	}
}
