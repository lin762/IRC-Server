
#ifndef IRC_SERVER
#define IRC_SERVER

#define PASSWORD_FILE "password.txt"

class IRCServer {
	// Add any variables you need

private:
	int open_server_socket(int port);

public:
	void initialize();
	bool checkPassword(char * user, char * password);
	void processRequest( int socket );
	void addUser(int fd, char * user, char * password, char * args);
	void enterRoom(int fd, char * user, char * password, char * args);
	void leaveRoom(int fd, char * user, char * password, char * args);
	void sendMessage(int fd, char * user, char * password, char * args);
	void getMessages(int fd, char * user, char * password, char * args);
	void getUsersInRoom(int fd, char * user, char * password, char * args);
	void getAllUsers(int fd, char * user, char * password, char * args);
	void runServer(int port);
};
truct MessageContainer {
	char *message;
	char *username;
};

typedef struct MessageContainer MessageContainer;

/**
	User List
**/
struct ListUserNode {
	int value;
	char *username;
	char *password;
	struct ListUserNode * next;
};

typedef struct ListUserNode ListUserNode;

struct UserList {
	ListUserNode * head;
};

typedef struct UserList UserList;

void userlist_init(UserList * list);
void userlist_add(UserList * list, char *username, char *password, int writeToFile);
int userlist_exists(UserList * list, char *username);
int userlist_remove(UserList * list, char *username);
void userlist_sort(UserList *list);

struct ListRoomNode {
	char *name;
	int messageCounter;	
	MessageContainer m[100];
	UserList *users_in_room;
	struct ListRoomNode * next;
};

typedef struct ListRoomNode ListRoomNode;

struct RoomList {
	ListRoomNode * head;
};

typedef struct RoomList RoomList;

void roomlist_init(RoomList * list);
void roomlist_add(RoomList * list, char *name);
void roomlist_exists(ROomList * list, char *name);
#endif
