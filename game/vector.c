
typedef struct Vector {
        float x;
        float y;
} vector;
typedef struct matrix {
        vector v[2];
} matrix;

void vecToString(vector *v) {
	printf("X: %f, Y: %f\n", v->x, v->y);
}
void matToString(matrix *m) {
	printf("| %f %f |\n", m->v->x, m->v->y);
	printf("| %f %f |\n", m->v[1].x, m->v[1].y);
}

vector makeVec(float x, float y) {
	vector r;
	r.x = x;
	r.y = y;
	return r;
}	

vector copyVec(vector *v) {
	return makeVec(v->x, v->y);
}

vector zeroVec() {
	return makeVec(0.0, 0.0);
}

vector upVec() {
	return makeVec(0.0, 1.0);
}

vector rightVec() {
	return makeVec(0.0, 1.0);
}

matrix makeMat(float x1, float x2, float y1, float y2) {
	vector x;
	x.x = x1; x.y = x2;
	vector y;
	y.x = y1; y.y = y2;
	matrix m;
	m.v[0] = x;
	m.v[1] = y;
	return m;
}
vector addVec(vector *v1, vector *v2) {
	vector r; r.x = v1->x + v2->x; r.y = v1->y + v2->y;
	return r;
}

vector subVec(vector *v1, vector *v2) {
	vector r; r.x = v1->x - v2->x; r.y = v1->y - v2->y;
	return r;
}

vector scaleVec(vector *v, float f) {
	vector r; r.x = v->x * f; r.y = v->y * f;
	return r;
}

float dotVec(vector *v1, vector *v2) {
	return v1->x*v2->x + v1->y*v2->y; 
}

float lengthVec(vector *v) {
	return sqrt(v->x*v->x + v->y*v->y);
}

vector normalizeVec(vector *v) {
	vector d = scaleVec(v, 1.0/lengthVec(v));
	return d;
}

vector transformVec(vector *v, matrix *M) {
	vector r;
	r.x = v->x*M->v->x + v->y*M->v->y;
	r.y = v->x*M->v[1].x + v->y*M->v[1].y;
	
	return r;
}

vector rotateVec(vector *v, float a) {
	matrix rm = makeMat(cos(a), -1*sin(a), sin(a), cos(a));
	return transformVec(v, &rm);
}

float angleBetweenVec(vector *a, vector *b) {
	return acos(dotVec(a, b)/(lengthVec(a)*lengthVec(b)));
}

vector lerpVec(vector *from, vector *to, float spd) {
	float angle = angleBetweenVec(from, to);
	if (angle < 0.1) return *from;	
	else if (angle > spd) return rotateVec(from, spd);
	return rotateVec(from, angle);
}

