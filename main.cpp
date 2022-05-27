/*!
 \file main.cpp
 \brief Module principal, gère l'interface graphique.
 \author Sylvain Pellegrini
 \group Sylvain Pellegrini
		Simon Gilgien
 \modifications pour rendu2 public   R. Boulic
 \version 1.1
 \date 2 mai 2018
 */
 
#include <GL/glui.h>
#include <GL/glut.h>
#include <cstring>
#include <cstdio>

extern "C"
{
	#include "simulation.h"
	#include "graphic.h"
	#include "constantes.h"
}

#define CONTROL_AUTO	0
#define CONTROL_MANUEL	1
#define TAILLE_INITIALE 600
#define CHAR_MAX		20

namespace
{
	int view_window;
	bool simulation_started = false;
	int record;
	unsigned count=0 ;
	char *filename_in  = NULL;
	char *filename_out = NULL;
	GLUI_Button *open  = NULL;
	GLUI_Button *start_bouton;
	GLUI_RadioGroup *control_group;
    bool debut_sim = true;
    double Td=0;
    double Si=0;
    double Sd=0;
    int mode;
    GLUI_StaticText *rate;
    GLUI_StaticText *cycle;
    GLUI_StaticText *translation;
    GLUI_StaticText *rotation;
    GLUI_Checkbox *recordi;
}

enum Widgets
{
	BUTTON_OPEN, BUTTON_SAVE, BUTTON_START_STOP, BUTTON_STEP, CHECKBOX_RECORD,
	CONTROL_MODE
};

/**
 * \brief	Initialise les fenêtres graphiques
 * \param argcp	Un pointeur sur argc à passer à glutInit()
 * \param argv	Un pointeur sur argv à passer à glutInit()
 */
void main_init_gui(int *argcp, char **argv);

/**
 * \brief	Callback réagissant à la mise à jour d'un widget.
 * \param widget	Le widget mis à jour, comme défini par Widgets.
 */
void main_widget_update(int widget);

/**
 * \brief	Callback dessinant l'état actuel de la simulation dans la fenêtre glut.
 */
void main_affichage();

/**
 * \brief	Callback de gestion des changement de taille de la fenêtre.
 * \param w	La nouvelle largeur de la fenêtre
 * \param h La nouvelle hauteur de la fenêtre
 */
void main_reshape(int w, int h);

/**
 * \brief	Fonction callback idle : effectue une seule mise à jour
 */
void main_update_one_step(void);

/**
 * \brief	Rassemble les fonctions GLUI
 * \param GL_window   id de la fenêtre GL associée
 */
 void main_create_glui_interface(int GL_window);

/**
 * \brief	Fonction main, parse la ligne de commande
 * \param argc	Nombre d'arguments.
 * \param argv	Arguments de la ligne de commande.
 */
void mouse_cb (int button, int button_state, int x, int y);
void processSpecialKeys(int key, int x, int y);
void afficher_rotation(double vrot);
void afficher_translation(double vtrans);
void afficher_rate(double taux);
void afficher_turn(int etape);

int main (int argc, char* argv[])
{	
	switch(argc)
	{
	case 3:
		if(strcmp(argv[1], "Error") == 0)
		{
			simulation_lecture(argv[2]);
			return EXIT_SUCCESS;
		}
		if(strcmp(argv[1], "Draw") == 0)
		{
			simulation_lecture(argv[2]);
			
            main_init_gui(&argc, argv);
			return EXIT_SUCCESS;
		}
		break;
	case 1:
        main_init_gui(&argc, argv);
		return EXIT_SUCCESS;
	}
	printf("Usage : %s [{Error|Draw} filename]\n", argv[0]);
	return EXIT_FAILURE;
}

