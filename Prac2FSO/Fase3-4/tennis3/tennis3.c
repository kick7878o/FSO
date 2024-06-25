/*****************************************************************************/
/*									     */
/*				     Tennis3.c				     */
/*									     */
/*  Programa inicial d'exemple per a les practiques 2 i 3 de FSO.	     */
/*     Es tracta del joc del tennis: es dibuixa un camp de joc rectangular   */
/*     amb una porteria a cada costat, una paleta per l'usuari, una paleta   */
/*     per l'ordinador i una pilota que va rebotant per tot arreu; l'usuari  */
/*     disposa de dues tecles per controlar la seva paleta, mentre que l'or- */
/*     dinador mou la seva automaticament (amunt i avall). Evidentment, es   */
/*     tracta d'intentar col.locar la pilota a la porteria de l'ordinador    */
/*     (porteria de la dreta), abans que l'ordinador aconseguixi col.locar   */
/*     la pilota dins la porteria de l'usuari (porteria de l'esquerra).      */
/*									     */
/*  Arguments del programa:						     */
/*     per controlar la posicio de tots els elements del joc, cal indicar    */
/*     el nom d'un fitxer de text que contindra la seguent informacio:	     */
/*		n_fil n_col m_por l_pal					     */
/*		pil_pf pil_pc pil_vf pil_vc pil_ret			     */
/*		ipo_pf ipo_pc po_vf pal_ret				     */
/*									     */
/*     on 'n_fil', 'n_col' son les dimensions del taulell de joc, 'm_por'    */
/*     es la mida de les dues porteries, 'l_pal' es la longitud de les dues  */
/*     paletes; 'pil_pf', 'pil_pc' es la posicio inicial (fila,columna) de   */
/*     la pilota, mentre que 'pil_vf', 'pil_vc' es la velocitat inicial,     */
/*     pil_ret es el percentatge respecte al retard passat per paràmetre;    */
/*     finalment, 'ipo_pf', 'ipo_pc' indicara la posicio del primer caracter */
/*     de la paleta de l'ordinador, mentre que la seva velocitat vertical    */
/*     ve determinada pel parametre 'po_fv', i pal_ret el percentatge de     */
/*     retard en el moviment de la paleta de l'ordinador.		     */
/*									     */
/*     A mes, es podra afegir un segon argument opcional per indicar el      */
/*     retard de moviment de la pilota i la paleta de l'ordinador (en ms);   */
/*     el valor d'aquest parametre per defecte es 100 (1 decima de segon).   */
/*									     */
/*  Compilar i executar:					  	     */
/*     El programa invoca les funcions definides en 'winsuport.o', les       */
/*     quals proporcionen una interficie senzilla per a crear una finestra   */
/*     de text on es poden imprimir caracters en posicions especifiques de   */
/*     la pantalla (basada en CURSES); per tant, el programa necessita ser   */
/*     compilat amb la llibreria 'curses':				     */
/*									     */
/*	   $ gcc tennis0.c winsuport.o -o tennis0 -lcurses		     */
/*	   $ tennis0 fit_param [retard]					     */
/*									     */
/*  Codis de retorn:						  	     */
/*     El programa retorna algun dels seguents codis al SO:		     */
/*	0  ==>  funcionament normal					     */
/*	1  ==>  numero d'arguments incorrecte 				     */
/*	2  ==>  fitxer no accessible					     */
/*	3  ==>  dimensions del taulell incorrectes			     */
/*	4  ==>  parametres de la pilota incorrectes			     */
/*	5  ==>  parametres d'alguna de les paletes incorrectes		     */
/*	6  ==>  no s'ha pogut crear el camp de joc (no pot iniciar CURSES)   */
/*****************************************************************************/

#include <stdio.h> /* incloure definicions de funcions estandard */
#include <sys/wait.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include "winsuport2.h"
#include "memoria.h"
#include "memoria.c"
//#include "winsuport.h" /* incloure definicions de funcions propies */
#include <time.h>

