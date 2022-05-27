/*!
 \file robot.c
 \brief Module gérant le type opaque robot
 \author Simon Gilgien
 \group Sylvain Pellegrini
		Simon Gilgien
 \modifications pour rendu2 public   R. Boulic
 \version 1.1
 \date 2 mai 2018
*/
 
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "constantes.h"
#include "error.h"
#include "particule.h"
#include "robot.h"

#define EPAISSEUR_ROBOT		2
#define NB_COORD			2
#define RAYON_PARTICULE		1
#define ID_PARTICULE		0
#define RAYON_CENTRE		0.1

typedef struct Robot ROBOT;
struct Robot
{
    bool occupe;
    bool manual;
    double vrot;
    double vtrans;
	C2D position;
	double angle;
	int particule_cible;
};

static COULEUR couleur_robot =  {0., 0., 0.};
static COULEUR couleur_centre = {1., 0., 0.};
static ROBOT* tab = NULL;
static int nb = 0;

void robot_set_nombre(int nb_robots)
{
	assert(nb_robots >=0);
	if(nb_robots >0)
		tab = realloc(tab, nb_robots*sizeof(ROBOT));
	else
	{
		if(tab)
			free(tab);
		tab = NULL;
	}
	nb = nb_robots;
}

void robot_set_robot(int i, S2D pos, double angle)
{
	assert(0<i && i<=nb);
	tab[i-1].position.centre = pos;
	tab[i-1].position.rayon = R_ROBOT;
	tab[i-1].angle = angle;
	tab[i-1].particule_cible = -1;
    tab[i-1].occupe = false;
    tab[i-1].vrot = VROT_MAX;
    tab[i-1].vtrans = VTRAN_MAX;
    tab[i-1].manual = false;
}	

void robot_ecrire_fichier(FILE* fichier)
{
	int i;
	
	fprintf(fichier, "# Fichier généré\n#\n%d\n", nb);
	
	if(nb)
	{
		for(i=0; i<nb; i++)
		{
			fprintf(fichier, "\t%g %g %g\n", tab[i].position.centre.x, 
											 tab[i].position.centre.y,
											 tab[i].angle);
		}
		fprintf(fichier, "FIN_LISTE\n");
	}	
}

int robot_nb_robots(void)
{
	return nb;
}

C2D robot_position(int i)
{
	assert(0<=i && i <=nb);
	return tab[i-1].position;
}

double robot_orientation(int i)
{
	assert(0<i && i <= nb);
	return tab[i-1].angle;
}

void robot_dessiner(void)
{
	int i;
	S2D centre;
    for(i =1; i<=nb; i++)
	{
		centre = robot_position(i).centre;
		if (robot_manual(i-1))
		{
			util_dessiner_cercle(robot_position(i), couleur_centre, false,
							 EPAISSEUR_ROBOT);
		}
		else
		{
			util_dessiner_cercle(robot_position(i), couleur_robot, false,
								EPAISSEUR_ROBOT);
		}
		util_dessiner_segment(centre,
							  util_deplacement(centre, robot_orientation(i), R_ROBOT),
							  couleur_robot, EPAISSEUR_ROBOT);
		C2D point_central = {centre, RAYON_CENTRE};
		util_dessiner_cercle(point_central, couleur_centre, true, EPAISSEUR_ROBOT);
	}
}

bool robot_collision(int i)
{
	assert(0<i && i<=nb);
	int j;

	for (j=1; j < i; j++)
	{
		if (robot_collision_r_r(j, i))
		{
			error_collision(ROBOT_ROBOT, j, i);
			return true;
		}
	}
	return false;
}

bool robot_collision_particule(int i_part)
{
	int j;
	
	for (j = nb; j > 0; j--)
	{
		if(robot_collision_r_p(j, i_part))
		{
			error_collision(ROBOT_PARTICULE, j, i_part);
			return true;
		}
	}
	return false;
}

bool robot_collision_r_r(int i, int j)
{
	assert(0<i && i<=nb);
	assert(0<j && j<=nb);
	double dist;
	return util_collision_cercle(robot_position(i), robot_position(j), &dist);
}

bool robot_collision_r_p(int robot, int part)
{
	assert(0<robot && robot<=nb);
	double dist;
	return util_collision_cercle(robot_position(robot), particule_position(part),
								 &dist);
}



