// constants and global variables
#define pi 3.14159
#define TIMER 16
float theta = 0.0;
#define dtheta 0.016

// Colors of objects
#define LASERCOLOR 0.7411, 0.2156, 0.298
#define ASTEROIDCOLOR 0.4666, 0.3098,  0.2196
#define SHIPCOLOR 0.968, 0.968, 0.6
#define MESSAGECOLOR 1.0, 0.0, 0.0
#define STARCOLOR 1.0, 1.0, 1.0
#define MISSILECOLOR 0.533, 0.0235, 0.0235

#define FIRECOLOR 1.0, 0.509, 0.027 

// Game related constants and global variables
#define MAXSHIPS 8
#define MAXSHOTS 32
int shotIndex = 0;
#define MAXASTEROIDS 16
#define MAXSTARS 100
#define MAXSPEED 200.0
#define MAXMISSILESPEED 5.0
int missileTarget = 0;

object *ships, *myship, *asteroids, *LAZORZ, *shipGibs, **asteroidGibs, *message, *stars,
		missile;
		
				
// Zoom: Converts between world space and screen space
#define ZOOM 25.0
vector toWorld(vector *v) {return scaleVec(v, ZOOM);}
vector fromWorld(vector *v) {return scaleVec(v, 1.0/ZOOM);}
vector localToWorld(vector *v, object *o) {
	vector r = rotateVec(v, o->ang);
	r = addVec(&r, &(o->pos));
	return r;
}

// Object init funcs

// Create the ship (called only once, at start)
void initShip(object *o) {
	object ship;
	vector* v = (vector *)malloc(sizeof(vector)*3);
	ship.verts = v;
	v->x = 0.0; v->y = -1.0; v++;
	v->x = -0.5; v->y = 1.0; v++;
	v->x = 0.5; v->y = 1.0;
	ship.size = 3;
	ship.pos = zeroVec();
	ship.dir = makeVec(0.0, -1.0);;
	ship.ang = 0.0;
	ship.turn = 10.0;
	ship.vel = zeroVec();
	ship.spd = 25.0;
	ship.type = SHIP;
	ship.mode = GL_LINE_LOOP;
	ship.state = SHIP_INACTIVE;
	LAZORZ = (object *)malloc(sizeof(object)*MAXSHOTS);  
	*o = ship;
}

// Create laser shot (called each time a laser is shot)
void initLazor(object *o, object *ship) {
	object laser;
	vector* v = (vector*)malloc(sizeof(vector)*2);
	laser.verts = v;
	v->x = 0.0; v->y = 0.0;v++;
	v->x = 0.0; v->y = -0.5;
	laser.size = 2;
	laser.pos = copyVec(&(ship->pos));
	laser.dir = copyVec(&(ship->dir));
	laser.ang = ship->ang;
	laser.turn = 0.0;
	laser.vel = scaleVec(&(ship->dir), 30.0);
	laser.spd = 50.0;
	laser.type = LAZOR;
	laser.mode = GL_LINE_LOOP;
	laser.state = LAZER_ACTIVE;
	*o = laser;
}

// // Create missile shot (called once in init)
// void initMissile() {
	// object M;
	// vector* v = (vector*)malloc(sizeof(vector)*5);
	// M.verts = v;
	// v->x=0.0;v->y=-0.3;v++;
	// v->x=0.3;v->y=0.0;v++;
	// v->x=-0.3;v->y=0.3;v++;
	// v->x=0.3;v->y=0.3;v++;
	// v->x=-0.3;v->y=0.0;
	// M.size = 5;
	// M.pos = copyVec(&(ship.pos));
	// M.dir = copyVec(&(ship.dir));
	// M.ang = ship.ang;
	// M.turn = 20.0;
	// M.vel = scaleVec(&(ship.dir), 3.0);
	// M.spd = 10.0;
	// M.type = MISSILE;
	// M.mode = GL_LINE_LOOP;
	// M.state = MISSILE_ACTIVE;
	// vector *a = (vector*)malloc(sizeof(vector)*2);
	// M.aux = a;
	// a->x=0.0;a->y=0.3;a++;
	// a->x=0.0;a->y=0.0;
	// missile = M;
