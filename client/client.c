/*	CLIENT THREAD
 *  Runs a client that connects to the server and exchanges messages with it
 */ 

void *client_loop(void *p) {

	WSADATA wsaData;

	struct addrinfo addr_hints, *addr_server, *ap; 
	SOCKET sock_server;
	int err;
	
	//data_pipe *pipe = (data_pipe *)p;
	
	struct init_context ic;
	
	// load init file for session info
	memset(&ic, 0, sizeof ic);
	if (!load_init( client_init_filename, &ic) ) {
		fprintf(stderr, "Init file failed to load\n");
		goto clt_loaderror;
	}
	
	// start winsock
    if (WSAStartup(MAKEWORD(2,0), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        goto clt_loaderror;
    }
	
	// get server address info
	memset(&addr_hints, 0, sizeof(struct addrinfo));
	addr_hints.ai_family = AF_INET;
	addr_hints.ai_socktype = SOCK_STREAM;

	printf("server at %s:%s\n", ic.send_addr, ic.send_port);
	
	if ( (err = getaddrinfo(ic.send_addr, ic.send_port, &addr_hints, &addr_server)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
		goto clt_addrinfoerror;
	}
	
	while (1) {
	    for(ap = addr_server; ap != NULL; ap = ap->ai_next) {
			if ((sock_server = socket(ap->ai_family, ap->ai_socktype,
				   ap->ai_protocol)) == -1) {
				continue;
			}

			if (connect(sock_server, ap->ai_addr, ap->ai_addrlen) == -1) {
				printf("attempt failed: %d\n", WSAGetLastError());
				closesocket(sock_server);
				continue;
			}

			break;
		}	
		if ( ap != NULL ) break;
		usleep(500000);
	}
	send( sock_server, (void *)"Hi!", 4, 0);
	
	fprintf(stderr, "Client failed to connect to server.\n");
	closesocket( sock_server );
	freeaddrinfo(addr_server);
clt_addrinfoerror:
	WSACleanup();
clt_loaderror:
	pthread_exit(NULL);
	return NULL;
}