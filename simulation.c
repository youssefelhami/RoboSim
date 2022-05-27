/*!
 \file simulation.c
 \brief Module gérant la simulation à haut niveau et les fichiers.
 \author Sylvain Pellegrini
 \group Sylvain Pellegrini
		Simon Gilgien
 \modifications pour rendu2 public   R. Boulic
 \version 1.1
 \date 2 mai 2018
 */
 
#include <stdio.h>
#include <string.h>
#include "robot.h"
#include "particule.h"
#include "utilitaire.h"
#include "error.h"
#include "constantes.h"
#include "simulation.h"

#define LARGEUR_CADRE	5
/**
 * \brief états de l'automate de lecture
 * SET_NB_ROBOT		lecture du nombre de robots
 * E_ROBOT			lecture des données des robots
 * SET_NB_PART		lecture du nombre de particules
 * E_PARTICULE		lecture des données des particules
 * FIN				lecture complétée
 */
enum etats_lecture {SET_NB_ROBOT, E_ROBOT,SET_NB_PART,E_PARTICULE,FIN};

static bool simulation_decodage_robot(int *etat,int nb_robots, char *tab,int *i,
									  int ligne);
									  
static bool simulation_decodage_particule(int *etat,int nb_particules,char *tab,int *i,
										  int ligne);
										  
static bool simulation_decodage_fin_liste(char *tab);

static bool simulation_fermeture_fichier_erreur(FILE *file);

static bool simulation_decodage_nombre_robots(char *tab,int *i, int *etat,
											  int *nb_robots,int ligne);
											  
static bool simulation_decodage_nombre_particules(char *tab,int *i,int *etat,
												  int *nb_particules,int ligne);

bool simulation_lecture(char *nom_fichier)
{
	FILE *file = NULL;
	int nb_robots, nb_particules,etat = SET_NB_ROBOT,ligne = 0,i;
	char tab[MAX_LINE];
	char c;
	
	if (!(file = fopen (nom_fichier, "r")))
	{
		error_file_missing(nom_fichier);
		return false;
	}
	while (fgets(tab,MAX_LINE,file))
	{
		++ligne;
		if (sscanf(tab," %c",&c)!=1 || c == '#')
			continue;
		switch(etat)
		{
		case SET_NB_ROBOT :
			if (!simulation_decodage_nombre_robots(tab,&i,&etat,&nb_robots,ligne))
				return simulation_fermeture_fichier_erreur(file);
			break;
		case E_ROBOT :
			if(!simulation_decodage_robot(&etat,nb_robots,tab,&i,ligne))
				return simulation_fermeture_fichier_erreur(file);
			break;	
		case SET_NB_PART :
			if (!simulation_decodage_nombre_particules(tab,&i,&etat,&nb_particules,
													   ligne))
				return simulation_fermeture_fichier_erreur(file);
			break;
		case E_PARTICULE :
			if(!simulation_decodage_particule(&etat,nb_particules,tab,&i,ligne))
				return simulation_fermeture_fichier_erreur(file);
			break;
		case FIN :
			error_useless_char(ligne);
			return simulation_fermeture_fichier_erreur(file);
		default :                                                              
			return simulation_fermeture_fichier_erreur(file);
		}			
	}
	if(etat != FIN)
	{
		error_end_of_file(ligne);
		return simulation_fermeture_fichier_erreur(file);
	}
	error_no_error_in_this_file();	
	fclose(file);									 
	return true;
}

void simulation_ecriture(char *nom_fichier)
{
	FILE *fichier;
	
	if((fichier = fopen(nom_fichier, "w")))
	{
		robot_ecrire_fichier(fichier);
		particule_ecrire_fichier(fichier);

		fclose(fichier);
	}
}

void simulation_dessiner(void)
{
	util_debut_dessin(-DMAX, DMAX, -DMAX, DMAX);

	COULEUR noir = {0., 0., 0.};
	S2D hg = {-DMAX, -DMAX};
	S2D hd = { DMAX, -DMAX};
	S2D bg = {-DMAX,  DMAX};
	S2D bd = { DMAX,  DMAX};
	util_dessiner_segment(hg, hd, noir, LARGEUR_CADRE);
	util_dessiner_segment(hd, bd, noir, LARGEUR_CADRE);
	util_dessiner_segment(bd, bg, noir, LARGEUR_CADRE);
	util_dessiner_segment(bg, hg, noir, LARGEUR_CADRE);

	robot_dessiner();

	particule_dessiner();
}