// }

// Create gibs (called each time an object is destroyed)
// oa: array of objects to put gibs into
// o: object to create gibs form
// flags: state to begin gibs in
void initGib(object *oa, object *o, int flags) {
	int i;
	for (i=0; i<o->size; i++) {
		object gib;
		vector *gv = (vector*)malloc(sizeof(vector)*2);
		gib.verts = gv;
		
		gv->x = o->verts[i].x; gv->y = o->verts[i].y; gv++;
		if (i==o->size-1) {
			gv->x = o->verts[0].x; gv->y = o->verts[0].y;
		} else {
			gv->x = o->verts[i+1].x; gv->y = o->verts[i+1].y;
		}
		vector mid = makeVec((gib.verts[0].x+gib.verts[1].x)/2.0,
						(gib.verts[0].y+gib.verts[1].y)/2.0);
		
		gib.pos = localToWorld(&mid, o);
		
	//	vector dv = subVec(&(gib.pos), &(o->pos));
		gib.verts[0] = subVec(&mid, &(gib.verts[0]));
		gib.verts[1] = subVec(&mid, &(gib.verts[1]));
		gib.size = 2;
		gib.dir = upVec();
		gib.ang = o->ang;
		gib.turn = 1.0+0.5*cos((float)rand());
		vector vel = makeVec(sin((float)rand()), cos((float)rand()));	
		gib.vel = scaleVec(&vel, 0.2+cos(rand()));
		gib.spd = 2.0;
		gib.type = GIB;
		gib.mode = GL_LINES;
		gib.state = flags;
		gib.lifetime = 0.5;
		oa[i] = gib;
	}	

}

// Create asteroid (called at start and whenever an asteroid is destroyed)
void initAsteroid(object *o, vector *p, float r) {
	object A;
	A.verts = (vector*)malloc(sizeof(vector)*13);
	int i;
	for(i = 0; i < 12; i++) {
		vector vi = makeVec(cos(i*pi/6.0), sin(i*pi/6.0));
		A.verts[i] = scaleVec(&vi, r*(0.75+0.25*cos(rand())));
	}
	A.verts[i] = A.verts[0];
	A.size = 13;
	A.pos = copyVec(p);
	A.dir = upVec();
	A.ang = (float)(rand()%10);
	A.turn = 2.0;
	vector vel = makeVec(sin((float)rand()), cos((float)rand()));	
	A.vel = scaleVec(&vel, 2.0+cos(rand()));
	A.spd = 5.0;
	A.type = ASTEROID;
	A.mode = GL_LINE_LOOP;
	A.state = ASTEROID_ACTIVE;
	A.radius = r;
	*o = A;
}

