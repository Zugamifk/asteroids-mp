#define INIT_MAXLINE 80

struct init_context {
	char id[INIT_MAXLINE];
	char listen_port[INIT_MAXLINE];
	char send_port[INIT_MAXLINE];
	char send_addr[INIT_MAXLINE];
};

int load_init(const char *filename, struct init_context *ic) {

	char line[INIT_MAXLINE];
	char token[INIT_MAXLINE];
	int ci, cj;
	
	// open file
	FILE *fp = fopen(filename, "r");
	if (!fp) {
		printf("\'%s\' couldn't be opened!\n", filename);
		return 0;
	}

	// parse file
	while(!feof(fp)) {
		
		if (fgets(line, INIT_MAXLINE, fp) == NULL) break;
		
		sscanf(line, "%s", token);

		// if it's a comment, skip it
		if (token[0] == '#') {
			continue;
		} else
		if (strcmp(token, "ID") == 0) {
			for (ci = 0; line[ci] != '\"'; ci++) {}
			for (cj = ci+1; line[cj] != '\"'; cj++) {}
			memcpy(ic->id, &(line[ci+1]), cj-ci-1);
		} else
		if (strcmp(token, "LISTEN") == 0) {
			sscanf(line, "%*s %*s %s", ic->listen_port);
		} else
		if (strcmp(token, "SEND") == 0) {
			sscanf(line, "%*s %*s %s", token);
			for (ci = 0; line[ci] != '='; ci++) {}
			for (cj = 0; line[cj] != ':'; cj++) {}
			memcpy(ic->send_addr, &(line[ci+2]), cj-ci-2);
			strcpy(ic->send_port, &(line[cj+1]));
			ic->send_port[strlen(ic->send_port)-1] = '\0';
		}
		
	}
	
	fclose(fp);
	
	return 1;

}