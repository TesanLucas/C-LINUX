//  -----------------------ENCABEZADO------------------------------------------------------------------------

//  Trabajo practico: 3
//  Ejercicio: 3
//  Entrega: 1era
//  Integrantes:
// 	    Tesan, Lucas Brian DNI 41.780.526
// 	    Miño, Maximiliano  DNI 41.783.275

//  ----------------------FIN ENCABEZADO---------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#define TAM_FILAS 4
#define TAM_COLUM 4
#define EXISTE 1
#define NO_EXISTE 0
#define CANT_LETRAS 8
#define IGUAL 1
#define DISTINTO 0

int SALIR = 0;

void ayuda();
void inicializarTablero(int *tablero , int tam_filas, int tam_columnas);
void rellenarTablero(int *tablero, int tam_filas, int tam_columnas);
int existeLetra(int letra, int *vec, int tam);
int hayLetraEnTablero(int fila_pos, int colum_pos, int *tablero);
void iniciarServer(int *tablero, int tam_filas, int tam_columnas, int *enJuego);
void manejadorSignal(int signal);

int main(int argc, char const *argv[]) {

  if (argc == 2) {
        if (strcmp(argv[1],"--help") == 0 || strcmp(argv[1],"-h") == 0){
          ayuda();
          exit(0);
        }
        else{
          printf("Si quiere ayuda con el programa, ejecutar con %s -help o %s -?\n", argv[0], argv[0] );
          exit(0);
        }
  }else if(argc > 2){
    printf("Este programa no recibe parametros. Para pedir ayuda ejecutar: %s -help o %s -?\n",  argv[0], argv[0]);
    exit(1);
  }

  int *tablero = NULL;
  int *enJuego = NULL;
  int *serverCorriendo = NULL;

  int shmid = shmget(200, sizeof(int[TAM_FILAS][TAM_COLUM]), IPC_CREAT | 0666);
  int shmid2 = shmget(300, sizeof(int), IPC_CREAT | 0666);
  int shmid3 = shmget(500, sizeof(int), IPC_CREAT | 0666);

  sem_t *semaforo=sem_open("/semaforo",O_CREAT,0666,0);

  tablero = (int *)shmat(shmid,NULL,0);
  enJuego = (int *)shmat(shmid2,NULL,0);
  serverCorriendo = (int *)shmat(shmid3,NULL,0);

  *enJuego = 0; //Todavia no hay nadie jugando cuando arranca el server

  if(*serverCorriendo == 1){
      shmdt( &tablero );
      shmdt( &enJuego );
      shmdt( &serverCorriendo );
      sem_close(semaforo);
      printf("No se pudo inicializar el servidor, ya esta corriendo un servidor en esta misma computadora.\n");
      exit(1);
  }

  *serverCorriendo = 1; //Esta instancia de server se esta ejecutando

  signal(SIGUSR1, &manejadorSignal);
  signal(SIGINT, SIG_IGN);

  while (!SALIR) {
    iniciarServer(tablero, TAM_FILAS, TAM_COLUM, enJuego);
    sem_wait(semaforo);
  }

  *serverCorriendo = 0; //Esta instancia de server dejo de ejecutarse

  sem_close(semaforo);
	sem_unlink("/semaforo");

  shmdt( &tablero );
  shmdt( &enJuego );
  shmdt( &serverCorriendo );

  shmctl(shmid,IPC_RMID,NULL);
  shmctl(shmid2,IPC_RMID,NULL);
  shmctl(shmid3,IPC_RMID,NULL);

  return 0;
}

void manejadorSignal(int signal){
  int *enJuego = NULL;
  int shmid = shmget(300, sizeof(int), IPC_CREAT | 0666);
  sem_t *semaforo=sem_open("/semaforo",O_CREAT,0666,0);
  enJuego = (int *)shmat(shmid,NULL,0);
  if(signal == SIGUSR1){
    if(*enJuego == 0){
      SALIR = 1;
      printf("Saliendo del servidor ... \n");
      sem_post(semaforo);
    }
    else
      printf("No se puede cerrar el servidor, hay una partida actualmente!\n");
  }
  shmdt( &enJuego );
}

void iniciarServer(int *tablero, int tam_filas, int tam_columnas, int *enJuego){

    inicializarTablero(tablero, tam_filas, tam_columnas);

}

void inicializarTablero(int *tablero, int tam_filas, int tam_columnas){
  int i,j;

  for ( i = 0; i < tam_filas; i++) {
    for ( j = 0; j < tam_columnas; j++) {
      tablero[j*tam_columnas + i] = 0;
    }
  }

  rellenarTablero(tablero, tam_filas, tam_columnas);

}

void rellenarTablero(int *tablero, int tam_filas, int tam_columnas){

  int i;
  int fila_pos;
  int colum_pos;
  int letra;

  srand(time(NULL));

  int letras [CANT_LETRAS] = {0,0,0,0,0,0,0,0};

  for (i = 0; i < CANT_LETRAS; i++) {
      while ( existeLetra(letra = (65 + rand() % 26), letras, CANT_LETRAS ) ){
        //nada
      }
      letras[i] = letra;
  }

  for (i = 0; i < CANT_LETRAS; i++) {
    while( ( hayLetraEnTablero(fila_pos = rand() % TAM_FILAS, colum_pos = rand() % TAM_COLUM, tablero ) ) ){
      //nada
    }
        tablero[colum_pos * tam_columnas + fila_pos] = letras[i];
    while( ( hayLetraEnTablero(fila_pos = rand() % TAM_FILAS, colum_pos = rand() % TAM_COLUM, tablero ) ) ){
      //nada
    }
        tablero[colum_pos * tam_columnas + fila_pos] = letras[i];

  }
}

int existeLetra(int letra, int *vec, int tam){
  int i;
  for ( i = 0; i < tam; i++) {
    if( *(vec + i) == letra)
      return EXISTE;
  }

  return NO_EXISTE;

}

int hayLetraEnTablero(int fila_pos, int colum_pos, int *tablero){
  return tablero[colum_pos * TAM_COLUM + fila_pos] ? EXISTE : NO_EXISTE;
}

void mostrarTablero(int *tablero, int tam_filas, int tam_columnas){

	int i, j;

	for ( i = 0; i < tam_filas; i++) {
		for ( j = 0; j < tam_columnas; j++) {
			printf("%c ", tablero[j * tam_columnas + i]);
		}
		printf("\n");
	}

}

void ayuda(){
    printf("\n");
    printf("Este programa sera el encargado de generar el tablero con letras aleatorias y gestionar el juego\n");
    printf("Para finalizar la ejecucion, se debe enviar la señal SIGUSR1(10).\n\n");
    printf("Tener en cuenta que este proceso no admite parametros, excepto -h y --help para obtener ayuda sobre que hace el proceso)\n");
}
