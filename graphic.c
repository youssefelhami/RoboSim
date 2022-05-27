/*!
 \file graphic.c
 \brief Module concentrant les dépendances à OpenGL
 \author Simon Gilgien
 \group Sylvain Pellegrini
		Simon Gilgien
 \version 1.0
 \date 19 avril 2018
 */
 
#include <GL/glu.h>
#include <math.h>
#include "graphic.h"
#include "constantes.h"

#define COTES_CERCLE	50

static double aspect_ratio = 1.;
static int width, height;
GLfloat x_min, x_max, y_min, y_max;

void graphic_cercle(double x, double y, double r, float* couleur, bool plein,
					double epaisseur)
{
	int i;
	glColor3fv(couleur);
	glLineWidth(epaisseur);	
	glBegin(plein?GL_POLYGON:GL_LINE_LOOP);
	for(i =0; i < COTES_CERCLE; i++)
	{
		glVertex2d(x+r*cos(2*i*M_PI/COTES_CERCLE),
				   y+r*sin(2*i*M_PI/COTES_CERCLE));
	}
	glEnd();
}

void graphic_segment(double x_a, double y_a, double x_b, double y_b, float *couleur,
					 double largeur)
{
	glColor3fv(couleur);
	glLineWidth(largeur);
	glBegin(GL_LINES);
	glVertex2d(x_a, y_a);
	glVertex2d(x_b, y_b);
	glEnd();
}

void graphic_reshape(int largeur, int hauteur)
{
	glViewport(0, 0, largeur, hauteur);
	width = largeur;
	height= hauteur;
	aspect_ratio = (double)largeur/(double)(hauteur?hauteur:1);
}

void graphic_begin_draw(double gauche, double droite, double haut, double bas)
{
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	if (aspect_ratio <= 1.)
	{
	x_min = -DMAX; x_max = DMAX;
	y_min = -DMAX/aspect_ratio ;
	y_max = DMAX/aspect_ratio ;
	}
	else
	{
	x_min = -DMAX*aspect_ratio;
	x_max = DMAX*aspect_ratio;
	y_min = -DMAX ; y_max = DMAX ;
	}
	glOrtho(x_min, x_max, y_min, y_max, -1.0, 1.0);
	//int target_x = droite-gauche;
	//int target_y = bas-haut;
	//double target_ratio = (double)target_x/target_y;
	//double rapport = target_ratio/aspect_ratio;
	//if(rapport < 1)
		//glOrtho(gauche/rapport, droite/rapport, haut, bas, -1.0, 1.0);
	//else
		//glOrtho(gauche, droite, haut*rapport, bas*rapport, -1.0, 1.0);
}

void conversion (float x_pos, float y_pos, double* mouse_pos_x, double* mouse_pos_y)
{
	*mouse_pos_x=((float) x_pos/width)*(x_max - x_min) + x_min;
	*mouse_pos_y=((float)(height - y_pos)/height)*(y_max - y_min) + y_min;
}