static bool simulation_decodage_robot(int *etat,int nb_robots, char *tab,int *i,
									  int ligne)
{
	S2D pos;
	double angle;
	int offset = 0, n;
	if(simulation_decodage_fin_liste(tab))
	{
		if(*i < nb_robots)
		{
			error_fin_liste_robots(ligne);
			return false;
		}
		*etat = SET_NB_PART;
		return true;
	}
	if((*i) == nb_robots)
	{
		error_missing_fin_liste_robots(ligne);
		return false;
	}
	// %n Receives an integer of value equal to the number of characters read so far
    while(sscanf(&tab[offset]," %lf %lf %lf %n",&pos.x,&pos.y,&angle, &n)==3)
	{
		if(util_alpha_dehors(angle))
		{
			error_invalid_robot_angle(angle);
			return false;
		}
		util_range_angle(&angle);
		robot_set_robot(++(*i), pos, angle);
		
		if(robot_collision(*i))
			return(false);

		offset+=n;
		if((*i) == nb_robots && offset != strlen(tab))
		{
			error_useless_char(ligne);
			return false;
		}
	}
	if(offset != strlen(tab))
	{
		error_invalid_robot();
		return false;
	}
	return true;
}

static bool simulation_decodage_particule(int *etat,int nb_particules,char *tab,int *i,
										  int ligne)
{
	C2D pos;
	double energie;
	int offset = 0, n;
	if(simulation_decodage_fin_liste(tab))
	{
		if(*i < nb_particules)
		{
			error_fin_liste_particules(ligne);
			return false;
		}
		*etat = FIN;
		return true;
	}
	if((*i) == nb_particules)
	{
		error_missing_fin_liste_particules(ligne);
		return false;
	}
	// %n Receives an integer of value equal to the number of characters read so far
    while(sscanf(&tab[offset], " %lf %lf %lf  %lf %n ", &energie, &pos.rayon,
				 &pos.centre.x, &pos.centre.y, &n)==4)
	{
		if (!particule_is_valid(pos, energie))
		{
			error_invalid_particule_value(energie,pos.rayon,pos.centre.x,pos.centre.y);
			return false;
		}
		particule_set_particule(++(*i),pos,energie);

		if(particule_collision(*i))
			return false;
			
		if(robot_collision_particule(*i))
			return false;
			
		offset+=n;
		if ((*i) == nb_particules && offset != strlen(tab))
		{
			error_useless_char(ligne);
			return false;
		}
	}
	if(offset != strlen(tab))
	{
		error_invalid_particule(ligne);
		return false;
	}
	return true;
}

static bool simulation_decodage_fin_liste(char *tab)
{
	int n;
    sscanf(tab," FIN_LISTE %n",&n);
    if (n != strlen(tab))
		return false;
	else
		return true;	
}

static bool simulation_fermeture_fichier_erreur(FILE *file)
{
	if(!fclose(file)==0)
		printf("le fichier s'est mal fermé\n");
	robot_set_nombre(0);
	particule_set_nombre(0);
	return false;
}

static bool simulation_decodage_nombre_robots(char *tab,int *i, int *etat,
											  int *nb_robots,int ligne)
{
	int n;
	if ((sscanf(tab," %d %n", nb_robots,&n) !=1)||(*nb_robots < 0)) 
	{
		error_invalid_nb_robots();
		return false;
	}
	if (n!=strlen(tab))
	{
		error_useless_char(ligne);
		return false;
	}
    *etat = *nb_robots ? E_ROBOT : SET_NB_PART;
	robot_set_nombre(*nb_robots);
	*i = 0;
	return true;
}