// Create message (called at start, activated on player death)
void initMessage() {
	message = (object*)malloc(sizeof(object)*5);
	vector** letters = (vector**)malloc(sizeof(vector*)*5);
	letters[0] = (vector*)malloc(sizeof(vector)*7);
	vector* vi = letters[0];
	vi->x = 3.0; vi->y = -5.0; vi++;
	vi->x = 3.0; vi->y = 4.0; vi++;
	vi->x = 1.0; vi->y = 5.0; vi++;	
	vi->x = -1.0; vi->y = 5.0; vi++;	
	vi->x = -3.0; vi->y = 4.0; vi++;
	vi->x = -3.0; vi->y = 4.0; vi++;
	vi->x = -3.0; vi->y = -5.0;
	letters[1] = (vector*)malloc(sizeof(vector)*7);
	vi = letters[1];
	float n = 0.0;
	int i;
	for(i=0;i<7;i++){vi->x=n;vi->y=n;vi++;}
	letters[2] = (vector*)malloc(sizeof(vector)*7);
	vi = letters[2];
	vi->x = 3.0; vi->y = -5.0; vi++;
	vi->x = -1.0; vi->y = -5.0; vi++;
	vi->x = -3.0; vi->y = -3.0; vi++;	
	vi->x = -3.0; vi->y = 3.0; vi++;	
	vi->x = -1.0; vi->y = 5.0; vi++;
	vi->x = 3.0; vi->y = 5.0; vi++;
	vi->x = 3.0; vi->y = -5.0;
	letters[3] = (vector*)malloc(sizeof(vector)*7);
	vi = letters[3];
	vi->x = -3.0; vi->y = -5.0; vi++;
	vi->x = 3.0; vi->y = -5.0; vi++;
	vi->x = 3.0; vi->y = 0.0; vi++;	
	vi->x = -3.0; vi->y = 0.0; vi++;	
	vi->x = 3.0; vi->y = 0.0; vi++;
	vi->x = 3.0; vi->y = 5.0; vi++;
	vi->x = -3.0; vi->y = 5.0;
	letters[4] = (vector*)malloc(sizeof(vector)*7);
	vi = letters[4];
	vi->x = 3.0; vi->y = -5.0; vi++;
	vi->x = -1.0; vi->y = -5.0; vi++;
	vi->x = -3.0; vi->y = -3.0; vi++;	
	vi->x = -3.0; vi->y = 3.0; vi++;	
	vi->x = -1.0; vi->y = 5.0; vi++;
	vi->x = 3.0; vi->y = 5.0; vi++;
	vi->x = 3.0; vi->y = -5.0;

	int li;
	for (li=0; li<5; li++) {
		object L;
		L.verts = letters[li];
		L.size = 7;
		L.pos = makeVec(16.0-8.0*(float)li, 0.0);
		L.dir = upVec();
		L.ang = 0.0;
		L.turn = 0.0;
		L.vel = zeroVec();
		L.spd = 0.0;
		L.type = MESSAGE;
		L.mode = GL_LINE_STRIP;
		L.state = 0;
		*(message+li) = L;
	}
}

// Create star that twinkles occasionally
void initStar(object *o, vector *l) {
	object star;
	vector* v = (vector*)malloc(sizeof(vector)*8);
	star.verts = v;
	int i; for(i=0;i<8;i+=2) {
		*v = makeVec(0.1*cos(pi*(float)i/4.0), 
			0.1*sin(pi*(float)i/4.0));
		v++;
		*v = makeVec(0.1*cos(pi*(float)(i+1)/4.0), 
				0.1*sin(pi*(float)(i+1)/4.0));
		v++;
	}
	star.size = 8;
	star.pos = copyVec(l);
	star.dir = upVec();
	star.ang = 0.0;
	star.turn = 0.0;
	star.vel = zeroVec();
	star.spd = 50.0;
	star.type = STAR;
	star.mode = GL_LINE_LOOP;
	star.state = 0;
	*o = star;
}

// Object definitions
// -----------------------------------------------------------------

// Wraps objects that go outside the screen's bounds
void wrapPosition(object *o) {
	
	// Convert object position to screen space
	vector toCheck = fromWorld(&(o->pos));
	
	// If a laser, deactivate the object
	if (o->type == LAZOR &&
		(toCheck.x > 1.0 || toCheck.x < -1.0 || 
		toCheck.y > 1.0 || toCheck.y < -1.0)) {
			o->state &= !LAZER_ACTIVE;
			return;
	}
	
	// Adjust position
	if (toCheck.x > 1.0) toCheck.x = -1.0;
	else if (toCheck.x < -1.0) toCheck.x = 1.0;
	if (toCheck.y > 1.0) toCheck.y = -1.0;
	else if (toCheck.y < -1.0) toCheck.y = 1.0;
	
	// Convert position back to world space
	o->pos = toWorld(&toCheck);
}

// Rotate object o by angle a
void turnObject(object *o, float a) {
	o->ang += a;
	o->dir = rotateVec(&(o->dir), a);	
}

