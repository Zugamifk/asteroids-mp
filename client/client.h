#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501

#include <time.h>
#include <unistd.h>

#include <winsock2.h>
#include <sys/types.h>
#include <ws2tcpip.h>
#include <errno.h>

const char *client_init_filename = "client.init";

#include "../load_init.c"
#include "../data_pipe.c"
#include "client.c"