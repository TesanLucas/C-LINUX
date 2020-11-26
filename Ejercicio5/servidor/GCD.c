//  -----------------------ENCABEZADO------------------------------------------------------------------------

//  Trabajo practico: 3
//  Ejercicio: 5
//  Entrega: 1era
//  Integrantes:
// 	    Tesan, Lucas Brian DNI 41.780.526
// 	    Miño, Maximiliano  DNI 41.783.275

//  ----------------------FIN ENCABEZADO---------------------------------------------------------------------
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CREATE_COLECTION 1
#define DROP_COLECTION 2
#define ADD_IN 3
#define FIND 4
#define REMOVE 5

#define EXISTE 1
#define NO_EXISTE 0

#define IGUALES 1
#define DISTINTOS 0

#define MAX_QUEUE 5

typedef struct{
    int nroOperacion;
    char nombreTabla[30];
    char clave[50];     // para las operaciones DROP_COLLECTION, FIND, REMOVE
    char valor[30];     // para FIND
    char query[100];    // para las operaciones CREATE_COLECTION, ADD_IN
}query_data;

void recibirDatos(struct sockaddr *ca, socklen_t *cl);
void gestorDeConsultas(query_data *datos);
int existeTabla(const char* nombreTabla);
int contadorDeCampos(const char* query);
char* obtenerPrimerCampo(char* tabla, const char* query);
void crearTabla(const char* tabla);
void escribirRegistroEnTabla(const char* tabla, const char* registro);
char* obtenerCampos(char* registro, const char* query);
void reemplazar_saltoDeLinea(char *);
char* obtenerEncabezadoTabla(char* encabezado, const char *tabla);
int comparadorDeCampos(const char* campos, const char* query);
char* recuperarRegistroPorClave(char* registro, const char* tabla, const char* key);
void obtenerValores(char *values, const char* registro);
void eliminarRegistro(const char *registro, const char* tabla );
void signal_handler(int );
void enviarDatos(char *);
void ayuda();

int server_socket;
int client_socket;

int main(int argc, char const *argv[]) {

  if (argc > 1)
  {
      if (strcmp(argv[1],"--help") == 0 || strcmp(argv[1],"-h") == 0) {
          ayuda();
          exit(1);
      }
  }

	if( argc < 2 ){
		printf("Debe ingresar el puerto de escucha !!!\n");
		return 1;
	}

  if(fork() != 0){
     return 0;
  }

  query_data datos;
  char respuesta[1024];
  socklen_t cl=sizeof(struct sockaddr_in);
	struct sockaddr_in sa;
	struct sockaddr_in ca;
	int habilitar = 1;

	if((server_socket=socket(AF_INET,SOCK_STREAM,0)) ==-1){
		printf("No se pudo crear el socket !!!\n");
		return 1;
	}

	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &habilitar, sizeof(query_data)) < 0){
   	printf("No se pudo configurar opciones del socket !!!\n");
		return 1;
	}

	bzero((char *) &sa, sizeof(struct sockaddr_in));
 	sa.sin_family 		= AF_INET;
 	sa.sin_port 		= htons(atoi(argv[1]));
	sa.sin_addr.s_addr= INADDR_ANY;

	bind(server_socket,(struct sockaddr *)&sa,sizeof(struct sockaddr_in));

	listen(server_socket,MAX_QUEUE);

  client_socket=accept(server_socket,(struct sockaddr *) &ca, &cl);

  if(fork() == 0) // realizamos un fork para que el proceso se ejecute en segundo plano
  {
    while (1)
    {
      signal(SIGUSR1, signal_handler);
      recibirDatos((struct sockaddr *) &ca, &cl);
    }
  }

  return 0;
}

void recibirDatos(struct sockaddr *ca, socklen_t *cl){
  query_data datos;
  bzero(&datos,sizeof(query_data));
  recv(client_socket,&datos,sizeof(query_data),0);
  gestorDeConsultas(&datos);
}

