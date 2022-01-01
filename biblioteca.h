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
void recvFile(char *path, int socket);