#define MIN_FIL 7 /* definir limits de variables globals */
#define MAX_FIL 25
#define MIN_COL 10
#define MAX_COL 80
#define MIN_PAL 3
#define MIN_VEL -1.0
#define MAX_VEL 1.0
//#define MIN_RET 0.0
//#define MAX_RET 5.0

// Numero maximo de paletas
#define MAX_PALETA 9

// Maximo de hilos (pal_usuario [1] + pelota [1])
#define MAX_THREADS 2

// Estructura de la Paleta
typedef struct {
	int ipo_pf, ipo_pc; /* posicio del la paleta de l'ordinador */
	float v_pal;		/* velocitat de la paleta del programa */
	float po_pf;		/* pos. vertical de la paleta de l'ordinador, en valor real */
} Paleta;

//float pal_ret;		  /* percentatge de retard de la paleta */
Paleta paletas[MAX_PALETA]; // Tabla de paletas del ordenador
pthread_t tid[MAX_THREADS]; // Tabla de hilos independientes
int numPaletas = 0; // Numero de paletas creadas
int tecla=0, contador = -1; // tecla pulsada y estado de la partida

// Creamos dos semaforos de exclusión mutua
pthread_mutex_t mutex_global = PTHREAD_MUTEX_INITIALIZER; // Para las var globales
pthread_mutex_t mutex_pantalla = PTHREAD_MUTEX_INITIALIZER; // Para la pantalla

pid_t tpid[MAX_PALETA]; // Identificador de los procesos hijo
int id_win, id_nMoviments, id_tecla, id_cont;
int *p_win, *p_nMoviments, *p_tecla, *p_cont;

/* variables globals */
int n_fil, n_col, m_por; /* dimensions del taulell i porteries */
int l_pal;					 /* longitud de les paletes */

int ipu_pf, ipu_pc; /* posicio del la paleta d'usuari */

int ipil_pf, ipil_pc; /* posicio de la pilota, en valor enter */
float pil_pf, pil_pc; /* posicio de la pilota, en valor real */
float pil_vf, pil_vc; /* velocitat de la pilota, en valor real*/
//float pil_ret;			 /* percentatge de retard de la pilota */ inservible

int retard;		/* valor del retard de moviment, en mil.lisegons */
int moviments; /* numero max de moviments paletes per acabar el joc */

