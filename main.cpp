//---------------------------------------
// Program: main.cpp
// Purpose: Assignment 5 Computer Graphics Spring 2025
// Author:  Lizzie Howell
// Date:    April 2025
//---------------------------------------
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef MAC
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include "libim/im_color.h"

// Global variables
#define ROTATE 1
#define TRANSLATE 2
#define MAX_RAIN 100
#define MAX_HOGS 12
#define HAIL_WIDTH 0.5
#define HAIL_HEIGHT 0.5
const float hailSpawnRate = 0.5;
const float spawnHeight = 1.9;
float Vy = 0.03;
float lastHailTime = 0.0;
int currentHail = 0;
unsigned char *background_texture;
int bg_xdim, bg_ydim;

struct Hail
{
   float x, y, z, Ax, Ay, Az;
   int img, xdim,ydim;
   bool draw;
   unsigned char *texture;
};

Hail rain[MAX_RAIN];

//---------------------------------------
// Initialize texture image
//---------------------------------------
void init_texture(char *name, unsigned char *&texture, int &xdim, int &ydim)
{
   // Read jpg image
   im_color image;
   image.ReadJpg(name);
   xdim = 1;
   while (xdim < image.R.Xdim)
      xdim *= 2;
   ydim = 1;
   while (ydim < image.R.Ydim)
      ydim *= 2;
   if (xdim - image.R.Xdim > image.R.Xdim - xdim / 2)
      xdim /= 2;
   if (ydim - image.R.Ydim > image.R.Ydim - ydim / 2)
      ydim /= 2;
   image.Interpolate(xdim, ydim);
   xdim = image.R.Xdim;
   ydim = image.R.Ydim;

   // Copy image into texture array
   texture = (unsigned char *)malloc((unsigned int)(xdim * ydim * 3));
   int index = 0;
   for (int y = 0; y < ydim; y++)
      for (int x = 0; x < xdim; x++)
      {
         texture[index++] = (unsigned char)(image.R.Data2D[y][x]);
         texture[index++] = (unsigned char)(image.G.Data2D[y][x]);
         texture[index++] = (unsigned char)(image.B.Data2D[y][x]);
      }
}

//---------------------------------------
// Function to draw 3D block
//---------------------------------------
void block(Hail rainDrop, float xmin, float ymin, float zmin,
   float xmax, float ymax, float zmax)
{
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, rainDrop.xdim, rainDrop.ydim, 0, GL_RGB, GL_UNSIGNED_BYTE, rainDrop.texture);
   // Front face
   glBegin(GL_POLYGON);
   glTexCoord2f(0.0, 1.0);
   glVertex3f(xmin, ymin, zmax);
   glTexCoord2f(1.0, 1.0);
   glVertex3f(xmax, ymin, zmax);
   glTexCoord2f(1.0, 0.0);
   glVertex3f(xmax, ymax, zmax);
   glTexCoord2f(0.0, 0.0);
   glVertex3f(xmin, ymax, zmax);
   glEnd();

   // Left face
   glBegin(GL_POLYGON);
   glTexCoord2f(0.0, 1.0);
   glVertex3f(xmin, ymin, zmin);
   glTexCoord2f(1.0, 1.0);
   glVertex3f(xmin, ymin, zmax);
   glTexCoord2f(1.0, 0.0);
   glVertex3f(xmin, ymax, zmax);
   glTexCoord2f(0.0, 0.0);
   glVertex3f(xmin, ymax, zmin);
   glEnd();

   // Back face
   glBegin(GL_POLYGON);
   glTexCoord2f(0.0, 1.0);
   glVertex3f(xmax, ymin, zmin);
   glTexCoord2f(1.0, 1.0);
   glVertex3f(xmin, ymin, zmin);
   glTexCoord2f(1.0, 0.0);
   glVertex3f(xmin, ymax, zmin);
   glTexCoord2f(0.0, 0.0);
   glVertex3f(xmax, ymax, zmin);
   glEnd();

   // Right face
   glBegin(GL_POLYGON);
   glTexCoord2f(0.0, 1.0);
   glVertex3f(xmax, ymin, zmax);
   glTexCoord2f(1.0, 1.0);
   glVertex3f(xmax, ymin, zmin);
   glTexCoord2f(1.0, 0.0);
   glVertex3f(xmax, ymax, zmin);
   glTexCoord2f(0.0, 0.0);
   glVertex3f(xmax, ymax, zmax);
   glEnd();

   // Bottom face
   glBegin(GL_POLYGON);
   glTexCoord2f(0.0, 1.0);
   glVertex3f(xmin, ymin, zmax);
   glTexCoord2f(1.0, 1.0);
   glVertex3f(xmin, ymin, zmin);
   glTexCoord2f(1.0, 0.0);
   glVertex3f(xmax, ymin, zmin);
   glTexCoord2f(0.0, 0.0);
   glVertex3f(xmax, ymin, zmax);
   glEnd();

   // Top face
   glBegin(GL_POLYGON);
   glTexCoord2f(0.0, 1.0);
   glVertex3f(xmax, ymax, zmin);
   glTexCoord2f(1.0, 1.0);
   glVertex3f(xmax, ymax, zmax);
   glTexCoord2f(1.0, 0.0);
   glVertex3f(xmin, ymax, zmax);
   glTexCoord2f(0.0, 0.0);
   glVertex3f(xmin, ymax, zmin);
   glEnd();
}

