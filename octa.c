/*
	Compile with: c99 -O3 -lm -lGL -lglfw -o octa octa.c
*/

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <GLFW/glfw3.h>
#define PI 3.1415926535897932384626433832795L
#define S static
#define SI static inline
#define max(x,y) (((x)>(y))?(x):(y))
#define min(x,y) (((x)>(y))?(y):(x))

/* ## Globals and Typedefs */
struct { int w,h; } W = {600,600};
typedef struct t8 { enum {SUBDIV,EMPTY} type; struct t8 *sub[8]; } Octree;
Octree Empty = {EMPTY,{0}};
Octree Bright = {255, {0}};
Octree Dark = {92, {0}};
Octree Fancy = {SUBDIV, {&Bright,&Dark,&Dark,&Empty,&Dark,&Dark,&Empty,&Dark}};
Octree T = {SUBDIV,{&Fancy,&Dark,&Fancy,&Dark,&Empty,&Empty,&Fancy,&Bright}};
float Xrot=0, Yrot=0, Xpos=0, Ypos=0, Zpos=8;
GLFWwindow* Win;
bool Done;

// TODO glFrustum is depricated.
void perspective(GLdouble fovy,GLdouble aspect,GLdouble zNear,GLdouble zFar){
	GLdouble xmin, xmax, ymin, ymax;
	ymax = zNear * tan(fovy * PI / 360.0);
	ymin = -ymax;
	xmin = ymin * aspect;
	xmax = ymax * aspect;
	glFrustum(xmin, xmax, ymin, ymax, zNear, zFar); }

// TODO This is nasty
void drawbox (x,y,z,s,c) float x,y,z,s,c; {
	glBegin(GL_QUADS);
	// Front
	glColor3f(c, 0, 0);
	glVertex3f(x, y, z);
	glVertex3f(x+s, y, z);
	glVertex3f(x+s, y+s, z);
	glVertex3f(x, y+s, z);
	// Back
	glVertex3f(x, y, z+s);
	glVertex3f(x+s, y, z+s);
	glVertex3f(x+s, y+s, z+s);
	glVertex3f(x, y+s, z+s);
	// Left
	glColor3f(0, c, 0);
	glVertex3f(x, y, z);
	glVertex3f(x, y+s, z);
	glVertex3f(x, y+s, z+s);
	glVertex3f(x, y, z+s);
	// Right
	glVertex3f(x+s, y, z);
	glVertex3f(x+s, y+s, z);
	glVertex3f(x+s, y+s, z+s);
	glVertex3f(x+s, y, z+s);
	// Bottom
	glColor3f(0, 0, c);
	glVertex3f(x, y, z);
	glVertex3f(x+s, y, z);
	glVertex3f(x+s, y, z+s);
	glVertex3f(x, y, z+s);
	// Top
	glVertex3f(x, y+s, z);
	glVertex3f(x+s, y+s, z);
	glVertex3f(x+s, y+s, z+s);
	glVertex3f(x, y+s, z+s);
	glEnd(); }

void drawoctree (t,x,y,z,d) Octree *t; int x,y,z,d; {
#	define div(x,y) (((float)x)/((float)y))
#	define nice(x) ((x)?1:0)
#	define p(n) (1<<n)
	switch (t->type) {
	case SUBDIV:
		for (int ii=0; ii<8; ii++) {
			int X = nice(ii%2), Y = nice(ii<4), Z = nice(ii%4<2);
			drawoctree(t->sub[ii],x*2+X,y*2+Y,z*2+Z,d+1); }
		break;
	case EMPTY: break;
	default:
		drawbox(div(x,p(d)),div(y,p(d)),div(z,p(d)),div(1,p(d)),div(t->type,255)); }}

#define d2r(x) (x*PI/180.0f)
void up (float amount) { Ypos += amount; }
void left (float amount) {
	Xpos -= amount*cos(d2r(Xrot)); Zpos -= amount*sin(d2r(Xrot)); }
void forward (float amount) {
	Xpos += amount*sin(d2r(Xrot)); Zpos -= amount*cos(d2r(Xrot)); }

/* ## Screen Dumps */
void display () {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	glRotatef(Yrot, 1, 0, 0);
	glRotatef(Xrot, 0, 1, 0);
	glTranslatef(-Xpos, -Ypos, -Zpos);
	drawoctree(&T,0,0,0,1);
	glfwSwapBuffers(Win); }

void glfwkeys(GLFWwindow *w,int k,int sc,int act,int mods) {
	GLdouble m = 1.0;
	if (act == GLFW_REPEAT) m=0.1,act=GLFW_PRESS;
	if (act != GLFW_PRESS) return;
	switch (k) {
	case GLFW_KEY_W: forward(m*0.2); break;
	case GLFW_KEY_S: forward(m*-0.2); break;
	case GLFW_KEY_A: left(m*0.2); break;
	case GLFW_KEY_D: left(m*-0.2); break;
	case GLFW_KEY_8: up(m*-0.2); break;
	case GLFW_KEY_Q: Done=true;
	case GLFW_KEY_SPACE: up(m*0.2); break;
	case GLFW_KEY_UP: if (Yrot>-80) Yrot -= m*3; break;
	case GLFW_KEY_DOWN: if (Yrot<80) Yrot += m*3; break;
	case GLFW_KEY_LEFT: Xrot -= m*3; break;
	case GLFW_KEY_RIGHT: Xrot += m*3; break; }}

void resize(GLFWwindow *win, int w, int h) {
	glViewport(0, 0, W.w=w, W.h=h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	perspective(25.0, ((GLfloat)W.w)/((GLfloat)W.h), 1, 100); }

void glinit () {
	glEnable (GL_DEPTH_TEST);
	glClearColor(0.3, 0.5, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	resize(Win,W.w, W.h); }

int main (int argc, char **argv) {
	if (!glfwInit()) return 1;
	Win = glfwCreateWindow(W.w,W.h,"Octre Demo",0,0);
	if (!Win) { glfwTerminate(); return 1; }
	glfwMakeContextCurrent(Win);
	glinit();
	glfwSetKeyCallback(Win,glfwkeys);
	glfwSetWindowSizeCallback(Win,resize);
	while (!Done && !glfwWindowShouldClose(Win)) display(),glfwPollEvents();
	glfwTerminate(); }
