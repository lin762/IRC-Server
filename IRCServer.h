
#ifndef IRC_SERVER
#define IRC_SERVER

#define PASSWORD_FILE "password.txt"

class IRCServer {
	// Add any variables you need

private:
	int open_server_socket(int port);

public:
	void initialize();
	bool checkPassword(int fd, const char * user, const char * password);
	void processRequest( int socket );
	void addUser(int fd, const char * user, const char * password, const char * args);
	void enterRoom(int fd, const char * user, const char * password, const char * args);
	void leaveRoom(int fd, const char * user, const char * password, const char * args);
	void sendMessage(int fd, const char * user, const char * password, const char * args);
	void getMessages(int fd, const char * user, const char * password, const char * args);
	void getUsersInRoom(int fd, const char * user, const char * password, const char * args);
	void getAllUsers(int fd, const char * user, const char * password, const char * args);
	void runServer(int port);
};

//data structure to store messages
struct MessageArray{
	char *message;
	char *username;
};

typedef struct MessageArray MessageArray;

//data structure for list of users
struct UserNode{
	int value;
	char *username;
	char *password;
	struct UserNode *next;
};

typedef struct UserNode UserNode;

struct UserList{
	UserNode *head;
};

typedef struct UserList UserList;

void initUserList(UserList *list);
void printUserList(UserList *list);
void addUserList(UserList *list, char *username, char *password, int writeFile);
int userExists(UserList *list, char *username);
int userRemove(UserList *list, char *username);
int numOfUsers(UserList *list);
int saveUserList(UserList *list, char *file_name);
void clearUsers(UserList *list);
void sortUserList(UserList *list);

//data structure for list of rooms
struct RoomNode{
	char *name;
	int messageCounter;
	MessageArray m[100];
	UserList *users;
	struct RoomNode *next;
};

typedef struct RoomNode RoomNode;

struct RoomList{
	RoomNode *head;
};

typedef struct RoomList RoomList;

void initRoomList(RoomList *list);
void printRoomList(RoomList *list);
void addRoom(RoomList *list, char *name);
RoomNode *getIthRoom(RoomList *list, int i);
int roomExists(RoomList *list, char *name);
int roomRemove(RoomList *list, char *name);
void clearRoom(RoomList *list);
#endif