void gestorDeConsultas(query_data *datos){

    if(datos->nroOperacion == CREATE_COLECTION){
        //verifico que tengo como minimo dos 3 campos. Obtengo el nombre de la tabla. Verifico que no existe la tabla, y si no existe: obtengo los campos y creo la tabla con sus campos.
        if (contadorDeCampos(datos->query) >= 3) {
          //Obtengo el nombre de la tabla. Verifico que no existe la tabla, y si no existe: obtengo los campos y creo la tabla con sus campos.
          char nTabla[100] = "";
          obtenerPrimerCampo(nTabla, datos->query); //en nTabla tengo el nombre de la tabla
          //Verifico que no existe la tabla, y si no existe: obtengo los campos y creo la tabla con sus campos.
          if(!existeTabla(nTabla)){
            //obtener campos
            char registro[500] = "";
            obtenerCampos(registro,datos->query); //En registro tengo todos los campos.
            //crear tabla con el nombre "nTabla"
            crearTabla(nTabla);
            escribirRegistroEnTabla(nTabla, registro); //escribo en la tabla el registro;
            enviarDatos("Tabla creada correctamente.");
          }else
          {
            enviarDatos("ERROR: Colección existente.");
          }
        }else{
          enviarDatos("Sintaxis incorrecto, verifique que se indique el Nombre de la tabla y al menos 2 campos.");
        }
    }
    else if(datos->nroOperacion == DROP_COLECTION){
        char pathRelativo[100] = "../BD/";
        if(remove(strcat(pathRelativo, datos->nombreTabla))){
           enviarDatos("No existe la tabla indicada.");
        }else{
          enviarDatos("Tabla eliminada con exito...");
        }
    }
    else if(datos->nroOperacion == ADD_IN){

        if(contadorDeCampos(datos->query) >= 3){

            char nTabla[100] = "";
            obtenerPrimerCampo(nTabla, datos->query); //en nTabla tengo el nombre de la tabla

            if(existeTabla(nTabla)){
              //Me quedo con los campos. Verifico que tenga la misma cantidad de parametros la query con la tabla y por ultimo que tengan el mismo nombre
              //(estos datos estan en el primer registro de la tabla)
              //Si se cumple trato de añadirlo, siempre que no exista una clave igual
              char registro[1024] = "";
              char encabezado[1024] = "";
              obtenerCampos(registro,datos->query); //En registro tengo todos los campos.
              obtenerEncabezadoTabla(encabezado,nTabla);

              if(contadorDeCampos(registro) == contadorDeCampos(encabezado)){

                //Me tengo que fijar si los nombre de campos tienen el mismo nombre
                //Si se cumple trato de añadirlo, siempre que no exista una clave igual

                  if(comparadorDeCampos(encabezado, registro)){
                     //Trato de añadirlo, siempre que no exista una clave igual
                      char keyRegistro[100] = "";
                      char values[1024] = "";
                      //Necesito los valores
                      obtenerValores(values, registro);
                      if(!recuperarRegistroPorClave(registro,nTabla, obtenerPrimerCampo(keyRegistro, values))){
                          escribirRegistroEnTabla(nTabla, values);
                          enviarDatos("Registro añadido correctamente.");
                      }else{
                          //Ya existe la key. Va en el fifo.
                          enviarDatos("ERROR: Fila existente.");
                      }
                  }else{
                      // Los campos no son iguales. Va en el fifo de respuesta
                      enviarDatos("ERROR: Campos inválidos.");
                  }

              }else{
                  // No son iguales la cantidades de campos. Va en el fifo de respuesta
                  enviarDatos("ERROR: Campos inválidos.");
              }

            }else{
                //La coleccion no existe.
                enviarDatos("ERROR: Colección inexistente.");
            }

        }else{
            enviarDatos("Sintaxis incorrecto, verifique que se indique el Nombre de la tabla y al menos 2 campos");
        }

    }
    else if(datos->nroOperacion == FIND){

        if(existeTabla(datos->nombreTabla)){
            char registro[1024] = "";
            if (recuperarRegistroPorClave(registro, datos->nombreTabla, datos->valor) != NULL) {
              // Recupero el registro.
              enviarDatos(registro);
            }
            else{
              // No existe la key.
              enviarDatos("ERROR: Fila inexistente.");
            }
        }
        else{
            //La coleccion no existe.
            enviarDatos("ERROR: Colección inexistente.");
        }

    }
    else if(datos->nroOperacion == REMOVE){

      if(existeTabla(datos->nombreTabla)){
          char registro[1024] = "";
          if (recuperarRegistroPorClave(registro, datos->nombreTabla, datos->valor) != NULL) {
              eliminarRegistro(registro, datos->nombreTabla);
              enviarDatos("Registro eliminado correctamente.");
          }
          else{
            //No existe la key. Va en el fifo.
            enviarDatos("ERROR: Fila inexistente.");
          }
      }
      else{
          //La coleccion no existe.
          enviarDatos("ERROR: Colección inexistente.");
      }
    }
}

int contadorDeCampos(const char* query){

  int campos = 0;
  int band = 0;

  while(*query) {
    if(*query ==  ' ')
      campos++;
    query++;
    band = 1;
  }

  return band ? campos + 1 : 0;
}

