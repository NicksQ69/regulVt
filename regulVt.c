/********************************************************/
/*	 Réglage de la vitesse du Ventilateur par PWM		*/
/********************************************************/



// Headers

#include <wiringPi.h>			// I/O du Raspberry
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <gtk/gtk.h>			// bibliothèque graphique


// Déclarations des variables globales

int _PWM_ON = 0;
int _COMPTEUR = 0;
	




void initPin (void) {
	
	wiringPiSetupGpio() ;
	
	pinMode(15,INPUT) ;					// GPIO 15 : entrée logique
	
	pinMode(18,PWM_OUTPUT) ;		// GPIO 18 : sortie pwm
	
	pinMode(24,OUTPUT) ;		// GPIO 24 : sortie logique pour Tests
		
	}


void initPWM (void) {
	
	pwmSetMode (PWM_MODE_MS);		// PWM in mark-space mode
	
	pwmSetRange (256);				// 1024 par défaut
	
	pwmSetClock (3);				// clock divisor
	
		
	/* pwmFrequency in Hz = 19.2e6 Hz / pwmClock / pwmRange
	 * Target Freq Pour Vt Noctua PWM : 19.2e6 / 3 / 256 = 25 kHz */
	
	pwmWrite (18, 0);				//  ?? Cf data Sheet

	}


double temperatureCPU (void) {
	
	FILE *temperatureFile;
	
	double T;
	
	temperatureFile = fopen ("/sys/class/thermal/thermal_zone0/temp", "r") ;
	
//	if (temperatureFile == NULL)
//	printf ("Pb mesure T\n") ; //print some message
	
	fscanf (temperatureFile, "%lf", &T);
	T /= 1000;
	
//	printf ("The temperature is %6.3f C.\n", T);
	
	fclose (temperatureFile);
	
	return (T);
	
	}


gint fct_regulation (void *label) {
	

	double Temp_CPU;
	double cons_Vt;
	char buffer[256];
	
	digitalWrite(24,HIGH) ;		// pour TEST
	
	
	
	_COMPTEUR = _COMPTEUR + 1;
	
	
	/* Lecture de la température CPU */
	Temp_CPU = temperatureCPU();

	/* Régulation de la température par action sur
	 * la vitesse du ventilateur (PWM) */
	
	if(_COMPTEUR > 3) {
		if(Temp_CPU > 50) {
			_PWM_ON = _PWM_ON + 20;
			_COMPTEUR = 0;
			} else if(Temp_CPU < 40) {
			_PWM_ON = _PWM_ON - 10;
			_COMPTEUR = 0;
			} else _COMPTEUR = _COMPTEUR - 1;
		}
	
	/* Bornes du rapport cyclique */
	if(_PWM_ON > 256) _PWM_ON = 256;
	if(_PWM_ON < 20) _PWM_ON = 0;			

	
	/* Application du nouveau rapport cyclique */
	pwmWrite (18, _PWM_ON);

		
	/* Consigne N Vt en % */
	cons_Vt = (float) ((_PWM_ON * 100) / 256);

	
	/* Acquisition de la vitesse du ventilateur */
	
	

	
	
		
	/* Mise à jour de l'affichage */				
	sprintf(buffer, "Température CPU : %.1f °C \nrapport cyclique PWM Vt : %.0f %%\nvitesse Vt : xxx rpm" , Temp_CPU, cons_Vt);	
	gtk_label_set_text(GTK_LABEL(label),buffer);


	digitalWrite(24,LOW) ;		// pour TEST
	
	return TRUE;	
	
	}



int main (int argc, char *argv[]) {

	


/* Initialisation Hardware
 * *********************** */
 

	initPin() ;				// Initialisation des pins
	
	initPWM() ;				// Initialisation du PWM
	
	

/*	Partie Graphique
 *  **************** */	
	
	/* Initialisation de GTK+ */
	gtk_init (&argc, &argv);
	
	/* Déclaration des Widgets */
	GtkWidget *window;
	GtkWidget *label;
	

	/* Creation de la fenetre principale de l'application */	
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "T° CPU Raspberry Pi");
	gtk_window_set_default_size(GTK_WINDOW(window), 300, 120);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_signal_connect (GTK_OBJECT (window), "destroy", GTK_SIGNAL_FUNC (gtk_main_quit ), NULL);

	/* Affichage de la fenetre principale */
	gtk_widget_show (window);

	/* Création du Widget label */ 	
	label = gtk_label_new ("      v1.0   by Daddy");
	gtk_misc_set_alignment( GTK_MISC(label), 0.4, 0.4 );
	gtk_container_add (GTK_CONTAINER (window), label);
	gtk_widget_show (label);						

	/* Lancement de la boucle événementielle */
	gtk_timeout_add (3000, fct_regulation, label);
	
	/* Lancement de la boucle principale */	
	gtk_main ();
	
}





/*	commande de compilation :
 *  gcc regulVt.c -o regulVt `pkg-config --cflags --libs gtk+-2.0` -lwiringPi -lpthread
 *  lancer l'exécutable avec sudo ./regulVt */
 
 
 
 /*	Pourlancer le programme au démarrage du système :
  * Ajouter la ligne :
  * sudo /home/pi/Documents/regulVt
  * dans le fichier /etc/rc.local avant exit(0) */
