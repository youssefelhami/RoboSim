/*!
 \file robot.h
 \brief Module gérant le type opaque robot
 \author Simon Gilgien
 \group Sylvain Pellegrini
		Simon Gilgien
 \modifications pour rendu2 public   R. Boulic
 \version 1.1
 \date 2 mai 2018
 */
 
#ifndef ROBOT_H
#define ROBOT_H

#include "utilitaire.h"

/**
 * \brief	Configure le nombre de robots. Les indices valides sont dans l'intervalle
 *			[1, nb_robots]. Si nb_robots = 0, les données sont effacées et aucun
 *			indice n'est valide. Les robots nouvellement créés doivent être configurés
 *			avant utilisation.
 * \param nb_robots	Le nombre de robots, doit être >=0.
 */
void robot_set_nombre(int nb_robots);

/**
 * \brief	Configure les paramètres d'un robot.
 * \param i		L'indice du robot à modifier, doit être un indice valide.
 * \param pos	La nouvelle position du robot.
 * \param angle	Le nouvel angle du robot.
 */
void robot_set_robot(int i, S2D pos, double angle);

/**
 * \brief			écriture de l'ensemble des robots dans un fichier.
 * \param fichier	pointeur vers le fichier ouvert en écriture

 */
void robot_ecrire_fichier(FILE* fichier);


/**
 * \brief	Retourne le nombre de robots existants, c'est-à-dire le plus grand indice
 *			valide.
 * \return	Le plus grand indice valide.
 */
int robot_nb_robots(void);

/**
 * \brief	Retourne la position d'un robot.
 * \param i	L'indice du robot, doit être un indice valide.
 * \return	La position du robot i.
 */
C2D robot_position(int i);

/**
 * \brief	Retourne l'orientation d'un robot.
 * \param i	L'indice du robot, doit être un indice valide.
 * \return l'orientation du robot, comprise dans ]-π, π].
 */
double robot_orientation(int i);

/**
 * \brief	Dessinne les robots en utilisant les fonctions de dessin de utilitaire.h
 */
void robot_dessiner(void);

/**
 * \brief	Contrôle si le robot i est en collision avec l'un des robots 
 *          d'indice inférieur à i.
 * \param i	L'indice du robot, doit être un indice valide.
 */
bool robot_collision(int i);

/**
 * \brief			Contrôle si un robot est en collision avec la particule i_part
 * \param i_part	L'indice de la particule testée, doit être un indice valide.
 */
bool robot_collision_particule(int i_part);

/**
 * \brief	Contrôle si les robots i et j sont en collision.
 * \param i	L'indice du premier robot, doit être un indice valide.
 * \param j L'indice du deuxième robot, doit être un indice valide.
 */
bool robot_collision_r_r(int i, int j);

/**
 * \brief	Contrôle si un robot est en collision avec une particule.
 * \param robot	L'indice du robot, doit être un indice de robot valide.
 * \param part	L'indice de la particule, doit être un indice de particule valide.
 */
bool robot_collision_r_p(int robot, int part);

void set_robot_occupe(void);

double calcul_temps(C2D particule, int id_robot);

void attribution_but(void);

void robot_proche(int part);
void decontamination(int id);
void deplacement_robot_normal(int i);
void robot_collision_correction(int i, C2D robot);
C2D recul_robot(C2D pos_actuelle, int i, C2D cible);
bool robot_collision_rob_cercle(C2D a, C2D b);
bool robot_manual(int i);
void deplacement_robot_manual(int i);
void changer_vitesse_manual(int id, double rotation, double transition);
void deselectionner_robot(int id);
void selectionner_robot(int id);
void ajouter_vitesse_rotation(int i);
void soustraire_vitesse_rotation(int i);
void ajouter_vitesse_translation(int i);
void soustraire_vitesse_translation(int i);
double retourner_vtran(int id);
double chercher_vrot(int id);
void eliminer_tout_robot(void);

#endif