static bool simulation_decodage_nombre_particules(char *tab,int *i,int *etat,
												  int *nb_particules,int ligne)
{
	int n;
	if ((sscanf(tab," %d %n", nb_particules,&n) !=1)||(*nb_particules < 0))
	{
		error_invalid_nb_particules();	
		return false;
	}
	if (n!=strlen(tab))
	{
		error_useless_char(ligne);
		return false;
	}
    *etat = *nb_particules ? E_PARTICULE : FIN;
	particule_set_nombre(*nb_particules);
	*i = 0;
	return true;
}

void simulation_deplacement(void)
{
    int i;
    int nb_part = particule_nb_particules();

    if (nb_part==0)
		return;
   
    int nb_robot = robot_nb_robots();
    for (i=0; i<nb_robot; i++)
    {
        if (robot_manual(i))
        {
			deplacement_robot_manual(i);
		}
		else
		{
			deplacement_robot_normal(i);
			decontamination(i);
		}
    }
    if (update_nb_part())
    {
        set_robot_occupe();
        attribution_but();
    }
    decomposition();
}

void but_initial(void)
{
    attribution_but();
}

bool manual_robot(int id)
{
	return robot_manual(id);
}

double somme_des_energies(void)
{
	double somme;
	for (int i=1; i <= particule_nb_particules(); i++)
	{
		somme += particule_energie(i);
	}
	return somme;
}


void record_ecriture(int count, double Td)
{
	FILE *fichier;
	
	if((fichier = fopen("out.dat", "a")))
	{
		fprintf(fichier, "%d %lf\n", count, Td);
		fclose(fichier);
	}
}

void update_taux_decontamination (double*Td, double*Si, double*Sd)
{
	if (*Td<CENT_POUR_CENT)
	{
		*Sd= *Si-somme_des_energies();	
		*Td= CENT_POUR_CENT*((*Sd)/(*Si));	
	}
}

void simulation_selectioner_robot (int x, int y)
{
	S2D mouse_position;
	int robot_selection=-1;
	utilitaire_conversion(x,y, &mouse_position.x, &mouse_position.y);
	for (int i=0; i<robot_nb_robots(); i++)
		{	
			if (robot_manual(i))
			{
				robot_selection=i;
				deselectionner_robot(i);
			}
		}
	for (int i=0; i<robot_nb_robots(); i++)
	{
		if (util_point_dans_cercle(mouse_position, robot_position(i+1)))
		{
			if (robot_selection !=i)
			{
				selectionner_robot(i);
			}
			else 
			{
				deselectionner_robot(i);
			}
			return;
		}
	}
}	

void deselectionner_tout(void)
{
	for (int i=0; i< robot_nb_robots(); i++)
		deselectionner_robot(i);
}

void simulation_ajouter_vitesse_rotation(int i)
{
	if (chercher_vrot(i) + DELTA_VROT<=VROT_MAX)
		ajouter_vitesse_rotation(i);
}

void simulation_soustraire_vitesse_rotation(int i)
{
	if (chercher_vrot(i) - DELTA_VROT>=-VROT_MAX)
		soustraire_vitesse_rotation(i);
}

void simulation_augmenter_vitesse_translation(int i)
{
	if (retourner_vtran(i) + DELTA_VTRAN <=VTRAN_MAX)
		ajouter_vitesse_translation(i);
}

void simulation_soustraire_vitesse_translation(int i)
{
	if (retourner_vtran(i) - DELTA_VTRAN >=-VTRAN_MAX)
		soustraire_vitesse_translation(i);
}

void simulation_deplacement_robot_manuel(int i)
{
	deplacement_robot_manual(i);
}
int simulation_nombre_robot(void)
{
	return robot_nb_robots();
}

double vitesse_angle(void)
{
	double vitesse=0;
	for (int i=0; i<robot_nb_robots(); i++)
	{
		if (robot_manual(i))
		{
			vitesse = chercher_vrot(i);
			break;
		}
	}
	return vitesse;
}

double vitesse_transition(void)
{
	double vitesse=0;
	for (int i=0; i<robot_nb_robots(); i++)
	{
		if (robot_manual(i))
		{
			vitesse = retourner_vtran(i);
			break;
		}
	}
	return vitesse;
}

void eliminer_tout(void)
{
	supprimer_tout_part();
	printf("DONE2\n");
	eliminer_tout_robot();
}

