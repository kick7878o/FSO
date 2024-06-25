/***********************************************************/
/*                                                         */
/*                      winsuport.h                        */
/*                                                         */
/* Definicio de constants i prototipus de funcions per     */
/* utilitzar la interficie d'acces a "curses" implementada */
/* al modul WinSuport.c                                    */
/*							   */
/***********************************************************/


/* Definicions generals per utilitzar les funcions de "winsuport.c" */

#define  INVERS   1
#define  NO_INV   0

#define TEC_AMUNT   'w'
#define TEC_AVALL   's'
#define TEC_DRETA   'd'
#define TEC_ESQUER  'a'
#define TEC_RETURN  13  	/* tecla de Carry Return */
#define TEC_ESPAI   ' '  	/* tecla de barra espaiadora */



/* Definicions de rutines publiques del fitxer "winsuport.c" */


int win_ini(int *fil, int *col, char creq, unsigned int inv);
/* Rutina per crear i inicialitzar una finestra de sortida de text, */
/* utilitzant l'entorn de les curses. S'ha de passar per referencia */
/* el numero de files i de columnes que ha de tenir la finestra     */
/* (*fil, *col). Si s'especifica un 0 al valor dels dos parametres, */
/* la finestra ocupara tota la pantalla del terminal. L'ultima fila */
/* de la finestra es reserva per mostrar missatges de text. La      */
/* resta de files es reserva per a tauler de joc. Es dibuixa un     */
/* requadre a tot el tauler de joc, format per caracters creq, que  */
/* es mostren invertits si el parametre inv es diferent de zero.    */
/* Els errors que retorna aquesta rutina son els seguents:          */
/*      =0 -> cap problema, tot ha anat be			    */
/*      -1  -> la finestra ja havia estat creada                    */
/*      -2  -> no es pot inicialitzar l'entorn de curses            */
/*      -3  -> les mides demanades de camp de joc son massa grans   */
/*      -4  -> la finestra finalment no s'ha pogut crear            */
/* Si el valor dels parametres *fil, *col era zero al cridar la ru- */
/* tina, en sortir de la rutina contindran el numero assignat de    */
/* files i de columnes (les dimensions de la finestra de terminal)  */
/* Per restaurar el terminal cal cridar a la rutina win_fi abans    */
/* de sortir del programa.                                          */ 


void win_fi();
/* Rutina per eliminar la finestra i restaurar el terminal.	    */ 


void win_escricar(int f, int c, char car, unsigned int invers);
/* Rutina per escriure un caracter en una posicio de la finestra,   */
/* on la primera posicio es la (0,0); si invers es diferent de zero,*/
/* el caracter surt invertit.                                       */


char win_quincar(int f, int c);
/* Rutina per llegir quin caracter hi ha en una posicio de finestra */
/* on la primera posicio es la (0,0).                               */


int win_quinatri(int f, int c);
/* Rutina per llegir quin atribut hi ha en una posicio de finestra. */
/* De fet, nomes determina si l'atribut es INVERS (retorna != 0), o */
/* si l'atribut no es INVERS (retorna = 0).			    */


void win_escristr(char *str);
/* Rutina per escriure un string en l'ultima fila de la finestra.   */


int win_gettec(void);
/* Rutina per llegir una tecla; retorna zero si no hi ha tecla per llegir, */
/* o el codi ASCII d'una tecla normal.					   */


int win_retard(int ms);
/* Rutina per retardar ms mil.lisegons.				   */