int comparadorDeCampos(const char* campos, const char* query){

  while(*campos && * query) {

    while (*campos != ' ' && *campos) {
      if(*campos != *query){
        return DISTINTOS;
      }
      campos++;
      query++;
    }
    campos++;
    query = strchr(query, ' ') + 1;
  }
  return IGUALES;
}

char* obtenerPrimerCampo(char* primerCampo, const char* query){

  char *pInicio = primerCampo;

  while (*query != ' ') {
    *primerCampo = *query;
    query++;
    primerCampo++;
  }

  *primerCampo = '\0';

  return pInicio;
}

void obtenerValores(char *values, const char* registro){

  registro = strchr(registro, '=') + 1;
  while (*registro) {
    if(*registro == ' '){
      registro = strchr(registro, '=') + 1;
      *values = 32;
      values++;
    }
    *values = *registro;
    values++;
    registro++;
  }

  *values = '\0';
}

char* obtenerCampos(char *registro, const char* query ){
  char *pQuery = NULL;

  pQuery = strchr(query, ' ') + 1;

  strcpy(registro, pQuery);

  return registro;
}

char* obtenerEncabezadoTabla(char* encabezado, const char *tabla){

  char pathRelativo[100] = "../BD/";
  FILE *pf = fopen(strcat(pathRelativo, tabla), "rt");
  fgets(encabezado, 1024, pf);
  reemplazar_saltoDeLinea(encabezado);
  fclose(pf);
  return encabezado;
}

char* recuperarRegistroPorClave(char* registro, const char* tabla, const char* key){ //devolver la linea

  char pathRelativo[100] = "../BD/";
  char keyRegistro[200] = "";
  FILE *pf = fopen(strcat(pathRelativo, tabla), "rt");
  while (fgets(registro,1024,pf)) {
      reemplazar_saltoDeLinea(registro);
      obtenerPrimerCampo(keyRegistro,registro);
      if(!strcmp(keyRegistro, key)){
          fclose(pf);
          return registro;
      }
  }
  fclose(pf);
  return NULL;
}

void reemplazar_saltoDeLinea(char *cad){
    while(*cad != '\0'){
        if(*cad == '\n'){
            *cad = '\0';
        }
        cad++;
    }
}

int existeTabla(const char *nombreTabla){

  char pathRelativo[100] = "../BD/";
  int salida;
  FILE *pf = fopen(strcat(pathRelativo, nombreTabla), "rt");
  if(pf){
    fclose(pf);
    salida = EXISTE;
  }
  else{
    salida = NO_EXISTE;
  }
  return salida;
}

void crearTabla(const char* tabla){

  char pathRelativo[100] = "../BD/";
  FILE *pf = fopen(strcat(pathRelativo, tabla), "wt");
  fclose(pf);
}

void eliminarRegistro(const char *registroABuscar, const char* nombreTabla) {
    char pathRelativo[100] = "../BD/";
    char registro[100] = "";
    FILE *pfL = fopen(strcat(pathRelativo, nombreTabla), "rt");
    FILE *pfE = fopen("../BD/aux", "wt");

    while (fgets(registro,1024,pfL)) {
        reemplazar_saltoDeLinea(registro);
        if(strcmp(registroABuscar, registro)){
            fprintf(pfE, "%s\n", registro);
        }
    }
    remove(pathRelativo);
    rename("../BD/aux", pathRelativo);
    fclose(pfL);
    fclose(pfE);
}

void escribirRegistroEnTabla(const char *tabla, const char* registro){
  char pathRelativo[100] = "../BD/";
  FILE *pf = fopen(strcat(pathRelativo, tabla), "a+t");
  fprintf(pf, "%s\n", registro);
  fclose(pf);
}

/*Manejador de la senal*/
void signal_handler(int num_sig)
{
    //printf("\nProceso GCD terminado\n");
    close(client_socket);
    close(server_socket);
    exit(0);
}

void enviarDatos(char *respuesta){
    send(client_socket,respuesta,100,0);
}

void ayuda(){
    printf("\n");
    printf("Este programa ejecuta en segundo plano, encargandose del procesamiento de las consultas\n");
    printf("\tenviadas a travez de la interfaz correspondiente.\n");
    printf("Tener en cuenta que se necesita expecificar 1 parametro que sera el puerto de escucha. Por ejemplo 2222\n");
    printf("Para finalizar la ejecucion, se debe enviar la señal SIGUSR1(10).\n\n");
}