// Destroys an asteroid
void killAsteroid(object *o) {
	// If a big asteroid, create 2 medium asteroids
	if (o->state & ASTEROID_BIG) {
		initAsteroid(o, &(o->pos), 2.0);
		initAsteroid(o+2, &(o->pos), 2.0);
		o->state &= !ASTEROID_BIG;
		o->state |= ASTEROID_MEDIUM | ASTEROID_ACTIVE;
		(o+2)->state &= !ASTEROID_BIG;
		(o+2)->state |= ASTEROID_MEDIUM | ASTEROID_ACTIVE;
	// If a medium asteroid, create 2 small asteroids
	} else if (o->state & ASTEROID_MEDIUM) {
		initAsteroid(o, &(o->pos), 1.0);
		initAsteroid(o+1, &(o->pos), 1.0);
		o->state &= !ASTEROID_MEDIUM;
		o->state |= ASTEROID_SMALL | ASTEROID_ACTIVE;
		(o+1)->state &= !ASTEROID_MEDIUM;
		(o+1)->state |= ASTEROID_SMALL | ASTEROID_ACTIVE;
	// If a small asteroid, deactivate it
	} else if (o->state & ASTEROID_SMALL) {
		o->state &= !ASTEROID_ACTIVE;
	}
}

// Function checks if a point is inside a polygon using the point-polygon
// collision detection discussed in class
unsigned int pointPolygonCollide(object *o, vector *v) {

	// cols = number of edge collisions
	int cols = 0, i;
	vector *pv = o->verts; 
	vector v1, v2;

	// iterate through all polygon vertices
	for (i = 0; i < o->size-1; i++) {

		v1 = localToWorld(pv+i, o);
		v2 = localToWorld(pv+i+1, o);

		// Ensure v1.y < v.y < v2.y or skip this edge
		if (v->y > v1.y && v->y < v2.y) {
		} else if (v->y < v1.y && v->y > v2.y) {
			v1 = v2; v2 = localToWorld(pv+i, o);
		} else continue;

		// Ensure the edge is to the right of v and if so, increment cols
		if ((v->y - v1.y) / (v2.y - v1.y) * v2.x +
		    (v2.y - v->y) / (v2.y - v1.y) * v1.x > v->x) {
			cols++;
		}
	}

	// If there is an odd number of collisions return true, else return false
	return cols%2;
}

// Collisions
void checkCollisions(void) {

	// Laser->asteroid collisions
	int li;
	for (li=0; li<MAXSHOTS; li++) {
		object *l = LAZORZ+li;
	
		// If shot is inactive, skip
		if (!(l->state & LAZER_ACTIVE)) continue;
		int a;
		// Else check all asteroids for a collision
		for (a = 0; a < MAXASTEROIDS; a++) {
			// Get distance vector from laser to asteroid centre
			vector dv = subVec(&(l->pos), &((asteroids+a)->pos));

			if (asteroids[a].state & ASTEROID_ACTIVE // Asteroid active?
			 && lengthVec(&dv) < asteroids[a].radius //Laser close enough?
			 && pointPolygonCollide(asteroids+a, &(l->pos))) {//Collision?

				// Kill asteroid and deactivate laser shot
				initGib(asteroidGibs[a], asteroids+a, GIB_ACTIVE);
				killAsteroid(asteroids+a);
				l->state &= !LAZER_ACTIVE;
				break;			
			}		
		}
	}
	
	// // Missile->asteroid collisions
	// if (missile.state & MISSILE_ACTIVE) {
		
		// int si;
		// // Check eack vertex of the ship for a collision
		// for (si=0; si<missile.size; si++) {

			// int a;
			// // Get world coordinate of ship vertex
			// vector sv = localToWorld(missile.verts+si, &missile);

			// // Check all asteroids for collision
			// for (a = 0; a < MAXASTEROIDS; a++) {

				// // Skip if asteroid is not active (destroyed)
				// if (!(asteroids[a].state & ASTEROID_ACTIVE)) continue;

				// // Get distance vector
				// vector dv = subVec( &sv, &((asteroids+a)->pos));
				// if (lengthVec(&dv) < asteroids[a].radius //Close enough?
				 // && pointPolygonCollide(asteroids+a, &sv)) {//Collision?

					// // Kill asteroid and ship, display lose message
					// killAsteroid(asteroids+a);
				// //	initGib(shipGibs, &ship, GIB_ACTIVE);
					// initGib(asteroidGibs[a], asteroids+a, GIB_ACTIVE);
				// //	initMessage();
					// missile.state &= !MISSILE_ACTIVE;				
					// break;			
				// }		
			// }
		// }
	// }

	// Ship->asteroid collisions
	int si;
	object *ship;
	for ( si = 0; si < MAXSHIPS; si++ ) {
		ship = ships+si;

		if ( !(ship->state & ( SHIP_DED | SHIP_INACTIVE)) ) {
			
			int si;
			// Check eack vertex of the ship for a collision
			for (si=0; si<ship->size; si++) {

				int a;
				// Get world coordinate of ship vertex
				vector sv = localToWorld(ship->verts+si, ship);

				// Check all asteroids for collision
				for (a = 0; a < MAXASTEROIDS; a++) {

					// Skip if asteroid is not active (destroyed)
					if (!(asteroids[a].state & ASTEROID_ACTIVE)) continue;

					// Get distance vector
					vector dv = subVec( &sv, &((asteroids+a)->pos));
					if (lengthVec(&dv) < asteroids[a].radius //Close enough?
					 && pointPolygonCollide(asteroids+a, &sv)) {//Collision?

						// Kill asteroid and ship, display lose message
						killAsteroid(asteroids+a);
						initGib(shipGibs+si*ship->size, ship, GIB_ACTIVE);
						initGib(asteroidGibs[a], asteroids+a, GIB_ACTIVE);
						initMessage();
						ship->state += SHIP_DED;				
						break;			
					}		
				}
			}
		}
	}
}

