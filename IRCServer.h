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
	void createRoom(int fd, char * user, char * password, char * args);
	void getAllUsers(int fd, char * user, char * password, char * args);
	void checkAuth(int fd, char * user, char * password, char * args);
	void listRooms(int fd, char * user, char * password, char * args);
	void runServer(int port);
};
/**
	Messages Array
**/
struct MessageContainer {
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
void userlist_print(UserList * list);
void userlist_add(UserList * list, char *username, char *password, int writeToFile);
int userlist_exists(UserList * list, char *username);
int userlist_remove(UserList * list, char *username);
int userlist_number_elements(UserList * list);
int userlist_save(UserList * list, char * file_name);
int userlist_read(UserList * list, char * file_name);
void userlist_clear(UserList *list);
void userlist_sort(UserList *list);

/**
	Room List
**/

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
void roomlist_print(RoomList * list);
void roomlist_add(RoomList * list, char *name);
ListRoomNode* roomlist_get_ith(RoomList * list, int ith);
int roomlist_exists(RoomList * list, char *name);
int roomlist_remove(RoomList * list, int value);
void roomlist_clear(RoomList *list);
#endif