/* funcio per realitzar la carrega dels parametres de joc emmagatzemats */
/* dins un fitxer de text, el nom del qual es passa per referencia en   */
/* 'nom_fit'; si es detecta algun problema, la funcio avorta l'execucio */
/* enviant un missatge per la sortida d'error i retornant el codi per-	*/
/* tinent al SO (segons comentaris del principi del programa).		*/
void carrega_parametres(const char *nom_fit) {
	FILE *fit;

	fit = fopen(nom_fit, "rt"); /* intenta obrir fitxer */

	if (fit == NULL) { // Si no se abre correctamente el fichero
		fprintf(stderr, "No s'ha pogut obrir el fitxer \'%s\'\n", nom_fit);
		exit(2);
	}

	/** Primera línea del fichero ->
	 * n_fil, n_col: dimensiones del tablero de juego
	 * m_por: medida de las dos porterias
	 * l_pal: longitud de las paletas
	*/
	if (!feof(fit))
		fscanf(fit, "%d %d %d %d\n", &n_fil, &n_col, &m_por, &l_pal);

	// Comprobamos que los parámetros especificados están correctamente delimitados
	if ((n_fil < MIN_FIL) || (n_fil > MAX_FIL) ||
		 (n_col < MIN_COL) || (n_col > MAX_COL) || 
		 (m_por < 0		  ) || (m_por > n_fil - 3) || 
		 (l_pal < MIN_PAL) || (l_pal > n_fil - 3))
	{
		fprintf(stderr, "Error: dimensions del camp de joc incorrectes:\n");
		fprintf(stderr, "\t%d =< n_fil (%d) =< %d\n", MIN_FIL, n_fil, MAX_FIL);
		fprintf(stderr, "\t%d =< n_col (%d) =< %d\n", MIN_COL, n_col, MAX_COL);
		fprintf(stderr, "\t0 =< m_por (%d) =< n_fil-3 (%d)\n", m_por, (n_fil - 3));
		fprintf(stderr, "\t%d =< l_pal (%d) =< n_fil-3 (%d)\n", MIN_PAL, l_pal, (n_fil - 3));
		fclose(fit);
		exit(3);
	}

	/** Segunda línea del fichero para la pelota:
	 * ipil_pf, ipil_pc: posición inicial de la pelota.
	 * pil_vf, pil_vc: velocidad inicial de la pelota.
	*/
	if (!feof(fit))
		fscanf(fit, "%d %d %f %f\n", &ipil_pf, &ipil_pc, &pil_vf, &pil_vc);
		
	if ((ipil_pf < 1		 ) || (ipil_pf > n_fil - 3) ||
		 (ipil_pc < 1		 ) || (ipil_pc > n_col - 2) ||
		 (pil_vf  < MIN_VEL) || (pil_vf  > MAX_VEL  ) ||
		 (pil_vc  < MIN_VEL) || (pil_vc  > MAX_VEL  ))
	{
		fprintf(stderr, "Error: parametre pilota incorrectes:\n");
		fprintf(stderr, "\t1 =< ipil_pf (%d) =< n_fil-3 (%d)\n", ipil_pf, (n_fil - 3));
		fprintf(stderr, "\t1 =< ipil_pc (%d) =< n_col-2 (%d)\n", ipil_pc, (n_col - 2));
		fprintf(stderr, "\t%.1f =< pil_vf (%.1f) =< %.1f\n", MIN_VEL, pil_vf, MAX_VEL);
		fprintf(stderr, "\t%.1f =< pil_vc (%.1f) =< %.1f\n", MIN_VEL, pil_vc, MAX_VEL);
		//fprintf(stderr, "\t%.1f =< pil_ret (%.1f) =< %.1f\n", MIN_RET, pil_ret, MAX_RET);
		fclose(fit);
		exit(4);
	}

	// Bucle que recorre el resto de líneas con los parámetros de las paletas
	while (!feof(fit) && (numPaletas < MAX_PALETA)) {
		/** Resto de líneas del fichero para las diferentes paletas
		 * ipo_pf, ipo_pc: posición del primer carácter del ordenador
		 * v_pal: velocidad vertical de la paleta
		*/
		fscanf(fit, "%d %d %f\n", &paletas[numPaletas].ipo_pf, &paletas[numPaletas].ipo_pc, 
											  &paletas[numPaletas].v_pal);

		if ((paletas[numPaletas].ipo_pf  < 1) 		  || (paletas[numPaletas].ipo_pf + l_pal > n_fil - 2) ||
			 (paletas[numPaletas].ipo_pc  < 5) 		  || (paletas[numPaletas].ipo_pc 		  > n_col - 2) ||
			 (paletas[numPaletas].v_pal   < MIN_VEL) || (paletas[numPaletas].v_pal 			  > MAX_VEL))
			 //(paletas[numPaletas].pal_ret < MIN_RET) || (paletas[numPaletas].pal_ret 		  > MAX_RET))
		{
			fprintf(stderr, "Error: parametres paleta ordinador incorrectes:\n");
			fprintf(stderr, "\t1 =< ipo_pf (%d) =< n_fil-l_pal-3 (%d)\n", paletas[numPaletas].ipo_pf, (n_fil - l_pal - 3));
			fprintf(stderr, "\t5 =< ipo_pc (%d) =< n_col-2 (%d)\n", paletas[numPaletas].ipo_pc, (n_col - 2));
			fprintf(stderr, "\t%.1f =< v_pal (%.1f) =< %.1f\n", MIN_VEL, paletas[numPaletas].v_pal, MAX_VEL);
			//fprintf(stderr, "\t%.1f =< pal_ret (%.1f) =< %.1f\n", MIN_RET, paletas[numPaletas].pal_ret, MAX_RET);
			fclose(fit);
			exit(5);
		} else 
			numPaletas++; // Nueva paleta creada correctamente
	}


	fclose(fit); /* fitxer carregat: tot OK! */
}

