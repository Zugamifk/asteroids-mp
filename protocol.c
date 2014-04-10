typedef enum asteroid_messagecode {
	AM_NULL,
	AM_OK,
	AM_ACK,
	AM_HI,
	AM_BYE,
	AM_UPDATE
} asteroid_messagecode;

typedef struct asteroid_dgram {
	int id;
	int seq;
	asteroid_messagecode code;
} asteroid_dgram;