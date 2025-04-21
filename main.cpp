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
#define MAX_RAIN 50
#define MAX_HOGS 12
#define DROP_WIDTH 0.5
#define DROP_HEIGHT 0.5
#define DROP_DEPTH 0.5
const float dropSpawnRate = 0.3; // lower means more frequent
const float spawnHeight = 1.9;
// float Vy = 0.03;
float lastDropTime = 0.0;
int currentDrop = 0;
unsigned char *background_texture;
int bg_xdim, bg_ydim;

struct RainDrop
{
   float x, y, z, Ax, Ay, Az, Vy;
   int img, xdim, ydim;
   bool draw;
   unsigned char *texture;
};

RainDrop rain[MAX_RAIN];

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
void block(RainDrop rainDrop)
{
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, rainDrop.xdim, rainDrop.ydim, 0, GL_RGB, GL_UNSIGNED_BYTE, rainDrop.texture);
   // Front face
   float xmin = rainDrop.x;
   float ymin = rainDrop.y;
   float zmin = rainDrop.z;
   float xmax = rainDrop.x + DROP_WIDTH;
   float ymax = rainDrop.y + DROP_HEIGHT;
   float zmax = rainDrop.z + DROP_DEPTH;

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

//--------------------------------------
// Time function to figure out how long
// the program has been running
// returns it in seconds
//--------------------------------------
float getTime()
{
   return glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
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
      rain[i].x = -1.5f + static_cast<float>(rand()) / (RAND_MAX / 3.0f);
      rain[i].y = spawnHeight;
      rain[i].z = 0.1;
      rain[i].Ax = rand() / RAND_MAX;
      rain[i].Ay = rand() / RAND_MAX;
      rain[i].Az = rand() / RAND_MAX;
      rain[i].Vy = 0.01f + static_cast<float>(rand()) / RAND_MAX * (0.05f - 0.01f);
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
      string filename = "textures/hog" + to_string(rain[i].img + 1) + ".jpg";
      init_texture((char *)filename.c_str(), rain[i].texture, rain[i].xdim, rain[i].ydim);
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

   // spin and draw the drop weeeeeeeeee
   for (int i = 0; i < MAX_RAIN; i++)
   {
      if (rain[i].draw)
      {
         glPushMatrix();

         float centerX = rain[i].x + DROP_WIDTH / 2.0f;
         float centerY = rain[i].y + DROP_HEIGHT / 2.0f;
         float centerZ = rain[i].z + DROP_DEPTH;

         glTranslatef(centerX, centerY, centerZ);
         glRotatef(rain[i].Ax, 1.0f, 0.0f, 0.0f);
         glRotatef(rain[i].Ay, 0.0f, 1.0f, 0.0f);
         glRotatef(rain[i].Az, 0.0f, 0.0f, 1.0f);

         glTranslatef(-centerX, -centerY, -centerZ);

         block(rain[i]);
         glPopMatrix();
      }
   }
   glFlush();
}

void idle()
{
   float currentTime = getTime();

   if ((currentTime - lastDropTime) > dropSpawnRate)
   {
      // spawn currentDrop
      lastDropTime = currentTime;
      rain[currentDrop].draw = true;
      // move to next RainDrop
      currentDrop++;
   }
   for (int i = 0; i < MAX_RAIN; i++)
   {
      if ((rain[i].draw) && (rain[i].y > -2.1))
      {
         // keep falling and spinning
         rain[i].y -= rain[i].Vy;
         rain[i].Ax += 1.5;
         rain[i].Ay += 1.5;
         rain[i].Az += 1.5;
      }
      else
      {
         // stop falling and randomize start points again
         rain[i].draw = false;
         rain[i].x = -1.5f + static_cast<float>(rand()) / (RAND_MAX / 3.0f); // -2 to +2
         rain[i].y = spawnHeight;                                            // -2 to +2
         rain[i].z = 0.1;                                                    // -2 to +2
         rain[i].img = rand() % MAX_HOGS;
      }
   }
   if (currentDrop >= MAX_RAIN)
   {
      currentDrop = 0;
   }
   glutPostRedisplay();
}

//---------------------------------------
// Main program
//---------------------------------------
int main(int argc, char *argv[])
{
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