/* funcio per inicialitar les variables i visualitzar l'estat inicial del joc */
int inicialitza_joc(void)
{
	int i_port, f_port, retwin;
	char strin[51];

	retwin = win_ini(&n_fil, &n_col, '+', INVERS); /* intenta crear taulell */

	if (retwin < 0) { /* si no pot crear l'entorn de joc amb les curses */
		fprintf(stderr, "Error en la creacio del taulell de joc:\t");

		switch (retwin) {
			case -1: 
				fprintf(stderr, "camp de joc ja creat!\n"); break;

			case -2:
				fprintf(stderr, "no s'ha pogut inicialitzar l'entorn de curses!\n"); break;

			case -3:
				fprintf(stderr, "les mides del camp demanades son massa grans!\n"); break;

			case -4:
				fprintf(stderr, "no s'ha pogut crear la finestra!\n"); break;
		}
		return (retwin);
	}

	// Creamos las zonas de memoria compartida y obtenemos sus direcciones
	id_win = ini_mem(retwin);
	p_win = map_mem(id_win);

	id_nMoviments = ini_mem(sizeof(int));
	p_nMoviments = map_mem(id_nMoviments);

	id_tecla = ini_mem(sizeof(char));
	p_tecla = map_mem(id_tecla);
	*p_tecla = 0; // Asignamos un valor

	id_cont = ini_mem(sizeof(int));
	p_cont = map_mem(id_cont);
	*p_cont = -1; // Asignamos un valor

	win_set(p_win, n_fil, n_col);

	/* crea els forats de la porteria */
	i_port = n_fil / 2 - m_por / 2;
	if (n_fil % 2 == 0) i_port--;

	if (i_port == 0) i_port = 1;

	f_port = i_port + m_por - 1;

	for (int i = i_port; i <= f_port; i++) {
		win_escricar(i, 0, ' ', NO_INV);
		win_escricar(i, n_col - 1, ' ', NO_INV);
	}

	ipu_pf = n_fil / 2;
	ipu_pc = 3; /* inicialitzar pos. paletes */

	if (ipu_pf + l_pal >= n_fil - 3)
		ipu_pf = 1;

	for (int i = 0; i < l_pal; i++) { /* dibuixar paleta inicialment */
		win_escricar(ipu_pf + i, ipu_pc, '0', INVERS);
		// win_escricar(ipo_pf + i, ipo_pc, '1', INVERS); inservible
	}

	for (int j=0; j < numPaletas; j++) {
		for (int i = 0; i < l_pal; i++) {
			win_escricar(paletas[j].ipo_pf+i, paletas[j].ipo_pc, (j+1+'0'), INVERS);
		}
		paletas[j].po_pf = paletas[j].ipo_pf; // Fijar valor real paleta ordenador
	}
	
	pil_pf = ipil_pf;
	pil_pc = ipil_pc; /* fixar valor real posicio pilota */
	win_escricar(ipil_pf, ipil_pc, '.', INVERS); /* dibuix inicial pilota */

	sprintf(strin, "Tecles: \'%c\'-> amunt, \'%c\'-> avall, RETURN-> sortir.", TEC_AMUNT, TEC_AVALL);
	win_escristr(strin);
	return (0);
}