void main_init_gui(int *argcp, char **argv)
{
    glutInit(argcp, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(TAILLE_INITIALE, TAILLE_INITIALE);
	view_window = glutCreateWindow("Simulation");
	
	glutDisplayFunc(main_affichage);
	glutReshapeFunc(main_reshape);

    if (debut_sim)
    {
        but_initial();
        debut_sim=false;
    }

	main_create_glui_interface(view_window); 
	GLUI_Master.set_glutMouseFunc(mouse_cb);
	glutSpecialFunc(processSpecialKeys);
	glutMainLoop();
}

void main_create_glui_interface(int GL_window)
{
	filename_in  = (char*)malloc(sizeof(GLUI_String));
	filename_out = (char*)malloc(sizeof(GLUI_String));
	*filename_in  = 0;
	*filename_out = 0;

	GLUI_Master.set_glutIdleFunc(main_update_one_step);
	
	GLUI *glui = GLUI_Master.create_glui("Control");

	GLUI_Panel *opening = glui->add_panel("Opening");
	glui->add_edittext_to_panel(opening, "File name : ", GLUI_EDITTEXT_TEXT,
								filename_in);
	open = glui->add_button_to_panel(opening, "Open", BUTTON_OPEN, main_widget_update);

	GLUI_Panel *saving = glui->add_panel("Saving");
	glui->add_edittext_to_panel(saving, "File name : ", GLUI_EDITTEXT_TEXT,
								filename_out);
	glui->add_button_to_panel(saving, "Save", BUTTON_SAVE, main_widget_update);

	glui->add_column(true);
	GLUI_Panel *simulation = glui->add_panel("Simulation");
	start_bouton=glui->add_button_to_panel(simulation, "Start", BUTTON_START_STOP,
																main_widget_update);
	glui->add_button_to_panel(simulation, "Step" , BUTTON_STEP, main_widget_update);

	GLUI_Panel *recording = glui->add_panel("Recording");
	recordi = glui->add_checkbox_to_panel(recording, "Record", &record,CHECKBOX_RECORD,
																main_widget_update);
	rate = glui->add_statictext_to_panel(recording, "Rate: 0.000");
	cycle = glui->add_statictext_to_panel(recording, "Turn: 0");

	glui->add_column(true);
	GLUI_Panel *control_mode = glui->add_panel("Control mode");
	control_group = glui->add_radiogroup_to_panel(control_mode, &mode, 
																CONTROL_MODE,
																main_widget_update);
	glui->add_radiobutton_to_group(control_group, "Automatic");
	glui->add_radiobutton_to_group(control_group, "Manual");
	control_group->set_int_val(CONTROL_AUTO);

	GLUI_Panel *robot_control = glui->add_panel("Robot control");
	translation=glui->add_statictext_to_panel(robot_control, "Translation: 0.000");
	rotation=glui->add_statictext_to_panel(robot_control, "Rotation: 0.000");

	glui->add_button("Exit", EXIT_SUCCESS, exit);

	glui->set_main_gfx_window(GL_window);	
}

void main_widget_update(int widget)
{
	switch(widget)
	{
	case BUTTON_OPEN:
		eliminer_tout();
		simulation_lecture(filename_in);
        but_initial();
        simulation_deplacement();
        // parfois si on ouvre un nouveau fichier la simulation ne commence pas
		if(glutGetWindow() != view_window)
			glutSetWindow(view_window);
		glutPostRedisplay();
		break;
		
	case BUTTON_SAVE:
		simulation_ecriture(filename_out);
		break;
		
	case BUTTON_START_STOP:
		if(!simulation_started)
		{
			simulation_started = true;
			start_bouton-> set_name("Stop");
			printf("simulation started\n");
		}
		else
		{
			simulation_started = false;
			recordi->set_int_val(0);
			start_bouton-> set_name("Start");
			printf("simulation stopped\n");
			
		}
		break;
		
	case BUTTON_STEP:
		printf("One step only\n");

		if (simulation_started==false && Td< CENT_POUR_CENT)
		{
			
			simulation_deplacement();
			if (count==0)
			{
				Si = somme_des_energies();
			}
			update_taux_decontamination (&Td, &Si, &Sd);
			count++;
			afficher_rate(Td);
			if (record)
			{
				record_ecriture(count,Td);
			}
			
			afficher_turn(count);
			glutSetWindow(view_window);
			glutPostRedisplay();
		}
		break;
		
	case CHECKBOX_RECORD:
		printf("checkbox record changed : reset turn counter\n");
		count = 0;
		FILE *fichier;
		fichier = fopen("out.dat", "w");
		fichier=NULL;
		break;
		
	case CONTROL_MODE:
		printf("Radiobutton activated: mode changed\n");
		break;
	}
}

void main_affichage()
{
	simulation_dessiner();
	glutSwapBuffers();
}

void main_reshape(int w, int h)
{
	graphic_reshape(w, h);
	glutPostRedisplay();
}

void main_update_one_step(void)
{
	if (mode== CONTROL_AUTO)
	{
		deselectionner_tout();
		glutPostRedisplay();
	}
	if (simulation_started== true && Td<CENT_POUR_CENT)
		{
			
			simulation_deplacement();
			if (count==0)
			{
				Si = somme_des_energies();
			}
			update_taux_decontamination (&Td, &Si, &Sd);
			count++;
			if (record)
			{
				record_ecriture(count,Td);
				afficher_rate(Td);
			}
			
			afficher_turn(count);
			afficher_rate(Td);
			glutSetWindow(view_window);
			glutPostRedisplay();
		}
}
void mouse_cb (int button, int button_state, int x, int y)
{
	if (mode==CONTROL_MANUEL && button_state == GLUT_DOWN && button == GLUT_LEFT_BUTTON)
	{ 	
		simulation_selectioner_robot(x, y);
		glutPostRedisplay();
	}
}

void processSpecialKeys(int key, int x, int y) 
{
	// les vitesses changent seulement si on clique sur le dessin et pas sur le gui
	for (int i=0; i<simulation_nombre_robot(); i++)
	{
		if (manual_robot(i))
		{
			switch(key)
			{
				case GLUT_KEY_LEFT:
					simulation_ajouter_vitesse_rotation(i);
					break;
				case GLUT_KEY_RIGHT:
					simulation_soustraire_vitesse_rotation(i);
					break;
				case GLUT_KEY_UP:
					simulation_augmenter_vitesse_translation(i);
					break;
				case GLUT_KEY_DOWN:
					simulation_soustraire_vitesse_translation(i);
					break;
			}

		afficher_rotation(vitesse_angle());
		afficher_translation(vitesse_transition());
		glutSetWindow(view_window);
		glutPostRedisplay();	
		}
	}
}

void afficher_rotation(double vrot)
{
	char text1[20] = "Rotation :";
	char text2[20];
	sprintf(text2, "%.3lf", vrot);
	strcat (text1, text2);
	rotation->set_name(text1);
}

void afficher_translation(double vtrans)
{
	char text1[CHAR_MAX] = "Translation :";
	char text2[CHAR_MAX];
	sprintf(text2, "%.3lf", vtrans);
	strcat (text1, text2);
	translation->set_name(text1);
}

void afficher_rate(double taux)
{
	char text1[CHAR_MAX] = "rate :";
	char text2[CHAR_MAX];
	sprintf(text2, "%.3lf", taux);
	strcat (text1, text2);
	rate->set_name(text1);
}

void afficher_turn(int etape)
{
	char text1[CHAR_MAX] = "turn :";
	char text2[CHAR_MAX];
	sprintf(text2, "%d", etape);
	strcat (text1, text2);
	cycle->set_name(text1);
}