// move object o in the direction it's facing
void moveObject(object *o) {

	// If object is ship, adjust for player controls
	if (o->type == SHIP) {

		// Acceleration
		if (o->state & OBJ_ACCELERATING) {	
			vector acc = scaleVec(&(o->dir), o->spd*dtheta); 
			if (lengthVec(&acc) < MAXSPEED) 
				o->vel = addVec(&(o->vel), &acc);
		}

		// Turning
		if (o->state & OBJ_TURNING_LEFT) {
			turnObject(o, o->turn*dtheta);
		} else if (o->state & OBJ_TURNING_RIGHT) {
			turnObject(o, o->turn*-dtheta);
		}
	}
	
	// If object is gib, rotate it
	if (o->type == GIB) turnObject(o, o->turn*dtheta);

	// translate object and wrap its position if necessary
	vector x = scaleVec(&(o->vel), dtheta);
	o->pos = addVec(&(o->pos), &x);
	wrapPosition(o);
}


// Game initialization function
// ==========================================================
void init(void) {

	// init random seed
	unsigned int seed = time(NULL);
	srand(seed);
	
	// Init asteroid object array
	asteroids = (object *)malloc(sizeof(object)*MAXASTEROIDS);
	int i;
	for(i=0;i<MAXASTEROIDS;i++) {

		// Create an asteroid at a random location on screen
		vector p = makeVec(10.0-(float)(rand()%25), 10.0-(float)(rand()%25));
		initAsteroid(asteroids+i, &p, 3.0);

		// If asteroid spawns too close to ship, make it somewhere else
		while(lengthVec(&(asteroids[i].pos)) < 5.0) {
			p = makeVec(10.0-(float)(rand()%25), 10.0-(float)(rand()%25));
			initAsteroid(asteroids+i, &p, 3.0);
		}

		// Activate 4 big asteroids at the beginning
		if (i%4!=0) asteroids[i].state &= !ASTEROID_ACTIVE;
		else asteroids[i].state |= ASTEROID_BIG;
	}

	// Init asteroid gibs (inactive)
	asteroidGibs = (object**)malloc(sizeof(object*)*MAXASTEROIDS);
	int ab;
	for (ab=0; ab<MAXASTEROIDS; ab++) {
		asteroidGibs[ab] = (object*)malloc(sizeof(object)*asteroids[ab].size);
		initGib(asteroidGibs[ab], asteroids+ab, 0);
	}
	
	// init ship
	ships = malloc(sizeof(object)*MAXSHIPS);
	for (i = 0; i < MAXSHIPS; i++) {
		initShip(ships+i);
	}
	myship = ships;
	myship->state = 0;
	
	// init ship stuff
	//initMissile();	

	// init ship gibs (inactive)
	shipGibs = (object*)malloc(sizeof(object)*ships[0].size*MAXSHIPS);	
	for (i=0; i < MAXSHIPS; i++) {
		initGib(shipGibs+i*ships[0].size, ships, 0);
	}
	
	// init stars
	stars = (object*)malloc(sizeof(object)*MAXSTARS);
	int si; for(si=0;si<MAXSTARS; si++) {
		vector v =  makeVec((float)(rand()%50)-25.0, (float)(rand()%50)-25.0);
		initStar(stars+si, &v);
	}

}	