/* funcio per moure la pilota; retorna un valor amb alguna d'aquestes	*/
/* possibilitats:							*/
/*	-1 ==> la pilota no ha sortit del taulell			*/
/*	 0 ==> la pilota ha sortit per la porteria esquerra		*/
/*	>0 ==> la pilota ha sortit per la porteria dreta		*/
//int moure_pilota(void) {
void * moure_pilota(void * cap) {
	int f_h, c_h /*result*/;
	char rh, rv, rd /*pd*/;

	// Bucle para generar el movimiento de la pelota hasta que haya salido la pelota o se presione RETURN
	do {

			win_retard(retard);
			f_h = pil_pf + pil_vf; /* posicio hipotetica de la pilota */
			c_h = pil_pc + pil_vc;
			// result = -1; /* inicialment suposem que la pilota no surt */
			rh = rv = rd = /*pd =*/' ';

			/* si posicio hipotetica no coincideix amb la pos. actual */
			if ((f_h != ipil_pf) || (c_h != ipil_pc)) {

				if (f_h != ipil_pf) { /* provar rebot vertical */
					pthread_mutex_lock(&mutex_pantalla); // Accedemos a la pantalla correctamente
					rv = win_quincar(f_h, ipil_pc); /* veure si hi ha algun obstacle */
					
					pthread_mutex_unlock(&mutex_pantalla); // Desbloqueamos despues de acceder
					if (rv != ' ') { /* si no hi ha res */
						pil_vf = -pil_vf;		  /* canvia velocitat vertical */
						f_h = pil_pf + pil_vf; /* actualitza posicio hipotetica */
					}
				}

				if (c_h != ipil_pc) { /* provar rebot horitzontal */
					pthread_mutex_lock(&mutex_pantalla); // Accedemos a la pantalla correctamente
					rh = win_quincar(ipil_pf, c_h); /* veure si hi ha algun obstacle */
					pthread_mutex_unlock(&mutex_pantalla); // Desbloqueamos despues de acceder

					if (rh != ' ') { /* si no hi ha res */
						pil_vc = -pil_vc;		  /* canvia velocitat horitzontal */
						c_h = pil_pc + pil_vc; /* actualitza posicio hipotetica */
					}
				}

				if ((f_h != ipil_pf) && (c_h != ipil_pc)) { /* provar rebot diagonal */
					pthread_mutex_lock(&mutex_pantalla); // Accedemos a la pantalla correctamente				
					rd = win_quincar(f_h, c_h);
					pthread_mutex_unlock(&mutex_pantalla); // Desbloqueamos despues de acceder

					if (rd != ' ') { /* si no hi ha obstacle */
						pil_vf = -pil_vf;
						pil_vc = -pil_vc; /* canvia velocitats */
						f_h = pil_pf + pil_vf;
						c_h = pil_pc + pil_vc; /* actualitza posicio entera */
					}
				}

				pthread_mutex_lock(&mutex_pantalla); // Accedemos a la pantalla correctamente

				if (win_quincar(f_h, c_h) == ' ') { /* verificar posicio definitiva */
					/* si no hi ha obstacle */
					win_escricar(ipil_pf, ipil_pc, ' ', NO_INV); /* esborra pilota */
					pthread_mutex_unlock(&mutex_pantalla); // Desbloqueamos despues de acceder

					pil_pf += pil_vf;
					pil_pc += pil_vc;
					ipil_pf = f_h;
					ipil_pc = c_h; /* actualitza posicio actual */

					if ((ipil_pc > 0) && (ipil_pc <= n_col)) {/* si no surt */
						pthread_mutex_lock(&mutex_pantalla); // Accedemos a la pantalla correctamente
						win_escricar(ipil_pf, ipil_pc, '.', INVERS); /* imprimeix pilota */
						pthread_mutex_unlock(&mutex_pantalla); // Desbloqueamos despues de acceder
					} else
						*p_cont = ipil_pc; // Codigo del fin de partido
						// result = ipil_pc; /* codi de finalitzacio de partida */
				} else
					pthread_mutex_unlock(&mutex_pantalla); // Desbloqueamos despues de acceder
				
			} else {
				pil_pf += pil_vf;
				pil_pc += pil_vc;
			}

			pthread_mutex_lock(&mutex_global); // Accedemos a la variables correctamente

			// Si quedan movimientos o estan desactivados, simplemente desbloqueamos
			if (*p_nMoviments == -1 || *p_nMoviments > 0)
				pthread_mutex_unlock(&mutex_global);

			else { // Si no quedan movimientos o no estan desactivados
				pthread_mutex_unlock(&mutex_global);
				pthread_exit(0);
			}
	} while ((*p_tecla != TEC_RETURN) && (*p_cont == -1));
	pthread_exit(0); // Terminar el hilo llamado (el de la pelota)
	// return (result);
}

