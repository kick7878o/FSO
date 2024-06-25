#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>

#include "memoria.h"
#include "winsuport2.h"
#include "semafor.h"
#include "missatge.h"
#include <stdbool.h>

typedef struct{
  int ipo_pf, ipo_pc;       /* posicio del la paleta de l'ordinador */
  float po_pf;  /* pos. vertical de la paleta de l'ordinador, en valor real */
  float v_pal;      /* velocitat de la paleta del programa */
} Paleta;

int n_fil, n_col, l_pal, id_n_mov, id_tecla, id_cont, retard, id_bustias, numPaletas;
Paleta paleta;
int *p_win, *p_n_mov, *p_tecla, *p_cont, *p_bustias;

int id_semafor_pantalla, id_semafor_mov;

// Metodo que recorre la cola hasta el limite y verifica si contiene la paleta especificada
bool hayColaPaletas(int cola[], int paleta, int limite){
  int i = 0;
  bool hay = false;
  while(i < limite){
    if(cola[i] == paleta)
    {
    	hay = true; // Si encuentra la paleta en la cola, establece la variable a true
    }
    i++;
  }
  return hay;
}

int main(int n_args, char *ll_args[])
{
  int f_h, indice, des;
  char pos, mis[4], mis_aux[4];
  bool mover;

  // Inicializa las variables globales con los argumentos pasados al programa
  n_fil = atoi(ll_args[1]);
  n_col = atoi(ll_args[2]);
  l_pal = atoi(ll_args[3]);

  paleta.ipo_pf = atoi(ll_args[4]);
  paleta.ipo_pc = atoi(ll_args[5]);
  paleta.po_pf = atof(ll_args[6]);
  paleta.v_pal = atof(ll_args[7]);
  
  // Mapea las memoria compartidas
  p_win = map_mem(atoi(ll_args[8]));
  p_n_mov = map_mem(atoi(ll_args[9]));
  p_tecla = map_mem(atoi(ll_args[10]));
  p_cont = map_mem(atoi(ll_args[11]));

  indice = (atoi(ll_args[12]))-1;
  
  // Iniclaiza los semáforos y el retardo
  id_semafor_pantalla = atoi(ll_args[13]);
  id_semafor_mov = atoi(ll_args[14]);

  retard = atoi(ll_args[15]);
  
  // Mapea la memoria compartida para las bústias
  p_bustias = map_mem(atoi(ll_args[16]));

  numPaletas = atoi(ll_args[17]);
  
  // Configura la ventana de juego
  win_set(p_win, n_fil, n_col);
  
  // Arreglo para las colisiones de las paletas
  int cola_paletas[l_pal];

  do{ 		/*** Bucle Principal del juego ***/
  
    win_retard(retard);		// Espera el tiempo de retardo configurado
    receiveM(p_bustias[indice],mis);	// Recibe un mensaje desde la bustia correspondiente
    
    // Interpreta el mensaje recibido para determinar el movimiento
    if(atoi(mis) == 0)
    {
    	des = 0;
    }
    else if(atoi(mis) == 1)
    {
    	des = 1;
    }
    else if(atoi(mis) == 2)
    {
    	des = -1;
    }
    
    // Si hay un desplazamiento, intenta mover la paleta
    if(des != 0)
    {
      waitS(id_semafor_pantalla);		// Espera el semáforo de la pantalla
      mover = true;
      int j = 0;
      // Inicializa la cola de paletas a cero
      for (int i = 0; i < l_pal; i++)
      {
        cola_paletas[i] = 0;
      }
      
      // Verifica si hay colisiones con otras paletas
      for (int i = 1; i <= l_pal; i++)
      {
        pos = win_quincar(paleta.ipo_pf + l_pal - i, paleta.ipo_pc + des);
        if(pos != ' ' && pos != '+')
        {
          if(!hayColaPaletas(cola_paletas, (int) pos - 48, i))
          {
            cola_paletas[j++] = (int) pos - 48;
          }
          
          if(mover)
          {
            mover = false;
          }
        }
      }
      
      // Si no hay colisiones, mueve la paleta
      if(mover)
      {
        // Si la paleta alcanza el borde de la pantalla
        if(paleta.ipo_pc + des == (n_col - 1))
        {
          for(int i = 1; i <= l_pal; i++)
          {
            win_escricar(paleta.ipo_pf + l_pal - i, paleta.ipo_pc, ' ', NO_INV);
          }
          // Marca la bustia como inactiva
          p_bustias[indice] = -1;
        }
        else
        {
          // Mueve la paleta a la nueva posicion
          for(int i = 1; i <= l_pal; i++){
            win_escricar(paleta.ipo_pf + l_pal - i, paleta.ipo_pc, ' ', NO_INV);
            win_escricar(paleta.ipo_pf + l_pal - i,paleta.ipo_pc + des, ll_args[12][0], INVERS);
          }
          paleta.ipo_pc += des;		// Actualiza la posicion de la paleta
        }
      }
      else
      {
        // Si hay colisiones, envia mensajes a las paletas involucradas
        for (int i = 0; i < numPaletas; i++)
        {
          if(p_bustias[i] == -1) 
          {
            continue;
          }
          
          if(hayColaPaletas(cola_paletas, i + 1, j))
          {
            sprintf(mis_aux, "%s", mis);
          }
          else
          {
            sprintf(mis_aux, "%i", 0);
          }
          sendM(p_bustias[i],mis_aux,4);	// Envia el mensaje
        }
      }
      signalS(id_semafor_pantalla);		// Libera el semáforo de la pantalla
    }
    
    // Si la paleta se marca como inactival, finaliza el proceso
    if (p_bustias[indice] == -1)
    {
      exit(0);
    }
    
    f_h = paleta.po_pf + paleta.v_pal;    /* posicio hipotetica de la paleta */
    
    if (f_h != paleta.ipo_pf)
    { /* si pos. hipotetica no coincideix amb pos. actual */
      waitS(id_semafor_pantalla);
      if (paleta.v_pal > 0.0)
      {     /* verificar moviment cap avall */
        
        if (win_quincar(f_h+l_pal-1,paleta.ipo_pc) == ' ')
        {   /* si no hi ha obstacle */
          win_escricar(paleta.ipo_pf, paleta.ipo_pc, ' ', NO_INV);     /*  esborra primer bloc*/
          paleta.po_pf += paleta.v_pal;
          paleta.ipo_pf = paleta.po_pf;   /* actualitza posicio */
          win_escricar(paleta.ipo_pf + l_pal - 1, paleta.ipo_pc, ll_args[12][0], INVERS); /* impr. ultim bloc */
        }
        else
        {    /* si hi ha obstacle, canvia el sentit del moviment */
          paleta.v_pal = -paleta.v_pal;
        }
        signalS(id_semafor_pantalla);
      }
      else
      {      /* verificar moviment cap amunt */
        
        if (win_quincar(f_h,paleta.ipo_pc) == ' ')
        {        /* si no hi ha obstacle */
          win_escricar(paleta.ipo_pf + l_pal - 1, paleta.ipo_pc, ' ', NO_INV); /* esbo. ultim bloc */
          paleta.po_pf += paleta.v_pal;
          paleta.ipo_pf = paleta.po_pf;   /* actualitza posicio */
          win_escricar(paleta.ipo_pf, paleta.ipo_pc, ll_args[12][0], INVERS);  /* impr. primer bloc */
          
        }
        else
        {    /* si hi ha obstacle, canvia el sentit del moviment */
          
          paleta.v_pal = -paleta.v_pal;
        }
        signalS(id_semafor_pantalla);
      }
    }
    else paleta.po_pf += paleta.v_pal;   /* actualitza posicio vertical real de la paleta */
    
    waitS(id_semafor_mov);		// Espera el semaforo de movimiento
    
    // Decreementa el contador de movimientos si es mayor que cero
    if(*p_n_mov > 0)
    {
      *p_n_mov = *p_n_mov - 1;
    }
    signalS(id_semafor_mov);		// Libera el semáforo de movimiento
  } while ((*p_tecla != TEC_RETURN) && (*p_cont == -1) && (*p_n_mov > 0));
     
  return 0;
}
