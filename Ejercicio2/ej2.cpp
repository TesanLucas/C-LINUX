//  -----------------------ENCABEZADO------------------------------------------------------------------------

//  Trabajo practico: 3
//  Ejercicio: 2
//  Entrega: 1era
//  Integrantes:
// 	    Tesan, Lucas Brian DNI 41.780.526
// 	    Miño, Maximiliano  DNI 41.783.275

//  ----------------------FIN ENCABEZADO---------------------------------------------------------------------

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <iostream>
#include <thread>
#include <sys/time.h>
#include <pthread.h>
#include <stdio.h>
#include <dirent.h>
#include <time.h>
using namespace std;

typedef struct
{
    const char *pathDirectorio;
    int nroThread;
    int cantArchivosALeer;
    const char *directorioDestino;
} thread_info;  // info para los threads

typedef struct
{
char nombreArchivo[40];
int caracteresTotales;
char horaFinalizacion[40];
} datosProcesamiento;   // datos que devuelven los threads

void ayuda();
void crearHilos(int, char *, char *);
int contarCantArchivos(char *);
int contarLetras(void *);
void guardarResultados(void *, int, int , int , char *, char *, char *);
void mostrarEstadisticas(datosProcesamiento [], int);
sem_t semEscritura;
sem_t semLectura;

int main(int argc, char *argv[]) {
    if (argc > 1) {
        // Chequeamos si se recibio un parametro de ayuda
        if (strcmp(argv[1],"--help") == 0 || strcmp(argv[1],"-h") == 0) {
            ayuda();
        } // Chequeamos si se recibio menos que la cantidad necesaria de parametros
        else if (argc < 4) {
            cout << "Argumentos faltantes. Este script necesita 3 argumentos, revise la ayuda" << endl;
            ayuda();
        } else {
            // Convertimos el numero pasado por parametro a int
            int numThreads = atoi(argv[1]);
            if (numThreads < 0)
            {
                cout << "El número de threads tiene que ser mayor a 0. Vuelva a ejecutar este script." << endl;
                exit(1);
            }
            crearHilos(numThreads, argv[2], argv[3]);
          }
    } else
    {
        ayuda();
    }
}

void crearHilos(int numThreads, char *pathDirectorio, char *pathSalida) {
  int i;
  int cant_archivos = contarCantArchivos(pathDirectorio);
  int archivosPorThread = cant_archivos/numThreads;
  sem_init(&semLectura, 0, 0);
  sem_init(&semEscritura, 0, 1);

  if (cant_archivos % numThreads != 0){
      cout << "no se puede dividir el trabajo equitativamente, pruebe cambiando la cantidad de threads!" << endl;
      exit (1);
  }
  if (archivosPorThread == 0) {
      cout << "La cantidad de threads es muy alta. No se puede dividir el trabajo equitativamente." << endl;
      exit(1);
  }
  cout << "Cada thread operara con: " << archivosPorThread << " archivo/os" << endl;
  cout << endl;

  thread_info info[numThreads];
  thread procesarArchivos[numThreads];

  mkfifo("./fifo",0666);
    for (i = 1; i <= numThreads; i++){
      info[i].cantArchivosALeer = archivosPorThread;
      info[i].nroThread = i;
      info[i].pathDirectorio = pathDirectorio;
      info[i].directorioDestino = pathSalida;
      procesarArchivos[i - 1] = thread(contarLetras, &info[i]);
    }

  datosProcesamiento datos[cant_archivos];
  int r = open("./fifo", O_RDONLY);
  for (i = 0; i < cant_archivos; i++){
    sem_wait(&semLectura);  // pido semaforo de lectura
    read(r,&datos[i],sizeof(datosProcesamiento));
    sem_post(&semEscritura);  // libero semaforo de escritura para los threads
  }
  close(r);

  sem_destroy(&semLectura);
  sem_destroy(&semEscritura);

  for (i = 1; i <= numThreads; i++){   // Joineamos los threads.
    procesarArchivos[i - 1].join();
  }
  unlink("./fifo");
  mostrarEstadisticas(datos, cant_archivos);
}


int contarCantArchivos(char *pathDirectorio){
int cantArchivos = 0;
  /* Con un puntero a DIR abriremos el directorio */
  DIR *dir;
  /* en *ent habrá información sobre el archivo que se está "sacando" a cada momento */
  struct dirent *ent;

  /* Empezaremos a leer el directorio */
  dir = opendir (pathDirectorio);

  if (dir == NULL){
    cout << "no se pudo abrir el directorio" << endl;
    exit (1);
  }

  /* Leyendo uno a uno todos los archivos que hay */
  while ((ent = readdir (dir)) != NULL)
    {
      /* Nos devolverá el directorio actual (.) y el anterior (..), como hace ls */
      if ( (strcmp(ent->d_name, ".")!=0) && (strcmp(ent->d_name, "..")!=0) )
        {
         cantArchivos += 1;
        }
    }

  closedir (dir);
  return cantArchivos;
}

