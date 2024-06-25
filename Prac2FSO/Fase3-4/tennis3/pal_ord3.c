/*****************************************************************************/
/*				     			    pal_ord3.c      	                     */
/*****************************************************************************/

#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include "memoria.h"
#include "memoria.c"
#include "winsuport2.h"

// Estructura básica de la paleta
typedef struct{
  int ipo_pf, ipo_pc;   /* posicio del la paleta de l'ordinador */
  float po_pf;          /* pos. vertical de la paleta de l'ordinador, en valor real */
  float v_pal;      	/* velocitat de la paleta del programa */
  float pal_ret;
} Paleta;

int nFil, nCol, lPal, id_nMoviments, id_tecla, id_cont;
int *p_win, *p_n_moviments, *p_tecla, *p_cont;
Paleta paleta;

int main(int argc, char const *argv[]) {
    int f_h;

    // Obtenemos el tamaño del tablero y paleta
    nFil = atoi(argv[1]);
    nCol = atoi(argv[2]);
    lPal = atoi(argv[3]);

    // Cargamos la info. de la paleta
    paleta.ipo_pf = atoi(argv[4]);
    paleta.ipo_pc = atoi(argv[5]);
    paleta.po_pf = atof(argv[6]);
    paleta.v_pal = atof(argv[7]);

    // Reservamos espacio compartido
    p_win = map_mem(atoi(argv[8]));
    p_n_moviments = map_mem(atoi(argv[9]));
    p_tecla = map_mem(atoi(argv[10]));
    p_cont = map_mem(atoi(argv[11]));
    
    win_set(p_win, nFil, nCol);

    do {
    	
        f_h = paleta.po_pf + paleta.v_pal; /* Posición hipotética de la paleta*/

        // Si la hipotetica no coincide con la actual
        if (f_h != paleta.ipo_pf) {
            if (paleta.v_pal > 0.0) { /* Verificar movimiento hacia abajo */
                if (win_quincar(f_h + lPal-1, paleta.ipo_pc) == ' ') { /* Si no hay obstaculo */
                    win_escricar(paleta.ipo_pf, paleta.ipo_pc, ' ', NO_INV); /*  esborra primer bloc*/
                    paleta.po_pf += paleta.v_pal; 
                    paleta.ipo_pf = paleta.po_pf;   /* actualitza posicio */
                    /* impr. ultim bloc */
                    win_escricar(paleta.ipo_pf + lPal-1, paleta.ipo_pc, argv[12][0], INVERS);

                } else /* Si hay obstaculo cambia el sentido del movimiento */
                    paleta.v_pal = -paleta.v_pal;

            } else { /* Verificar movimiento hacia arriba */
                if (win_quincar(f_h, paleta.ipo_pc) == ' ') { /* Sin obstaculo */
                    /* esbo. ultim bloc */
                    win_escricar(paleta.ipo_pf + lPal-1, paleta.ipo_pc, ' ', NO_INV); 
                    paleta.po_pf += paleta.v_pal; 
                    paleta.ipo_pf = paleta.po_pf;   /* actualitza posicio */
                    /* impr. primer bloc */
                    win_escricar(paleta.ipo_pf, paleta.ipo_pc, argv[12][0], INVERS);

                } else /* Con obstaculo, cambiamos el sentido de movimiento */
                    paleta.v_pal = -paleta.v_pal;
            }

        } else paleta.po_pf += paleta.v_pal; /* Actualizamos pos vertical real de la paleta */

        /* Actualizamos numero de movimientos restantes */
        //if (*p_n_moviments > 0) *p_n_moviments = *p_n_moviments - 1;
                
    } while ((*p_tecla != TEC_RETURN) && (*p_cont == -1) && (*p_n_moviments > 0));    
}
