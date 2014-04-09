#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <time.h>

#include <GL/glut.h>
#include <GL/glu.h>

#include <pthread.h>
#include "./main.h"


pthread_t client_thread;
data_pipe client_dp;

// ==========
void drawVec(object *o, vector *d) {
	glBegin(GL_LINES);
	//vector from = fromWorld(&(o->pos));
	glVertex2f(o->pos.x, o->pos.y);
	vector to = fromWorld(d);
to = localToWorld(&to, o);
	glVertex2f(to.x, to.y);
	glEnd();
}

// draw thruster
void drawThrust(vector *o, vector *d, float s, int n) {
	vector wo = fromWorld(o); 
	vector wd = fromWorld(d);
	glColor3f(FIRECOLOR);
	glBegin(GL_LINES);	
	int i; for(i=0;i<n;i++) {
		glVertex2f(wo.x, wo.y);
		vector v = rotateVec(&wd, 0.2*sin((float)rand()));
		v = scaleVec(&v, s);
		v = scaleVec(&v, 0.5 + 0.5*sin((float)rand()));
		v = addVec(&wo, &v);
		glVertex2f(v.x, v.y);
	}
	glEnd();
}

// draw object o
void drawObject(object *o) {
	glBegin(o->mode);
	int i;
	for (i = 0; i < o->size; i++) {
		vector v = o->verts[i];
		v = localToWorld(&v, o);
		v = fromWorld(&v);
		glVertex2f(v.x, v.y);
	}
	glEnd();

	if (o->type == MISSILE) {
		vector to = localToWorld(o->aux, o);
		vector d = scaleVec(&(o->dir), -1.0);
		drawThrust(&to, &d, 3.0, 10);
	}
}


// Timer callback
void timer(int v) {

	// update timer
	glutPostRedisplay();
	update();
	glutTimerFunc(TIMER, timer, v);
}

// Draw callback
void draw(void) {
	
	glClear(GL_COLOR_BUFFER_BIT);
	
	// Draw all stars
	glColor3f(STARCOLOR);
	int si; for(si=0; si<MAXSTARS; si++) {
		drawObject(stars+si);
	} 

	// Draw all ship gibs	
	object ship;
	for (int i=0; i < MAXSHIPS; i++) {
		ship = ships[i];
		if ((shipGibs+i*ship.size)->state & GIB_ACTIVE) {
			glColor3f(SHIPCOLOR);
			int s;
			for (s=0; s<ship.size; s++) {
				drawObject(shipGibs+s+i*ship.size);
			}
		}
	}

	// Draw all asteroid gibs
	int aag;
	for (aag=0; aag<MAXASTEROIDS; aag++) {
		glColor3f(ASTEROIDCOLOR);
		int ag;
		for (ag=0; ag<asteroids->size; ag++) {
			if(asteroidGibs[aag][ag].state & GIB_ACTIVE) {
				drawObject(asteroidGibs[aag]+ag);
			}
		}
	}

	// Draw all asteroids
	int a;
	glColor3f(ASTEROIDCOLOR);
	for (a = 0; a < MAXASTEROIDS; a++) {
		if (asteroids[a].state & ASTEROID_ACTIVE) drawObject(asteroids+a);	
	}

	// Draw all lasers
	int l;
	glColor3f(LASERCOLOR);
	for (l = 0; l < MAXSHOTS; l++) {	
		if ((LAZORZ+l)->state & LAZER_ACTIVE) drawObject(LAZORZ+l);
	}

	// Draw missiles
	if (missile.state & MISSILE_ACTIVE) {	
		glColor3f(MISSILECOLOR);
		drawObject(&missile);
		glColor3f(0.0, 0.0, 1.0);
		drawVec(&missile, &(missile.dir));
	//	drawVec(&missile, &(missile.vel));
	}	

	// Draw ship if alive, else display message
	if (!(myship->state & (SHIP_DED | SHIP_INACTIVE))) {	
		glColor3f(SHIPCOLOR);
		drawObject(myship);
	} else {
		glColor3f(MESSAGECOLOR);
		int mi;
		for (mi=0; mi<5; mi++)	{
		drawObject(message+mi);
		}
	}

	glutSwapBuffers();
}

// -------------------------------------------------------
// Declarations, initializers, glut funcs, and main
void init(void);
void resize(int, int);
void keys(unsigned char, int, int);
void upkeys(unsigned char, int, int);
void skeys(int, int, int);
void supkeys(int, int, int);
int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(0,0);
	
	glutCreateWindow("Asteroids");
	glutDisplayFunc(draw);
	glutTimerFunc(TIMER, timer, 0);

	glutKeyboardFunc(keys);
	glutKeyboardUpFunc(upkeys);
	glutSpecialFunc(skeys);
	glutSpecialUpFunc(supkeys);
	glutIgnoreKeyRepeat(1);
	
	// init OpenGL
	glClearColor(0.133,0.113, 0.107, 1.0);
	glColor3f(0.968, 0.968, 0.6);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(1.0, -1.0, 1.0, -1.0);
	glMatrixMode(GL_MODELVIEW);
	
	// init game
	init();	

	// initialize cmdline thread pipe
	if ( data_pipe_init( &client_dp ) == 0) {
		return -1;
	}
	pthread_create(&client_thread, NULL, client_loop, &client_dp);
	
	glutMainLoop();
	return 0;
}

// -------------------------------------------------
// glut funcs

void keys(unsigned char key, int x, int y) {
	switch(key) {
		case 'z': case 'Z':
			if (!(myship->state & SHIP_SHOOTING))
				myship->state += SHIP_SHOOTING;
			break;
		case 'x': case 'X':
			if (!(myship->state & SHIP_SHOOTING_MISSILE))
				myship->state += SHIP_SHOOTING_MISSILE;
			break;
	}
}

void upkeys(unsigned char key, int x, int y) {
	switch(key) {
		case 'z': case 'Z':
			if (myship->state & SHIP_SHOOTING)
				myship->state -= SHIP_SHOOTING;
			break;
		case 'x': case 'X':
			if (myship->state & SHIP_SHOOTING_MISSILE)
				myship->state -= SHIP_SHOOTING_MISSILE;
			break;
		case 'q': case 'Q':
			exit(0);
	}
}

void skeys(int key, int x, int y) {
	if (key == GLUT_KEY_UP) {
		myship->state += OBJ_ACCELERATING;
	} else
	if (key == GLUT_KEY_LEFT) {
		myship->state += OBJ_TURNING_LEFT;
	} else
	if (key == GLUT_KEY_RIGHT) {
		myship->state += OBJ_TURNING_RIGHT;
	}
}

void supkeys(int key, int x, int y) {
	
	if (key == GLUT_KEY_UP) {
		myship->state -= OBJ_ACCELERATING;
	} else
	if (key == GLUT_KEY_LEFT) {
		myship->state -= OBJ_TURNING_LEFT;
	} else
	if (key == GLUT_KEY_RIGHT) {
		myship->state -= OBJ_TURNING_RIGHT;
	}
}
