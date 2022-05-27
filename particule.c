/*!
 \file particule.c
 \brief Module contenant le type opaque particule
        implementation avec une liste
 \author Simon Gilgien
 \group Sylvain Pellegrini
		Simon Gilgien
 \modifications pour rendu2 public   R. Boulic
 \version 1.2
 \date 9 mai 2018
 */
 
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "error.h"
#include "particule.h"
#include "constantes.h"

#define EPAISSEUR_TRAIT_PARTICULE		1

typedef struct Particule PARTICULE;
struct Particule
{
	C2D position;
	double energie;
	PARTICULE * next;
};

static COULEUR couleur_particule = {0.5, 0.5, 0.5};
static PARTICULE *tet = NULL; // tête de liste
static int nb = 0;
static int nb_precedent=0;

/**
 * \brief	NOUVEAU! ajouter une particule PENDANT LA SIMULATION, en fin de liste
 *          le nombre total de particule est incrémenté d'une unité.
 *          Cette fonction doit être appelée par particule_decomposition() 
 * 			qui est la fonction à exporter (à écrire).
 * 
 * \param pos		C2D avec La position et le rayon de la particule.
 * \param energie	L'énergie de la particule.
 */
static void particule_ajouter(C2D pos, double energie);


// initialisation seulement avec lecture fichier et nettoyage liste
void particule_set_nombre(int nb_part)
{
	assert(nb_part >=0);
	PARTICULE *courant=tet;

	// destruction de toute la liste si nb_part est nul
	if(nb_part == 0)
	{
		while(courant)
		{
			tet = courant->next;
			free(courant);
			courant = tet;
		}
		tet = NULL;
	}
	else
	{
		// allocation d'une liste de nb_part éléments non-initialisés sauf next
		PARTICULE *new;
		int i;
		
		for(i=0 ; i< nb_part ; i++)
		{
			if(!(new = malloc(sizeof(PARTICULE))))
				exit(EXIT_FAILURE);

			new->next = tet;
			tet = new;
		}
	}
	nb = nb_part;
	nb_precedent=nb;
}

// initialisation de l'élément indice dans phase de lecture
void particule_set_particule(int indice, C2D pos, double energie)
{
	assert(0<indice && indice <= nb);
	
	PARTICULE *new= tet;
	int i;
		
	for(i=1 ; i< indice ; i++) 
		new = new-> next;
				
	new->position = pos;
	new->energie  = energie;	
}

void particule_ecrire_fichier(FILE *fichier)
{	
	fprintf(fichier, "\n%d\n", nb);
	if(nb)
	{
		PARTICULE *courant = tet;
	
		while(courant)
		{
			C2D pos = courant->position;
			fprintf(fichier, "\t%g %g %g %g\n", courant->energie,
												pos.rayon,
												pos.centre.x,
												pos.centre.y);
		}
		fprintf(fichier, "FIN_LISTE\n");
	}
}

int particule_nb_particules(void)
{
	return nb;
}

C2D particule_position(int i)
{
	PARTICULE *courant = tet;
	C2D init={{0.,0.},0.};
	int k=1;
	
	while(courant)
	{
		if(k == i)
			return courant->position;
		else
		{
			courant = courant->next;
			k++;
		}
	}
	return init;
}

double particule_energie(int i)
{
	assert(0<i && i <=nb);

	PARTICULE *courant = tet;
	int k=1;
	
	while(courant)
	{
		if(k == i)
			return courant->energie;
		else
		{
			courant = courant->next;
			k++;
		}
	}
	return 0.;
}

void particule_dessiner(void)
{
	PARTICULE *courant = tet;
	
	while(courant)
	{
		util_dessiner_cercle(courant->position, couleur_particule, true,
							 EPAISSEUR_TRAIT_PARTICULE);
		courant = courant->next;
	}
}