/* funcio per moure la paleta de l'usuari en funcio de la tecla premuda */
//void mou_paleta_usuari(int tecla) {
void * mou_paleta_usuari(void * cap) {
	do {
		win_retard(retard);

		pthread_mutex_lock(&mutex_global);
		*p_tecla = win_gettec();
		pthread_mutex_lock(&mutex_pantalla);

		// Mover hacia 'ABAJO' la paleta del usuario
		if ((*p_tecla == TEC_AVALL) && (win_quincar(ipu_pf + l_pal, ipu_pc) == ' ')) {
			win_escricar(ipu_pf, ipu_pc, ' ', NO_INV);				 /* esborra primer bloc */
			ipu_pf++;												/* actualitza posicio */
			win_escricar(ipu_pf + l_pal - 1, ipu_pc, '0', INVERS); /* impri. ultim bloc */
		}

		// Mover hacia 'ARRIBA' la paleta del usuario
		if ((*p_tecla == TEC_AMUNT) && (win_quincar(ipu_pf - 1, ipu_pc) == ' ')) {
			win_escricar(ipu_pf + l_pal - 1, ipu_pc, ' ', NO_INV); /* esborra ultim bloc */
			ipu_pf--;											/* actualitza posicio */
			win_escricar(ipu_pf, ipu_pc, '0', INVERS);			/* imprimeix primer bloc */
		}

		pthread_mutex_unlock(&mutex_pantalla);

		if (*p_nMoviments == -1)  // Si está desactivado los movimientos, continuamos
			pthread_mutex_unlock(&mutex_global);

		else { // Miramos si quedan movimientos
			if (*p_nMoviments > 0) {
				if ((*p_tecla == TEC_AMUNT) || (*p_tecla == TEC_AVALL))
					*p_nMoviments = *p_nMoviments -1;

				pthread_mutex_unlock(&mutex_global);

			} else {
				pthread_mutex_unlock(&mutex_global);
				pthread_exit(0);
			}
		}
	
	} while ((*p_tecla != TEC_RETURN) && (*p_cont == -1));

	pthread_exit(0); // Terminar hilo del usuario
}

/* funcio per moure la paleta de l'ordinador autonomament, en funcio de la */
/* velocitat de la paleta (variable global v_pal) */
//void mou_paleta_ordinador(void)
//void * mou_paleta_ordinador(void * index) {
//	int f_h;
//   int index_th = (intptr_t)index;
//	/* char rh,rv,rd; */
//	do {
//		
//
//			win_retard(retard);
//			/* posicio hipotetica de la paleta */
//			f_h = paletas[index_th].po_pf + paletas[index_th].v_pal;
//
//			/* si pos. hipotetica no coincideix amb pos. actual */
//			if (f_h != paletas[index_th].ipo_pf) {
//
//				if (paletas[index_th].v_pal > 0.0) { /* verificar moviment cap avall */
//
//					pthread_mutex_lock(&mutex_pantalla);
//					/* si no hi ha obstacle */
//
//					if (win_quincar(f_h + l_pal - 1, paletas[index_th].ipo_pc) == ' ') {
//						/* esborra primer bloc */
//						win_escricar(paletas[index_th].ipo_pf, paletas[index_th].ipo_pc, ' ', NO_INV);
//						paletas[index_th].po_pf += paletas[index_th].v_pal;
//						paletas[index_th].ipo_pf = paletas[index_th].po_pf; /* actualitza posicio */
//
//						/* impr. ultim bloc */
//						win_escricar(paletas[index_th].ipo_pf + l_pal - 1, paletas[index_th].ipo_pc, '1', INVERS);
//
//						pthread_mutex_unlock(&mutex_pantalla);
//
//					} else  {/* si hi ha obstacle, canvia el sentit del moviment */
//						pthread_mutex_unlock(&mutex_pantalla);
//
//						paletas[index_th].v_pal = -paletas[index_th].v_pal;
//					}
//				} else { /* verificar moviment cap amunt */
//					pthread_mutex_lock(&mutex_pantalla);
//
//					if (win_quincar(f_h, paletas[index_th].ipo_pc) == ' ') { /* si no hi ha obstacle */
//
//						/* esbo. ultim bloc */
//						win_escricar(paletas[index_th].ipo_pf + l_pal - 1, paletas[index_th].ipo_pc, ' ', NO_INV);
//
//						paletas[index_th].po_pf += paletas[index_th].v_pal;
//						paletas[index_th].ipo_pf = paletas[index_th].po_pf; /* actualitza posicio */
//
//						/* impr. primer bloc */
//						win_escricar(paletas[index_th].ipo_pf, paletas[index_th].ipo_pc, '1', INVERS);
//
//						pthread_mutex_unlock(&mutex_pantalla);
//					} else {/* si hi ha obstacle, canvia el sentit del moviment */
//						pthread_mutex_unlock(&mutex_pantalla);
//
//						paletas[index_th].v_pal = -paletas[index_th].v_pal;
//					}
//				}
//			} else /* actualitza posicio vertical real de la paleta */
//
//				paletas[index_th].po_pf += paletas[index_th].v_pal;
//
//			pthread_mutex_lock(&mutex_global);
//
//			if (moviments == -1)  // Si está desactivado los movimientos, continuamos
//				pthread_mutex_unlock(&mutex_global);
//
//			else { // Miramos si quedan movimientos
//				if (moviments > 0) {
//					moviments--;
//					pthread_mutex_unlock(&mutex_global);
//
//				} else {
//					pthread_mutex_unlock(&mutex_global);
//					pthread_exit(0);
//				}
//			}	
//
//	} while ((tecla != TEC_RETURN) && (contador == -1));
//
//	pthread_exit(0);
//}