void set_robot_occupe(void)
{
    for (int i=0; i<nb; i++)
    {
        tab[i].occupe=false;
        tab[i].particule_cible=-1;
    }
}

void tri_particule(int taille, double tableau[NB_COORD][taille])
{
    int i, j;
    for (i = 1; i < taille; ++i) {
        double rayon = tableau[RAYON_PARTICULE][i];
        double indice = tableau[ID_PARTICULE][i];
        for (j = i; j > 0 && tableau[RAYON_PARTICULE][j-1] > rayon; j--)
        {
            tableau[RAYON_PARTICULE][j] = tableau[RAYON_PARTICULE][j-1];
            tableau[ID_PARTICULE][j] = tableau[ID_PARTICULE][j-1];
        }
        tableau[RAYON_PARTICULE][j] = rayon;
        tableau[ID_PARTICULE][j] = indice;
    }
}

double calcul_temps(C2D particule, int id_robot)
{
    double temps=0;
    C2D cercle_robot=tab[id_robot].position;
    double distance=util_angle(particule.centre, cercle_robot.centre);
    double angle;
    util_ecart_angle(cercle_robot.centre, tab[id_robot].angle, particule.centre, 
					 &angle);
    util_range_angle(&angle);
    angle=fabs(angle);
    temps= distance/VTRAN_MAX + angle/VROT_MAX;
    return temps;
}


void robot_proche(int part)
{
    C2D coord_particule = particule_position(part);
    int robot_min=0;
    double temps;
    double temps_max;
    for (int j=0; j<nb; j++)
    {
		if (tab[j].occupe==false)
		{
			temps_max=calcul_temps(coord_particule, 0);
			robot_min=j;
			break;
		}
	}
    for (int i=0; i<nb; i++)
    {
        temps=calcul_temps(coord_particule, i);
        if(tab[i].occupe==false)
        {
            if (temps<=temps_max)
            {
                robot_min=i;
                temps_max=temps;
            }
        }
    }
    tab[robot_min].occupe=true;
    tab[robot_min].particule_cible=part;
}

void attribution_but(void)
{
    int nb_part= particule_nb_particules();
    double rayon_part[NB_COORD][nb_part];
    C2D coord;
    int part;
    for (int i=0; i<nb_part; i++)
    {
        coord=particule_position(i+1);
        rayon_part[ID_PARTICULE][i]=i+1;
        rayon_part[RAYON_PARTICULE][i]=coord.rayon;
    }
    tri_particule(nb_part, rayon_part);
    set_robot_occupe();
    if (nb_part>=nb)
    {
        for (int j=1; j<=nb; j++)
        {
            part=rayon_part[ID_PARTICULE][nb_part-j];
            robot_proche(part);
        }
    }
    if (nb_part<nb)
    {
        int k;
        for (k=1; k<=nb_part; k++)
        {
            part=rayon_part[ID_PARTICULE][nb_part-k];
            robot_proche(part);
        }
        for (k=1; k<=nb-nb_part; k++)
        {
            part=rayon_part[ID_PARTICULE][nb_part-k];
            robot_proche(part);
        }
    }
}

bool robot_collision_rob_cercle(C2D a, C2D b)
{
    double dist;
    return util_collision_cercle(a, b, &dist);
}

C2D recul_robot(C2D pos_actuelle, int i, C2D cible)
{
    double delta_d = util_distance(tab[i].position.centre, pos_actuelle.centre);
    double D = util_distance(pos_actuelle.centre, cible.centre);
    double L = util_distance(tab[i].position.centre, cible.centre);
    double rayons = pos_actuelle.rayon + cible.rayon;
    double new_dist;
    util_inner_triangle(delta_d, D, L, rayons, &new_dist);
	if (fabs(new_dist) > fabs(tab[i].vtrans * DELTA_T))
		new_dist=0;
    if (tab[i].vtrans >0)
    {
        pos_actuelle.centre.x = tab[i].position.centre.x + new_dist*cos(tab[i].angle);
        pos_actuelle.centre.y = tab[i].position.centre.y + new_dist*sin(tab[i].angle);
    }
    else
    {
        pos_actuelle.centre.x = tab[i].position.centre.x - new_dist*cos(tab[i].angle);
        pos_actuelle.centre.y = tab[i].position.centre.y -new_dist*sin(tab[i].angle);
    }
    return pos_actuelle;
}

