#define DATAPIPE_MSGMAXLEN 1024

typedef struct data_pipe 
{
	pthread_mutex_t lock;
	pthread_cond_t in_sig;
	pthread_cond_t out_sig;
	int msg_len;
	void *message;
} data_pipe;

int data_pipe_init( data_pipe *p ) {
	
	pthread_mutex_init( &(p->lock), NULL);
	pthread_cond_init( &(p->in_sig), NULL);
	pthread_cond_init( &(p->out_sig), NULL);
	
	p->msg_len = 0;
	
	if ( (p->message = malloc( DATAPIPE_MSGMAXLEN )) == NULL ) {
		return 0;
	}
	
	return 1;
	
}

void data_pipe_destroy( data_pipe *p ) {
	pthread_mutex_destroy(&(p->lock));
	pthread_cond_destroy(&(p->in_sig));
	pthread_cond_destroy(&(p->out_sig));
	free (p->message);
	free(p);
}

int data_pipe_get (data_pipe *p, void *msg, int *len) {
	pthread_mutex_lock(&(p->lock));
	
	while (p->msg_len == 0) {
		pthread_cond_wait(&(p->in_sig), &(p->lock));
	}
	
	memcpy(msg, p->message, p->msg_len);
	
	if (len != NULL)
		*len = p->msg_len;
	
	memset(p->message, 0, p->msg_len);
	p->msg_len = 0;
	
	pthread_cond_signal(&(p->out_sig));
	
	pthread_mutex_unlock(&(p->lock));
	
	return 1;
}

int data_pipe_put (data_pipe *p, void *msg, int len) {
	pthread_mutex_lock(&(p->lock));
	
	while (p->msg_len > 0) {
		pthread_cond_wait(&(p->out_sig), &(p->lock));
	}
	memcpy(p->message, msg, len);
	p->msg_len = len;
	
	pthread_cond_signal(&(p->in_sig));
	
	pthread_mutex_unlock(&(p->lock));
	
	return 1;
}

int data_pipe_geta (data_pipe *p, void *msg, int *len) {
	pthread_mutex_lock(&(p->lock));
	
	if (p->msg_len == 0) {
		pthread_mutex_unlock(&(p->lock));
		return 0;
	}
	
	memcpy(msg, p->message, p->msg_len);
	if (len != NULL)
		*len = p->msg_len;
	
	memset(p->message, 0, p->msg_len);
	p->msg_len = 0;
	
	pthread_cond_signal(&(p->out_sig));
	
	pthread_mutex_unlock(&(p->lock));
	
	return 1;
}

int data_pipe_puta (data_pipe *p, void *msg, int len) {
	pthread_mutex_lock(&(p->lock));
	
	if (p->msg_len) {
		pthread_mutex_unlock(&(p->lock));
		return 0;
	}
	
	memcpy(p->message, msg, len);
	p->msg_len = len;
	
	pthread_cond_signal(&(p->in_sig));
	
	pthread_mutex_unlock(&(p->lock));
	
	return 1;
}

void data_pipe_sync (data_pipe *p) {
	
	pthread_mutex_lock(&(p->lock));

	if (p->msg_len != 0) {		
		while (p->msg_len != 0) {
			pthread_cond_wait(&(p->out_sig), &(p->lock));
		}
	} else {
		while (p->msg_len == 0) {
			pthread_cond_wait(&(p->in_sig), &(p->lock));
		}
	}
	
	pthread_mutex_unlock(&(p->lock));
}