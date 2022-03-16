void sendInt(int number, int socket);
int recvInt(int socket);

void sendDouble(double number, int socket);
double recvDouble(int socket);

void sendString(char *string, int socket);
char *recvString(int socket);

void sendVoid(void *voidPointer, int nbytes, int socket);
void *recvVoid(int socket);

void list_directory(char *path);
// char *read_file(char *path);

unsigned char *read_file(char *path);
void write_file(char *path, unsigned char *content);

void sendFile(char *path, int socket);
char *recvFile(int socket);

void altSendFile(char *path, int socket);
char *altRecvFile(int socket);

void send_files_list(char *path, int socket);
void list_server_files(int socket);

// void *download_file(char *path, int socket, int option);
void *download_file(void *ptr);
void upload_file(char *path, int socket, int option);