void robot_collision_correction(int i, C2D robot)
{
    int j;
    C2D robot_cible;
    for (j=0; j < nb; j++)
    {
        if (j != i)
        {
			robot_cible=tab[j].position;
            if (robot_collision_rob_cercle(robot, robot_cible))
            {
                robot = recul_robot(robot, i, tab[j].position);
            }
        }
    }
    int nb_part = particule_nb_particules();
    C2D particule;
    for (j=1; j<=nb_part; j++)
    {
        particule = particule_position(j);
        if (robot_collision_rob_cercle(robot, particule))
        {
            robot = recul_robot(robot, i, particule);
            tab[i].particule_cible=j;
        }
    }
    tab[i].position=robot;
}

void deplacement_robot_normal(int i)
{
    if (tab[i].occupe==false)
		return;
    int id_part=tab[i].particule_cible;
    C2D particule=particule_position(id_part);
    C2D cercle_robot=tab[i].position;
    double angle;
    util_ecart_angle(cercle_robot.centre, tab[i].angle, particule.centre, &angle);
    util_range_angle(&angle);
    if (fabs(angle)> M_PI*0.5)
    {
        if (angle>0)
        {
            tab[i].angle+=tab[i].vrot*DELTA_T;
        }
        else
        {
            tab[i].angle-=tab[i].vrot*DELTA_T;
        }
    }
    else
    {
        if (fabs(angle)<= fabs(tab[i].vrot*DELTA_T))
        {
            double temp =util_angle(cercle_robot.centre, particule.centre);
            util_range_angle(&temp);
            tab[i].angle=temp;
        }
        else
        {
            if (angle>0)
            {
                tab[i].angle+=tab[i].vrot*DELTA_T;
            }
            else
            {
                tab[i].angle-=tab[i].vrot*DELTA_T;
            }
        }
        double distance = tab[i].vtrans*DELTA_T;
        cercle_robot.centre.x+=distance*cos(tab[i].angle);
        cercle_robot.centre.y+=distance*sin(tab[i].angle);
        
        
    }
    robot_collision_correction(i, cercle_robot);
}

void decontamination(int id)
{
    int id_part = tab[id].particule_cible;
    C2D particule = particule_position(id_part);
    C2D cercle_robot = tab[id].position;
    double temp_angle = util_angle(cercle_robot.centre, particule.centre);
    util_range_angle(&temp_angle);
    if ((util_distance(particule.centre, cercle_robot.centre)<= particule.rayon +
        cercle_robot.rayon + EPSIL_ZERO) &&
        (fabs(temp_angle-tab[id].angle)<EPSIL_ALIGNEMENT))
    {
        eliminer_particule(id_part);
    }
}

void deplacement_robot_manual(int i)
{
	C2D cercle_robot=tab[i].position;
	tab[i].angle += tab[i].vrot*DELTA_T;
	double distance = tab[i].vtrans*DELTA_T;
    cercle_robot.centre.x+=distance*cos(tab[i].angle);
    cercle_robot.centre.y+=distance*sin(tab[i].angle);
	robot_collision_correction(i, cercle_robot);
}

bool robot_manual(int i)
{
	return tab[i].manual;
}

void ajouter_vitesse_rotation(int i)
{
	tab[i].vrot += DELTA_VROT;
}

void soustraire_vitesse_rotation(int i)
{
	tab[i].vrot -= DELTA_VROT;
}

void ajouter_vitesse_translation(int i)
{
	tab[i].vtrans += DELTA_VTRAN;
}

void soustraire_vitesse_translation(int i)
{
	tab[i].vtrans -= DELTA_VTRAN;
}

void selectionner_robot(int id)
{
	tab[id].manual=true;
	tab[id].vrot=0;
	tab[id].vtrans=0;
}

void deselectionner_robot(int id)
{
	tab[id].manual=false;
	tab[id].vrot=VROT_MAX;
	tab[id].vtrans=VTRAN_MAX;
}

double retourner_vtran(int id)
{
	return tab[id].vtrans;
}

double chercher_vrot(int id)
{
	return tab[id].vrot;
}

void eliminer_tout_robot(void)
{
	free(tab);
	tab=NULL;
}