//---------------------------------------
// Function to draw background
//---------------------------------------
void draw_background()
{
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bg_xdim, bg_ydim, 0, GL_RGB, GL_UNSIGNED_BYTE, background_texture);

   glDisable(GL_DEPTH_TEST);
   glMatrixMode(GL_MODELVIEW);
   glPushMatrix();
   glLoadIdentity();

   glBegin(GL_QUADS);
   glTexCoord2f(0.0, 1.0);
   glVertex3f(-2.0, -2.0, -1.9);
   glTexCoord2f(1.0, 1.0);
   glVertex3f(2.0, -2.0, -1.9);
   glTexCoord2f(1.0, 0.0);
   glVertex3f(2.0, 2.0, -1.9);
   glTexCoord2f(0.0, 0.0);
   glVertex3f(-2.0, 2.0, -1.9);
   glEnd();

   glPopMatrix();
   glEnable(GL_DEPTH_TEST);
}

//---------------------------------------
// Init function for OpenGL
//---------------------------------------
void init()
{
   // Init view
   glClearColor(0.0, 0.0, 0.0, 1.0);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(-2.0, 2.0, -2.0, 2.0, -2.0, 2.0);
   glEnable(GL_DEPTH_TEST);

   srand(time(NULL));

   for (int i = 0; i < MAX_RAIN; i++)
   {
      rain[i].x = -1.5f + static_cast<float>(rand()) / (RAND_MAX / 3.0f); // -2 to +2
      rain[i].y = spawnHeight; // -2 to +2
      rain[i].z = 0.1; // -2 to +2
      rain[i].img = rand() % MAX_HOGS;
      rain[i].draw = false;
   }

   // Init texture
   glEnable(GL_TEXTURE_2D);
   glTexParameterf(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

   for (int i = 0; i < MAX_RAIN; i++)
   {
      char filename[64];
      sprintf(filename, "textures/hog%d.jpg", rain[i].img + 1);
      init_texture((char *)filename, rain[i].texture, rain[i].xdim, rain[i].ydim);
   }

   init_texture((char *)"textures/cloud2.jpg", background_texture, bg_xdim, bg_ydim);
   glEnable(GL_TEXTURE_2D);
   glTexParameterf(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bg_xdim, bg_ydim,
                0, GL_RGB, GL_UNSIGNED_BYTE, background_texture);
}

//---------------------------------------
// Display callback for OpenGL
//---------------------------------------
void display()
{
   // Incrementally rotate objects
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   draw_background();

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   // glTranslatef(xpos / 500.0, ypos / 500.0, zpos / 500.0);
   // glRotatef(xangle, 1.0, 0.0, 0.0);
   // glRotatef(yangle, 0.0, 1.0, 0.0);
   // glRotatef(zangle, 0.0, 0.0, 1.0);
   // spin the blocks weeeeeeeeee
   for (int i = 0; i < MAX_RAIN; i++) {
      if (rain[i].draw) {
         glPushMatrix();
   
         float centerX = rain[i].x + HAIL_WIDTH / 2.0f;
         float centerY = rain[i].y + HAIL_HEIGHT / 2.0f;
         float centerZ = rain[i].z + 0.25f;
   
         glTranslatef(centerX, centerY, centerZ);
         glRotatef(rain[i].Ax, 1.0f, 0.0f, 0.0f);
         glRotatef(rain[i].Ay, 0.0f, 1.0f, 0.0f);
         glRotatef(rain[i].Az, 0.0f, 0.0f, 1.0f);
   
         glTranslatef(-centerX, -centerY, -centerZ);
   
         block(rain[i], rain[i].x, rain[i].y, rain[i].z, rain[i].x + HAIL_WIDTH,  rain[i].y + HAIL_HEIGHT, rain[i].z + 0.5f); // depth = 0.5
   
         glPopMatrix();
      }
   }   
   glFlush();
}

float getTime()
{
   return glutGet(GLUT_ELAPSED_TIME) / 1000.0f; // returns time since program started in seconds
}

void idle()
{
   float currentTime = getTime();

   if ((currentTime - lastHailTime) > hailSpawnRate)
   {
      // spawn currenthail
      lastHailTime = currentTime;
      rain[currentHail].draw = true;
      currentHail++;
   }
   for (int i = 0; i < MAX_RAIN; i++){
      if ((rain[i].draw) && (rain[i].y > -2.1)) {
         // keep falling and spinning
         rain[i].y -= Vy;
         rain[i].Ax += 1;
         rain[i].Ay += 1;
         rain[i].Az += 1;
      } else {
          // stop falling
          rain[i].draw = false;
      }
   }
   if (currentHail > MAX_RAIN){
      currentHail = 0;
      for (int i = 0; i < MAX_RAIN; i++){
         rain[i].x = -1.5f + static_cast<float>(rand()) / (RAND_MAX / 3.0f); // -2 to +2
         rain[i].y = spawnHeight; // -2 to +2
         rain[i].z = 0.1; // -2 to +2
         rain[i].img = rand() % MAX_HOGS;
         rain[i].draw = false;
      }
   }
   glutPostRedisplay();
}

//
//---------------------------------------
// Main program
//---------------------------------------
int main(int argc, char *argv[])
{
   // thankyoutoo@cox.net
   //  Create OpenGL window
   glutInit(&argc, argv);
   glutInitWindowSize(500, 500);
   glutInitWindowPosition(250, 250);
   glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE | GLUT_DEPTH);
   glutCreateWindow("Razorback Rain");
   glutDisplayFunc(display);
   glutIdleFunc(idle);

   init();
   glutMainLoop();
   return 0;
}