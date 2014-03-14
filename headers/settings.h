#ifndef SETTINGS_H_
#define SETTINGS_H_


//#define DEFAULT_PAGE "index.html"
//#define ROOT "/home/alex/workspace/static"
//#define LOCAL_PORT 80
//#define THREADS 1
//const static char METHODS_TO_FORBID[] = "POST PUT PATCH DELETE TRACE LINK UNLINK CONNECT";
//#define SEND_RETRY_INTERVAL_USEC 5

#define CONFIG_PATH "server.conf"

#define SERVER_NAME "myServ\r\n"

#define MAX_LISTEN 64

#define MAX_EPOLL_EVENTS 64

#define MAX_REQUEST_LENGTH 1024

const static char STATIC_EXT[] = "html css js jpg jpeg gif txt ico";

#endif
