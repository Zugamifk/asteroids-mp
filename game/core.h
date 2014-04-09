// ------------------------------
#define OBJ_ACCELERATING 1
#define OBJ_TURNING_LEFT 1<<1
#define OBJ_TURNING_RIGHT 1<<2
#define SHIP_SHOOTING 1<<3
#define SHIP_SHOOTING_MISSILE 1<<4
#define SHIP_DED 1<<5
#define SHIP_INACTIVE 1<<6

#define LAZER_ACTIVE 1

#define ASTEROID_ACTIVE 1
#define ASTEROID_BIG 1<<1
#define ASTEROID_MEDIUM 1<<2
#define ASTEROID_SMALL 1<<3
#define ASTEROID_DED 1<<4

#define GIB_ACTIVE 1

#define MESSAGE_ACTIVE 1

#define MISSILE_ACTIVE 1

#include "vector.c"

// Enum for checking object type
typedef enum objectTypes {
      SHIP,
      ASTEROID,
	LAZOR,
	GIB,
	MESSAGE,
	STAR,
	MISSILE
} oType;

// object struct
typedef struct obj {
        vector dir;
        float ang;
        float turn;
        vector vel;
        float spd;
        vector pos;
        vector* verts;
        int size;
        GLenum mode;
        oType type;
        int state;
	  float radius;
	  float lifetime;
	  vector* aux;
} object;

// Declarations for object creation functions
void initLazor(object*, object*);
void initMissile(void);
void initAsteroid(object*, vector*, float);
void initGib(object*, object*, int);
void initMessage(void);


#include "object.c"
#include "core.c"
