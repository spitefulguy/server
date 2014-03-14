#ifndef SETTINGS_H_
#define SETTINGS_H_

#define MAX_CONNECTIONS 1024

#define ROOT "/home/alex/workspace/static"

#define DEFAULT_PAGE "index.html"

#define SERVER_NAME "myServ\r\n"

#define LOCAL_PORT 80

#define THREADS 1

#define MAX_LISTEN 64
#define MAX_EPOLL_EVENTS 64

//#define CHUNK_SIZE 65536
#define CHUNK_INTERVAL_US 25

#define MAX_REQUEST_LENGTH 1024

const static char STATIC_EXT[] = "html css js jpg jpeg gif txt ico";

const static char METHODS_TO_FORBID[] = "POST PUT PATCH DELETE TRACE LINK UNLINK CONNECT";


#endif