int contarLetras(void * info){
    char NombreFichero[60];
    FILE *Fichero;
    char Linea[256];
    int contador = 0;
    int cuantos;
    int i, vocal, consonante, otroCaracter;
    char pathArchivo[60];

    // variables para la fecha y milisegundos
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char horaInicial[20], milisegundosInicial[20];
    sprintf(horaInicial, "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    //cout << "hora transformada: " << horaInicial << endl;
    struct timeval milisegundos;
    gettimeofday(&milisegundos, NULL);
    sprintf(milisegundosInicial, "%ld", milisegundos.tv_usec);

    // Convertimos el parametro recibido a puntero de nuestra struct
    thread_info * t_info = (thread_info *) info;
    int nroThread = t_info->nroThread;
    int cantArchivosALeer = t_info->cantArchivosALeer;
    char archivos [cantArchivosALeer][20];
    DIR *dir;
    struct dirent *ent;

    dir = opendir (t_info->pathDirectorio);   // puede tirar error de sintaxis, nose

  /* Miramos que no haya error */
    if (dir == NULL)
      cout << "No puedo abrir el directorio " << t_info->pathDirectorio << endl;

    int ultimoArchivoAProcesar = nroThread * cantArchivosALeer;  // cual es el ultimo archivo a procesar
    int primerArchivoAProcesar = ultimoArchivoAProcesar - cantArchivosALeer + 1;    // para saber cual es el primer archivo a procesar
    int nroArchivo = 1;
    int cantArchivosLeidos = 0;
    while ((ent = readdir (dir)) != NULL)
    {
      /* Nos devolverá el directorio actual (.) y el anterior (..), como hace ls */
        if ( (strcmp(ent->d_name, ".")==0) || (strcmp(ent->d_name, "..")==0)){
            // nada
        }
        else if((nroArchivo >= primerArchivoAProcesar) && (nroArchivo <= ultimoArchivoAProcesar))
        {
            strcpy(pathArchivo, t_info->pathDirectorio);
            strcat(pathArchivo, "/");
            strcat(pathArchivo, ent->d_name);
            Fichero = fopen(pathArchivo,"rb");
            // Si no existe el fichero se visualiza un error
            if(!Fichero) {
                cout << "\nEl archivo no existe o no se puede abrir..." << endl;
            }
            else {
                vocal = 0;
                consonante = 0;
                otroCaracter = 0;
                contador = 0;
                fgets(Linea, 255, Fichero);
                while(!feof(Fichero)) {
                    for(i = 0; i < strlen(Linea) - 1; i++){
                      if((Linea[i] >= 65 && Linea[i] <= 90) || (Linea[i] >= 97 && Linea[i] <= 122)){  // no permite ñ Ñ
                        if(Linea[i] == 65 || Linea[i] == 69 || Linea[i] == 73  || Linea[i] == 69 || Linea[i] == 85 || Linea[i] == 97 || Linea[i] == 101 || Linea[i] == 105 || Linea[i] == 111 || Linea[i] == 117)
                        {
                          vocal++;
                        }
                        else
                        {
                          consonante++;
                        }
                      }
                      else
                      {
                        otroCaracter++;
                      }
                    }
                    fgets(Linea, 255, Fichero);
                }
            contador = vocal + consonante + otroCaracter;
            guardarResultados(info, vocal, consonante, otroCaracter, ent->d_name, horaInicial, milisegundosInicial);
            strcpy(archivos[cantArchivosLeidos], ent->d_name);
            cantArchivosLeidos++;
            nroArchivo++;
            }
        }
        else if(nroArchivo > ultimoArchivoAProcesar){
            break;
        }
        else{
          nroArchivo++;
        }
    }

cout << "archivos leidos por el thread nro " << nroThread << ":" << endl;
for(i = 0; i < cantArchivosLeidos; i++){
  cout << archivos[i] << " - ";
}
cout << endl << endl;
closedir(dir);
return 0;
}


void guardarResultados(void *info, int cantVocales, int cantConsonantes, int cantOtrosCaracteres, char *nombreArchivo, char * horaInicial, char *milisegundosInicial){

  time_t t = time(NULL);  // date
  struct tm tm = *localtime(&t);
  struct timeval milisegundos;  // milisegundos

  // agarro los datos de la estructura
  thread_info * t_info = (thread_info *) info;
  int nroThread = t_info->nroThread;
  char pathDestino[40];
  // escribo el nombre del path, ya que sino solo funcionaria con el path ./<archivo>
  strcpy(pathDestino, t_info->directorioDestino);
  strcat(pathDestino, "/");
  strcat(pathDestino, nombreArchivo);
  strcat(pathDestino, ".txt");
  FILE * Fichero = fopen(pathDestino,"w");

  if(!Fichero) {
    cout << "no se pudo crear el archivo " << pathDestino << endl;
  }
  fprintf(Fichero, "hora inicio: %s_%s millisegundos\n", horaInicial, milisegundosInicial);
  fprintf(Fichero, "numero de Thread: %i\n", nroThread);
  fprintf(Fichero, "numero de vocales: %i\n", cantVocales);
  fprintf(Fichero, "numero de consonantes: %i\n", cantConsonantes);
  fprintf(Fichero, "numero de otros caracteres: %i\n", cantOtrosCaracteres);

  tm = *localtime(&t);
  char horaFinal[20], milisegundosFinal[20];
  sprintf(horaFinal, "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  gettimeofday(&milisegundos, NULL);
  sprintf(milisegundosFinal, "%ld", milisegundos.tv_usec);
  fprintf(Fichero, "hora final: %s_%s milisegundos", horaFinal, milisegundosFinal);

  // pasar info al proceso inicial
  char horaFinalizacion[40];
  sprintf(horaFinalizacion, "%s_%s", horaFinal, milisegundosFinal);
  datosProcesamiento datos;
  datos.caracteresTotales = cantConsonantes + cantOtrosCaracteres + cantVocales;
  strcpy(datos.horaFinalizacion, horaFinalizacion);
  strcpy(datos.nombreArchivo, nombreArchivo);

  sem_wait(&semEscritura);  // pido turno para escribir
  int w = open("./fifo", O_WRONLY);
  write(w,&datos,sizeof(datosProcesamiento));
  close(w);
  sem_post(&semLectura);  // aviso al proceso principal que puede leer
}


void mostrarEstadisticas(datosProcesamiento datos[], int tam){
  int mayorCantCaracteres, posMayorCantCaracteres;
  char *terminoUltimo, *terminoPrimero;
  int posTerminoPrimero, posTerminoUltimo;
  int i;

  // determino mayorCantCaracteres

  for(i = 0; i < tam; i++)
  {
    if(i == 0){
      mayorCantCaracteres = datos[0].caracteresTotales;
      posMayorCantCaracteres = 0;
    }
    else if(mayorCantCaracteres < datos[i].caracteresTotales)
    {
      mayorCantCaracteres = datos[i].caracteresTotales;
      posMayorCantCaracteres = i;
    }
  }

  // determino TerminoUltimo
  for(i = 0; i < tam; i++)
  {
    if(i == 0){
      terminoUltimo = datos[0].horaFinalizacion;
      posTerminoUltimo = 0;
    }
    else if((strcmp(datos[i].horaFinalizacion , terminoUltimo)) > 0)
    {
      terminoUltimo = datos[i].horaFinalizacion;
      posTerminoUltimo = i;
    }
  }

  // determino TerminoPrimero
  for(i = 0; i < tam; i++)
  {
    if(i == 0){
      terminoPrimero = datos[0].horaFinalizacion;
      posTerminoPrimero = 0;
    }
    else if((strcmp(datos[i].horaFinalizacion , terminoPrimero)) < 0)
    {
      terminoPrimero = datos[i].horaFinalizacion;
      posTerminoPrimero = i;
    }
  }

  cout << "nombre del archivo con menor cantidad de caracteres totales: \"" << datos[posMayorCantCaracteres].nombreArchivo << "\", con " << mayorCantCaracteres << " caracteres" << endl;
  cout << "nombre del primer archivo finalizado: \"" << datos[posTerminoPrimero].nombreArchivo << "\", a las" << strchr(terminoPrimero, ' ') << endl;
  cout << "nombre del ultimo archivo finalizado: \"" << datos[posTerminoUltimo].nombreArchivo << "\", a las" << strchr(terminoUltimo, ' ') << endl;
}

void ayuda() {
    printf("\n");
    printf("-Este script recibe 3 valores: un path a un directorio de “entrada”, un\n");
    printf("  path a un directorio de “salida” y el nivel de paralelismo (cantidad de threads).\n");
    printf("-el programa reparte todos los archivos del directorio de entrada en forma equitativa\n");
    printf("  entre los N threads que se han generado.\n");
    printf("-Cada thread cuenta la cantidad de letras totales en el contenido de cada archivo \n");
    printf("  generando un nuevo archivo en el directorio de salida con el mismo nombre original\n");
    printf("-indicando: nombre archivo, nro thread, cant vocales, consonantes, otros caracteres, fecha inicio/final de thread\n");
    printf("-Adicionalmente se muestra por pantalla: \n");
    printf("  *Cada thread, su número y qué archivos le tocó analizar. \n");
    printf("  *El nombre del archivo con menor cantidad de caracteres totales. \n");
    printf("  *El nombre del primer archivo finalizado. \n");
    printf("  *El nombre del último archivo finalizado \n");
    printf("\n");
    printf("ejemplo de ayuda: \n");
    printf("\t ./ejecutable 1 ./lote/archivos ./lote/resultados");
    printf("\n\n");
}