bool particule_collision(int i)
{
	int j=1;
	PARTICULE *courant = tet, *part_i=NULL;;
	double dist = 0.;
	
	while(courant)
	{
		if(j == i)
		{
			part_i = courant;
			break;
		}
		else
		{
			courant = courant->next;
			j++;
		}
	}

	if(part_i)
	{
		j=1;
		courant= tet;

		while(courant && courant != part_i)
		{
			if(util_collision_cercle(part_i->position, courant->position, &dist))
			{
				error_collision(PARTICULE_PARTICULE, i,j);
				return true;
			}
			courant = courant->next;
			j++;		
		}
	}
	return false;
}

bool particule_is_valid(C2D pos, double energie)
{
	return !util_point_dehors(pos.centre, DMAX) &&
		   pos.rayon <= R_PARTICULE_MAX			&&
		   pos.rayon >= R_PARTICULE_MIN 		&&
		   energie   <= E_PARTICULE_MAX			&&
		   energie   >= 0;
}

//
// fonction interne au module à utiliser dans future fonction particule_decomposition()
//
static void particule_ajouter(C2D pos, double energie)
{
	PARTICULE *new, *courant;
	
	if(!(new = malloc(sizeof(PARTICULE))))
		exit(EXIT_FAILURE);
		
	new->position = pos;
	new->energie  = energie;
	new->next     = NULL;
	
	if(!tet) tet = new;
	else
	{
		courant = tet;
		while(courant->next)
			courant = courant->next;
			
		courant->next = new;
	}
	
	nb++;
}

bool condition_decomposition(void)
{
    double p = rand()%RAND_MAX;
    p/=RAND_MAX;
    if (p <= DECOMPOSITION_RATE)
        return true;
    else
        return false;
}

PARTICULE* chercher_particule(int i)
{
    PARTICULE *courant = tet;
    int k=1;
    
    while(courant)
    {
        if(k == i)
            return courant;
        else
        {
            courant = courant->next;
            k++;
        }
    }
    return NULL;
}

void eliminer_particule(int id)
{
    if(id>1)
    {
        PARTICULE * precedent = chercher_particule(id-1);
        PARTICULE * courant = precedent->next;
        if (courant==NULL)
            return;
        if (courant!=tet)
        {
            precedent->next = courant->next;
            free(courant);
            nb--;
        }
    }
    else
    {
        PARTICULE * part = chercher_particule(id);
        if (part==NULL)
            return;
        tet= part->next;
        free(part);
        nb--;
    }
}

void decomposer_part(int id)
{
    PARTICULE* part = chercher_particule(id);
    if (part==NULL || part->position.rayon*R_PARTICULE_FACTOR < R_PARTICULE_MIN)
        return;
    C2D pos;
    double energie;
    pos.rayon=part->position.rayon*R_PARTICULE_FACTOR;
    energie = part->energie*E_PARTICULE_FACTOR;
    
    pos.centre.x=part->position.centre.x+pos.rayon;
    pos.centre.y=part->position.centre.y+pos.rayon;
    particule_ajouter(pos, energie);
    
    pos.centre.x=part->position.centre.x-pos.rayon;
    pos.centre.y=part->position.centre.y+pos.rayon;
    particule_ajouter(pos, energie);
    
    pos.centre.x=part->position.centre.x-pos.rayon;
    pos.centre.y=part->position.centre.y-pos.rayon;
    particule_ajouter(pos, energie);
    
    pos.centre.x=part->position.centre.x+pos.rayon;
    pos.centre.y=part->position.centre.y-pos.rayon;
    particule_ajouter(pos, energie);
    
    eliminer_particule(id);
}

void decomposition(void)
{
    int i=0, j=1;
    int nb_particules=nb;
    for (i=0; i<nb_particules; i++)
    {
        if (condition_decomposition())
            decomposer_part(j);
        else
            j++;
    }
}

bool update_nb_part(void)
{
	if (nb_precedent == nb)
		return false;
	nb_precedent=nb;
	return true;
}

void supprimer_tout_part(void)
{
	PARTICULE* e1;
	printf("BEGIN\n");
	while (nb > 0)
	{
		e1=tet;
		while (e1->next != NULL)
		{
			e1=e1->next;	
		}
		nb--;
		free(e1->next);
	}
	free(tet);
	tet=NULL;
	printf("end\n");
}






