#define _WIN32_WINNT 0x0501

#define CONNECTION_BUFFER 16
#define MAX_CLIENTS 8

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>

#include <winsock2.h>
#include <sys/types.h>
#include <ws2tcpip.h>

#include <GL/gl.h>

#include "../game/core.h"
#include "gamestate.c"
#include "../load_init.c"
#include "../data_pipe.c"

/*	CLIENT THREAD
 *  Runs a client that connects to the server and exchanges messages with it
 */ 
 
void *session_loop(void *p) {

	char msg[1024];
	int len;
	data_pipe *dp = (data_pipe *)p;
	
	SOCKET sock_client;

	data_pipe_get( dp, &msg, &len);
	sock_client = *(SOCKET *)msg;
	
	len = recv(sock_client, msg, 1024, 0);
	msg[len] = '\0';
	printf("%s\n", msg);	

	//start session

	pthread_exit(NULL);
	return NULL;
}

void *listen_thread(void *p) {
	int quit = 0;
	data_pipe *dp = (data_pipe *)p;
		
	int err;
	char yes = '1';
	char msg[1024];
	int port;
	SOCKET sock_listen;
	SOCKET sock_client;
	struct sockaddr_in sin;
	socklen_t sin_len = sizeof(sin);
	struct sockaddr_storage client_addr;
	socklen_t ca_size;
		
	sock_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock_listen == SOCKET_ERROR) {
		err = -1;
		goto listen_sock_error;
	}
	
	err = setsockopt(sock_listen, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	if (err == -1) goto listen_error;
	
	// get server address info
	data_pipe_get( dp, (void *)&msg, NULL );
	sscanf( msg, "%d", &port);
	
	memset(&sin, 0, sin_len);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = INADDR_ANY;
	
	err = bind(sock_listen, (struct sockaddr*)&sin, sin_len);
	if (err == SOCKET_ERROR) goto listen_error;
	
	err = listen(sock_listen, CONNECTION_BUFFER);
	if (err == SOCKET_ERROR ) goto listen_error;
	
	getsockname(sock_listen, (struct sockaddr *)&sin, &sin_len);
	//printf("listening on port %d\n", ntohs(sin.sin_port));
	
	while (!quit) {
		ca_size = sizeof client_addr;
		sock_client = accept(sock_listen, (struct sockaddr *)&client_addr, &ca_size);
		if (sock_client == SOCKET_ERROR) {
			printf("accept failed\n");
			continue;
		}
		getsockname(sock_client, (struct sockaddr *)&sin, &sin_len);
		printf("connection from %s:%d\n", inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
		data_pipe_put(dp, (void *)&sock_client, sizeof sock_client);
	}

listen_error:
	closesocket(sock_listen);
listen_sock_error:
	data_pipe_put(dp, (void *)&err, sizeof err);

	printf("Server closing\n");
	pthread_exit(NULL);
	return NULL;
}

void *cmd_thread(void *p) {
	int quit = 0;
	char cmdlin[1024];
	data_pipe *dp = (data_pipe *)p;
	
	printf("type \'help' for help, 'q' to quit\n");
	
	while (!quit) {
		printf("> ");
		scanf("%s", cmdlin);
		if (strcmp(cmdlin, "q") == 0) {
			data_pipe_put(dp, (void *)cmdlin, strlen(cmdlin)+1);
			quit = 1;
		}
	}
		
	pthread_exit(NULL);
	return NULL;
}

int main(void) {

	// Winsock data
	WSADATA wsaData;

	// Connection info:
	// address of server and sockets
//	struct sockaddr_in sin;
	SOCKET newclient;

	// info for initializing server:
	// id : server literal name
	// address: ip:port
	struct init_context ic;
	
	// threads for finding new peers and handling clients
	pthread_t cmd_td;
	pthread_t listen_td;
	pthread_t client_td[MAX_CLIENTS];
	
	// data pipes for threads
	data_pipe cmd_pipe;
	data_pipe listen_pipe;
	data_pipe client_pipes[MAX_CLIENTS];

	int numclients = 0;
	int quit = 0;
	char msg[1024];
	int len = 0;
	
	
	// load init file for session info
	memset(&ic, 0, sizeof ic);
	if (!load_init( "server.init", &ic) ) {
		printf("Init file failed to load\n");
		goto srv_loaderror;
	}
	
	// start winsock
    if (WSAStartup(MAKEWORD(2,0), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        goto srv_loaderror;
    }
	
	// initialize listen thread pipe
	if ( data_pipe_init( &listen_pipe ) == 0) {
		goto srv_listenpipeiniterror;
	}
	
	// put server address in pipe and start listen thread
	data_pipe_put( &listen_pipe, ic.listen_port, strlen(ic.listen_port) );
	pthread_create(&listen_td, NULL, listen_thread, (void *)&listen_pipe);
	data_pipe_sync( &listen_pipe );
	
	// initialize cmdline thread pipe
	if ( data_pipe_init( &cmd_pipe ) == 0) {
		goto srv_cmdpipeiniterror;
	}
	// start cmd line thread
	pthread_create(&cmd_td, NULL, cmd_thread, (void *)&cmd_pipe);
	
	// start game
	init();
	
	// main loop
	while(!quit) {
		//poll cmd pipe
		if (data_pipe_geta( &cmd_pipe, (void *)msg, NULL) == 1) {
			printf("cmd: %s\n", msg);
			quit = 1;
		}
		
		// update game state
		update();
		
		// poll listen pipe
		if (data_pipe_geta( &listen_pipe, (void *)msg, &len) == 1) {
			newclient = *((SOCKET *)msg);
			if (newclient < 0) {
				printf("listen thread error: %s\n", gai_strerror(newclient));
			} else {
				printf("listen: %d\n", newclient);
			}
			if (len == sizeof(SOCKET)) {
				if ( data_pipe_init( &client_pipes[numclients] ) != 0) {

					data_pipe_put( &client_pipes[numclients], (void *)&newclient, sizeof newclient );
					pthread_create(&client_td[numclients], NULL, session_loop, (void *)&client_pipes[numclients]);
					data_pipe_sync( &client_pipes[numclients] );
					numclients++;
				
				}
			}
		}

	}
	
	pthread_join( cmd_td, NULL );
	data_pipe_destroy( &cmd_pipe );
srv_cmdpipeiniterror:
	pthread_cancel( listen_td );
	data_pipe_destroy( &listen_pipe );
srv_listenpipeiniterror:
	WSACleanup();
srv_loaderror:
	return 0;
}