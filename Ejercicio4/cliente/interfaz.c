//  -----------------------ENCABEZADO------------------------------------------------------------------------

//  Trabajo practico: 3
//  Ejercicio: 4
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CREATE_COLECTION 1
#define DROP_COLECTION 2
#define ADD_IN 3
#define FIND 4
#define REMOVE 5

typedef struct{
    int nroOperacion;
    char nombreTabla[30];
    char clave[50];     // para las operaciones DROP_COLLECTION, FIND, REMOVE
    char valor[30];     // para FIND
    char query[100];    // para las operaciones CREATE_COLECTION, ADD_IN
}query_data;

void ayuda();
int mi_strncmp(char *, char *);
int mi_strcpy(char *, char *);
void reemplazar_saltoDeLinea(char *);
void enviarDatosFifo(query_data *datos);
void recibirDatosFifo();

int main(int argc, char *argv[]) {

    if (argc > 1)
    {
        if (strcmp(argv[1],"--help") == 0 || strcmp(argv[1],"-h") == 0) {
            ayuda();
            exit(1);
        }
    }
    char asd[20];
    char consulta[100];
    char comando[200] = "echo '";
    char *puntero;

    query_data datos;
    mkfifo("../fifoConsulta",0666);

while(1)
{
    signal(SIGINT, SIG_IGN);
    printf("Ingrese su consulta: ");
    fgets(consulta, 100, stdin);

    if(mi_strncmp(consulta, "CREATE COLLECTION") == 0)
    {
        reemplazar_saltoDeLinea(consulta);
        datos.nroOperacion = CREATE_COLECTION;
        strcpy(datos.query, strchr(consulta, 'N') + 2); // le paso lo que le sigue a la consulta despues del espacio
        enviarDatosFifo(&datos);
        recibirDatosFifo();
    }
    else if(mi_strncmp(consulta, "DROP COLLECTION") == 0)
    {
        datos.nroOperacion = DROP_COLECTION;
        //verificar sintaxis
        puntero = strchr(comando, ' ') + 2;
        strcpy(puntero, consulta);
        puntero = strchr(comando, '\n');
        strcpy(puntero, "' | awk ' NF==3 {print $0} ' >> ./arch.out");

        system(comando);
        if(system("test -s arch.out")){
            printf("Sintaxis incorrecta\n");
            system("rm arch.out");
        }
        else{
            //printf("sintaxis valida\n");
            sscanf(consulta, "%s %s %s",asd, asd, datos.nombreTabla);
            system("rm arch.out");
            enviarDatosFifo(&datos);
            recibirDatosFifo();
        }
    }

    else if(mi_strncmp(consulta, "ADD IN") == 0)
    {
        reemplazar_saltoDeLinea(consulta);
        datos.nroOperacion = ADD_IN;
        strcpy(datos.query, strchr(consulta, 'N') + 2); // le paso lo que le sigue a la consulta despues del espacio
        enviarDatosFifo(&datos);
        recibirDatosFifo();
    }
    else if(mi_strncmp(consulta, "FIND") == 0)
    {
        datos.nroOperacion = FIND;
        //verificar sintaxis
        puntero = strchr(comando, ' ') + 2;
        strcpy(puntero, consulta);
        puntero = strchr(comando, '\n');
        strcpy(puntero, "' | awk ' NF==3 {print $0} ' >> ./arch.out");

        system(comando);
        if(system("test -s arch.out")){
            printf("Sintaxis incorrecta\n");
            system("rm arch.out");
        }
        else{
            sscanf(consulta, "%s %s %s",asd, datos.nombreTabla, datos.valor);
            system("rm arch.out");
            enviarDatosFifo(&datos);
            recibirDatosFifo();
        }

    }
    else if(mi_strncmp(consulta, "REMOVE") == 0)
    {
        datos.nroOperacion = REMOVE;
        //verificar sintaxis
        puntero = strchr(comando, ' ') + 2;
        strcpy(puntero, consulta);
        puntero = strchr(comando, '\n');
        strcpy(puntero, "' | awk ' NF==3 {print $0} ' >> ./arch.out");

        system(comando);
        if(system("test -s arch.out")){
            printf("Sintaxis incorrecta\n");
            system("rm arch.out");
        }
        else{
            sscanf(consulta, "%s %s %s",asd, datos.nombreTabla, datos.valor);
            system("rm arch.out");
            enviarDatosFifo(&datos);
            recibirDatosFifo();
        }
    }
    else if(mi_strncmp(consulta, "QUIT") == 0)
    {
        reemplazar_saltoDeLinea(consulta);
        printf("Finalizando ejecucion.\n");
        unlink("../fifoConsulta");
        return 0;
    }
    else
    {
        printf("ERROR: Sintaxis incorrecta\n");
    }
}

}

int mi_strncmp(char *cad1, char *cad2){
    while(*cad1 != '\0' && *cad2 != '\0'){
        if(*cad1 != *cad2){
            return 1;
        }
        cad1++;
        cad2++;
    }
    return 0;
}

int mi_strcpy(char *cadDestino, char *cad2){
    while(*cad2 != '\0'){
        *cadDestino = *cad2;
        cadDestino++;
        cad2++;
    }
    *cadDestino = '\0';
    return 0;
}

void reemplazar_saltoDeLinea(char *cad){
    while(*cad != '\0'){
        if(*cad == '\n'){
            *cad = '\0';
        }
        cad++;
    }
}

void enviarDatosFifo(query_data *datos){
  int w = open("../fifoConsulta", O_WRONLY);
  write(w, datos, sizeof(query_data));
  close(w);
}

void recibirDatosFifo(){
char respuesta[1024];
int r = open("../fifoRespuesta", O_RDONLY);
read(r, respuesta, 1024);
printf("GCD: %s\n", respuesta);
close(r);
}

void ayuda() {
    printf("\n");
    printf("Este programa se encarga de funcionar como una interfaz para las consultas sobre una base de datos\n");
    printf("quedando a la espera de las siguientes acciones:\n\n");
    printf("   1: CREATE COLLECTION - Crea una nueva colección\n");
    printf("Sintaxis: CREATE COLLECTION nombre campo1 campo2 [ .. campoN ]\n");
    printf("Ejemplo: CREATE COLLECTION Producto código descripción\n\n");

    printf("   2: DROP COLLECTION - Elimina una colección existente\n");
    printf("sintaxis: DROP COLLECTION nombre");
    printf("Ejemplo: DROP COLLECTION Producto\n\n");

    printf("   3: ADD IN - Agrega una fila en una colección existente\n");
    printf("sintaxis: ADD IN nombre campo1=valor1 campo2=valor2 [ .. campoN=valorN ]\n");
    printf("Ejemplo: ADD IN Producto código=1 descripción=Harina\n\n");

    printf("   4: FIND - Consulta una fila en una colección existente\n");
    printf("Sintáxis: FIND nombre valor\n");
    printf("Ejemplo: FIND Producto 1\n\n");

    printf("   5: REMOVE - Elimina una fila en una colección existente\n");
    printf("Sintaxis: REMOVE nombre valor\n");
    printf("Ejemplo: REMOVE Producto 1\n\n");

    printf("   6: QUIT - Finaliza la ejecucion del proceso.\n");
}