/* programa principal */
int main(int n_args, const char *ll_args[]) {
	//int tec, cont; /* variables locals */

	int min, seg; // Contador de minutos y segundos
	time_t inicio, final;
	char buffer[51];

	if ((n_args != 3) && (n_args != 4)) {
		fprintf(stderr, "Comanda: tennis3 fit_param moviments [retard]\n");
		exit(1);
	}

	carrega_parametres(ll_args[1]); // Cargamos parametros del campo
	moviments = atoi(ll_args[2]); // Obtenemos el numero de movimientos maximo

	if (moviments == 0) moviments = -1; // Desactivamos movimientos

	//fprintf(stderr, "Va hasta aqui?\n\n"); 

	if (n_args == 4) // Miramos si se especifica un retraso
		retard = atoi(ll_args[3]);
	else
		retard = 100;

	if (inicialitza_joc() != 0) /* intenta crear el taulell de joc */
		exit(4);						 /* aborta si hi ha algun problema amb taulell */

	*p_nMoviments = moviments;

	// Iniciamos los semaforos mutex
	pthread_mutex_init(&mutex_global, NULL);
	pthread_mutex_init(&mutex_pantalla, NULL);

	// Creamos hilo para el usuario, la pelota y la tecla espacio
	pthread_create(&tid[0], NULL, mou_paleta_usuari, NULL);
	pthread_create(&tid[1], NULL, moure_pilota, NULL);

	char str_n_fil[3], str_n_col[3], str_l_pal[3], str_ipo_pf[4], str_ipo_pc[4];
	char str_po_pf[4], str_v_pal[4], str_id_win[20], str_id_nMoviments[20];
	char str_id_tecla[20], str_id_cont[20], str_index[1];

	sprintf(str_n_fil, "%i", n_fil);
	sprintf(str_n_col, "%i", n_col);
	sprintf(str_l_pal, "%i", l_pal);
	sprintf(str_id_win, "%i", id_win);
	sprintf(str_id_nMoviments, "%i", id_nMoviments);
	sprintf(str_id_tecla, "%i", id_tecla);
	sprintf(str_id_cont, "%i", id_cont);

	int nHijos = 0;
	for (int i = 0; i < numPaletas; i++) {
		tpid[nHijos] = fork(); /* Creamos un nuevo proceso */

		if (tpid[nHijos] == (pid_t) 0) { /* Rama del hijo */
			sprintf(str_ipo_pf, "%i", paletas[i].ipo_pf);
			sprintf(str_ipo_pc, "%i", paletas[i].ipo_pc);
			sprintf(str_po_pf, "%f", paletas[i].po_pf);
			sprintf(str_v_pal, "%f", paletas[i].v_pal);
			sprintf(str_index, "%i", i+1);
			sprintf(str_id_cont, "%i", id_cont);

			execlp("./pal_ord3", "pal_ord3", str_n_fil, str_n_col, str_l_pal,
				str_ipo_pf, str_ipo_pc, str_po_pf, str_v_pal, str_id_win,
				str_id_nMoviments, str_id_tecla, str_id_cont, str_index, (char *) 0);

			fprintf(stderr, "Error: Inejecutable el proceso hijo \'pal_ord3\'\n");
			exit(0);
		} else if (tpid[nHijos] > 0) nHijos++; /* Rama del padre */
	}

	inicio = time(NULL); // Empezamos a contar

	do { /********** bucle principal del joc **********/
		final = time(NULL);
		seg = difftime(final, inicio);
		min = seg / 60;
		seg = seg % 60;

		// Si los movimientos están desactivados
		if (*p_nMoviments == -1) {
			sprintf(buffer, "Temps: %2d:%2d", min, seg);
	
			pthread_mutex_lock(&mutex_pantalla);
			win_escristr(buffer);
			pthread_mutex_unlock(&mutex_pantalla);
		} else {
			sprintf(buffer, "MovFets: %d MovRestants: %d Temps: %2d:%2d", (moviments - *p_nMoviments), *p_nMoviments, min, seg);

			pthread_mutex_lock(&mutex_pantalla);
			win_escristr(buffer);
			pthread_mutex_unlock(&mutex_pantalla);
		}

		// Condicional para entrar en modo pausa del juego
		if (*p_tecla == TEC_ESPAI) {
			pthread_mutex_lock(&mutex_pantalla);
			pthread_mutex_lock(&mutex_global);
			*p_tecla = 0;

			// Bucle para seguir mostrando el tiempo correctamente pero con pantalla congelada
			while (*p_tecla != TEC_ESPAI) {
				win_retard(retard);
				time_t current_time = time(NULL);
				time_t diff_time = current_time - inicio;
				int minutos = diff_time / 60;
				int segundos = diff_time % 60;
				*p_tecla = win_gettec();
				win_update();
				if (*p_nMoviments == -1) {
					sprintf(buffer, "Temps: %2d:%2d", minutos, segundos);
					win_escristr(buffer);
				} else {
					sprintf(buffer, "MovFets: %d MovRestants: %d Temps: %2d:%2d", (moviments - *p_nMoviments), *p_nMoviments, minutos, segundos);
					win_escristr(buffer);
				}
			}
			pthread_mutex_unlock(&mutex_pantalla);
			pthread_mutex_unlock(&mutex_global);
		}

		win_update();
		win_retard(retard);
	} while ((*p_tecla != TEC_RETURN) && (*p_cont == -1) && 
			  ((*p_nMoviments > 0) || *p_nMoviments == -1));

	// Eliminamos las paletas creadas para el usuario y pelota
	for (int i = 0; i < MAX_THREADS; i++)
		pthread_join(tid[i], NULL);	
	
	pthread_mutex_destroy(&mutex_pantalla);
	pthread_mutex_destroy(&mutex_global);

	win_fi(); // Limpiamos el tablero

	if (*p_tecla == TEC_RETURN)
		printf("S'ha aturat el joc amb la tecla RETURN!\n");

	else {
		if (*p_cont == 0 || *p_nMoviments == 0)
			printf("Ha guanyat l'ordinador!\n");
		else
			printf("Ha guanyat l'usuari!\n");
	}

	// Eliminar zona de memoria compartida
	elim_mem(id_win);
	elim_mem(id_nMoviments);
	elim_mem(id_cont);
	elim_mem(id_tecla);

	return (0);
}
