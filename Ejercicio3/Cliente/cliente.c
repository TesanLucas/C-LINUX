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

void ayuda ();
void mostrarTablero(int *tablero, int tam_filas, int tam_columnas);
void jugar(int *tablero, int tam_filas, int tam_columnas);
void mostrarInterfaz(int *tablero, int tablero_interfaz[][TAM_COLUM], int tam_filas, int tam_columnas, int fila_mostrar1, int colum_mostrar1, int fila_mostrar2, int colum_mostrar2);
int comparar(int *tablero, int primera_fila_elegida, int primera_columna_elegida, int segunda_fila_elegida, int segunda_columna_elegida);
void actualizarInterfaz(int *tablero, int tablero_interfaz[][TAM_COLUM], int primera_fila_elegida, int primera_columna_elegida, int segunda_fila_elegida, int segunda_columna_elegida);


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

  time_t comienzo = time(NULL);
  time_t final;
  sem_t *mutexCliente=sem_open("/mutexCliente",O_CREAT,0666,1);

  int *tablero = NULL;
  int *enJuego = NULL;

  int shmid = shmget(200, sizeof(int[TAM_FILAS][TAM_COLUM]), IPC_CREAT | 0666);
  int shmid2 = shmget(300, sizeof(int), IPC_CREAT | 0666);

  tablero = (int *)shmat(shmid,NULL,0);
  enJuego = (int *)shmat(shmid2,NULL,0);

  signal(SIGINT, SIG_IGN);

  if(*enJuego == 1){
        int decision;
    do {
        printf("Ya hay un cliente jugando, eliga una opcion [1: Esperar a que termine. 2: Finalizar]: ");
        scanf("%d", &decision );
        if(decision == 1){
            printf("Esperando...\n\n");
            sem_wait(mutexCliente);
        }else if(decision == 2){
            shmdt( &tablero );
            shmdt( &enJuego );
            sem_close(mutexCliente);
            printf("Saliendo...\n");
            exit(1);
          }
    } while(decision != 1 && decision != 2);
  }

  *enJuego = 1; //Esta instancia de cliente empieza a jugar.
	jugar(tablero, TAM_FILAS, TAM_COLUM);
  *enJuego = 0; //Esta instancia de cliente termina de jugar.


  final = time(NULL);
  printf("Usted a tardado: %f segundos en terminar el juego.\n", difftime(final, comienzo) );

  shmdt( &enJuego );

  shmdt( &tablero );

  sem_post(mutexCliente);

  sem_close(mutexCliente);
	sem_unlink("/mutexCliente");

  return 0;
}

void jugar(int *tablero, int tam_filas, int tam_columnas){

	int primera_fila_elegida, primera_columna_elegida;
  int segunda_fila_elegida, segunda_columna_elegida;
	int cantidad_de_aciertos = 0;
	int tablero_interfaz [TAM_FILAS][TAM_COLUM] = {
                                                {'-','-','-','-'},
                                                {'-','-','-','-'},
                                                {'-','-','-','-'},
                                                {'-','-','-','-'}
                                              };
	sem_t *semaforo=sem_open("/semaforo",O_CREAT,0666,0);

		mostrarTablero(tablero,tam_filas,tam_columnas);

		system("sleep 4");

		system("clear");
	while (cantidad_de_aciertos != 8) {
		mostrarInterfaz(tablero, tablero_interfaz, tam_filas, tam_columnas, -1, -1, -1, -1);
		printf("\nIngrese Fila-Columna: ");
		scanf("%d-%d", &primera_fila_elegida, &primera_columna_elegida );
		system("clear");
		mostrarInterfaz(tablero, tablero_interfaz, tam_filas, tam_columnas, primera_fila_elegida, primera_columna_elegida, -1, -1);
		printf("\nIngrese Fila-Columna: ");
		scanf("%d-%d", &segunda_fila_elegida, &segunda_columna_elegida );
		system("clear");
		mostrarInterfaz(tablero, tablero_interfaz, tam_filas, tam_columnas, primera_fila_elegida, primera_columna_elegida, segunda_fila_elegida, segunda_columna_elegida);
		if (comparar(tablero, primera_fila_elegida, primera_columna_elegida, segunda_fila_elegida, segunda_columna_elegida) ) {
				cantidad_de_aciertos++;
				actualizarInterfaz(tablero, tablero_interfaz, primera_fila_elegida, primera_columna_elegida, segunda_fila_elegida, segunda_columna_elegida);
		}
		system("sleep 2");
		system("clear");
	}
	sem_post(semaforo);
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

void mostrarInterfaz(int *tablero, int tablero_interfaz[][TAM_COLUM], int tam_filas, int tam_columnas, int fila_mostrar1, int colum_mostrar1, int fila_mostrar2, int colum_mostrar2){

  int i, j;

  printf("  ");
  for ( j = 0; j < tam_columnas; j++) {
      printf("%d ", j);
  }
  printf("\n");

  for ( i = 0; i < tam_filas; i++) {
    printf("%d ", i);
    for ( j = 0; j < tam_columnas; j++) {
      if( i == fila_mostrar1 && j == colum_mostrar1)
        printf("%c ", tablero[j * tam_columnas + i] );
      else if( i == fila_mostrar2 && j == colum_mostrar2)
          printf("%c ", tablero[j * tam_columnas + i] );
      else
        printf("%c ", tablero_interfaz[i][j]);
    }
    printf("\n");
  }

}

int comparar(int *tablero, int primera_fila_elegida, int primera_columna_elegida, int segunda_fila_elegida, int segunda_columna_elegida){

  return (tablero[primera_columna_elegida * TAM_COLUM + primera_fila_elegida] == tablero[segunda_columna_elegida * TAM_COLUM + segunda_fila_elegida]) ? IGUAL : DISTINTO;

}

void actualizarInterfaz(int *tablero, int tablero_interfaz[][TAM_COLUM], int primera_fila_elegida, int primera_columna_elegida, int segunda_fila_elegida, int segunda_columna_elegida){

  tablero_interfaz[primera_fila_elegida][primera_columna_elegida] = tablero[primera_columna_elegida * TAM_COLUM + primera_fila_elegida];
  tablero_interfaz[segunda_fila_elegida][segunda_columna_elegida] = tablero[segunda_columna_elegida * TAM_COLUM + segunda_fila_elegida];

}

void ayuda(){
    printf("\n");
    printf("Este programa sera el encargado de generar la interfaz del usuario e ir actualizandola mientras el usuario ingrese las filas y columnas deseadas\n");
    printf("Tener en cuenta que este proceso no admite parametros, excepto -h y --help para obtener ayuda sobre que hace el proceso)\n");
}