// Game Update function
// ==========================================================
void update() {
	theta += dtheta;

	// Move ship
	int i;
	for (i = 0; i < MAXSHIPS; i++) {
		moveObject(&(ships[i]));
	}

	// Move all laser shots
	int l;
	for (l=0; l < MAXSHOTS; l++) {
		moveObject(LAZORZ+l);
	}

	// // Move missile
	// int mt = 0;
	// // Aquire new target
	// while (!(asteroids[missileTarget].state & ASTEROID_ACTIVE) && mt < MAXASTEROIDS) {
		// missileTarget = (missileTarget+1) % MAXASTEROIDS;
		// mt++;
	// }
	// // Get vector from missile to target
	// vector tt = subVec(&(asteroids[missileTarget].pos), &(missile.pos));
	// tt = normalizeVec(&tt);
	// float angle = angleBetweenVec(&(missile.dir), &tt) - 1.0;
	// if (angle > 0.0001 || angle < -0.0001) {
		// turnObject(&missile, dtheta*missile.turn*angle);
	// }
	// vector acc = scaleVec(&(missile.dir), missile.spd*dtheta);
	// missile.vel = addVec(&(missile.vel), &acc);
	// float spd = lengthVec(&(missile.vel));
	// if (spd > MAXMISSILESPEED) 
		// missile.vel =  scaleVec(&(missile.vel), MAXMISSILESPEED/spd);
	// moveObject(&missile);

	// Move all asteroids
	int a;
	for (a=0; a<MAXASTEROIDS; a++) {
		if (asteroids[a].state & ASTEROID_ACTIVE) moveObject(asteroids+a);
	}

	// Move all ship gibs
	object ship;
	for (i=0; i < MAXSHIPS; i++) {
		ship = ships[i];
		if ((shipGibs+i*ship.size)->state & GIB_ACTIVE) {
			int sb;
			for (sb=0; sb<ship.size; sb++) {
				moveObject(shipGibs+i*ship.size+sb);		
			}
		}
	}
	
	// Move all asteroid gibs
	int aag;
	for (aag=0; aag<MAXASTEROIDS; aag++) {
		int ag;
		for (ag=0; ag<asteroids->size; ag++) {
			if(asteroidGibs[aag][ag].state & GIB_ACTIVE) {
				moveObject(asteroidGibs[aag]+ag);

				// Decrease lifetime by update amount and kill if < 0.0
				asteroidGibs[aag][ag].lifetime -= 
					0.2*dtheta + 0.4*dtheta*cos((float)rand());
				if (asteroidGibs[aag][ag].lifetime < 0.0) {
					asteroidGibs[aag][ag].state -= GIB_ACTIVE;
				}
			}
		}
	}	

	// shoot lasers
	object *shipi;
	for (i = 0; i < MAXSHIPS; i++) {
		shipi = ships+i;
		if (shipi->state & SHIP_SHOOTING && !(shipi->state & (SHIP_DED | SHIP_INACTIVE))) {
			initLazor(LAZORZ+shotIndex, shipi);
			shotIndex = (shotIndex + 1) % MAXSHOTS;
			shipi->state -= SHIP_SHOOTING;
		}
	}

	// // shoot missiles
	// if (ship.state & SHIP_SHOOTING_MISSILE && !(ship.state & SHIP_DED)) {
		// initMissile();
		// missileTarget = rand() % MAXASTEROIDS;
		// ship.state -= SHIP_SHOOTING_MISSILE;
	// }

	// check collisions
	checkCollisions();
}
