//  -----------------------ENCABEZADO------------------------------------------------------------------------

//  Trabajo practico: 3
//  Ejercicio: 1
//  Entrega: 1era
//  Integrantes:
// 	    Tesan, Lucas Brian DNI 41.780.526
// 	    Miño, Maximiliano  DNI 41.783.275

//  ----------------------FIN ENCABEZADO---------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/prctl.h>
#include <limits.h>

#define TODO_BIEN 1
#define SIN_MEMORIA 2
#define HAY_DUPLICADO 3
#define TAM_INFO 500

typedef struct sNodo
{
    char info[TAM_INFO];
    struct sNodo *sig;
} tNodo;

typedef tNodo *tLista;

int paramValido(const char *);
void crearLista(tLista *);
int listaLlena(const tLista *);
int listaVacia(const tLista *);
int insertarEnLista(tLista *, const char *);
void vaciarLista(tLista *);
void mostrarListaString(const tLista *, int, int);

int main(int argc, char *argv[]){
    // ayuda y validaciones

    if (argc <= 2)
    {
        if(argc == 2){
            if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0){
                printf("Este programa muestra un arbol de procesos, opcionalmente se puede pasarle un parametro N (entre 0 y 5 inclusive) para especificar la descendencia maxima a mostrar\n");
                printf("\nEjemplo de ejecución: \"./ejercicio1 2\"\n");
                exit(1);
            }
            else if (!paramValido(argv[1])){
                printf("El parametro ingresado no es válido.\n");
                printf("Consulte la ayuda con `-h` o `--help`\n");
                exit(2);
            }
        }
    }
    else
    {
        printf("Error, la cantidad de parametros ingresados no es correcta.\n");
        printf("Consulte la ayuda con `-h` o `--help`\n");
        exit(3);
    }
    char cad[TAM_INFO];
    int pid_act;
    int descendencia_maxima = 5;
    int num_proceso = 1;
    tLista lista;

    if(argc == 2 && atoi(argv[1]) >= 0 && atoi(argv[1]) <= 5){
        descendencia_maxima = atoi(argv[1]);
    }else{
        printf("El parametro ingresado no es válido. Debe ingresar un numero natural comprendido entre 0 y 5 inclusive\n");
        exit(3);
    }
    crearLista(&lista);

    pid_act = getpid();

    snprintf(cad, TAM_INFO, "%d (%d)", num_proceso, pid_act);
    insertarEnLista(&lista, cad);
    mostrarListaString(&lista, 1, descendencia_maxima);

    // proceso 1
    for (int i = 0; i < 3; i++)
    {
        num_proceso++;
        // procesos 2, 3 y 4
        if (fork() == 0)
        {
            char cad[TAM_INFO];
            pid_act = getpid();
            snprintf(cad, TAM_INFO, "%d (%d)", num_proceso, pid_act);
            insertarEnLista(&lista, cad);
            mostrarListaString(&lista, 2, descendencia_maxima);

            if (i == 0)
            {
                num_proceso+= 2;
                // proceso 5 y 6
                for (int l = 0; l < 2; l++)
                {
                    num_proceso++;
                    if (fork() == 0)
                    {
                        char cad[TAM_INFO];
                        pid_act = getpid();
                        snprintf(cad, TAM_INFO, "%d (%d)", num_proceso, pid_act);
                        insertarEnLista(&lista, cad);
                        mostrarListaString(&lista, 3, descendencia_maxima);

                        // proceso 8
                        if (l == 1)
                        {
                            if (fork() == 0)
                            {
                                char cad[TAM_INFO];
                                num_proceso += 2;
                                pid_act = getpid();
                                snprintf(cad, TAM_INFO, "%d (%d)", num_proceso, pid_act);
                                insertarEnLista(&lista, cad);
                                mostrarListaString(&lista, 4, descendencia_maxima);
                                break;
                            }
                        }
                        break;
                    }
                }
            }

            if (i == 1)
            {
                // proceso 7
                if (fork() == 0)
                {
                    char cad[TAM_INFO];
                    num_proceso += 4;
                    pid_act = getpid();
                    snprintf(cad, TAM_INFO, "%d (%d)", num_proceso, pid_act);
                    insertarEnLista(&lista, cad);
                    mostrarListaString(&lista, 3, descendencia_maxima);

                    // proceso 9
                    num_proceso+= 2;
                    if (fork() == 0)
                    {
                        char cad[TAM_INFO];
                        pid_act = getpid();
                        snprintf(cad, TAM_INFO, "%d (%d)", num_proceso, pid_act);
                        insertarEnLista(&lista, cad);
                        mostrarListaString(&lista, 4, descendencia_maxima);

                        // proceso 10
                         num_proceso++;
                        if (fork() == 0)
                        {
                            char cad[TAM_INFO];
                            pid_act = getpid();
                            snprintf(cad, TAM_INFO, "%d (%d)", num_proceso, pid_act);
                            insertarEnLista(&lista, cad);
                            mostrarListaString(&lista, 5, descendencia_maxima);
                        }
                    }
                    break;
                }
            }
            break;
        }
    }
    sleep(10);
    return 0;
}

int paramValido(const char *s)
{
    while (*s)
    {
        if (isdigit(*s++) == 0)
            return 0;
    }
    return 1;
}

void crearLista(tLista *p)
{
    *p = NULL;
}

int listaLlena(const tLista *p)
{
    void *aux = malloc(sizeof(tNodo));
    free(aux);
    return aux == NULL;
}

int listaVacia(const tLista *p)
{
    return *p == NULL;
}

int insertarEnLista(tLista *p, const char *d)
{
    tNodo *nue = (tNodo *)malloc(sizeof(tNodo));

    if (nue == NULL)
        return SIN_MEMORIA;

    strcpy(nue->info, d);
    nue->sig = *p;
    *p = nue;

    return TODO_BIEN;
}

void vaciarLista(tLista *p)
{
    tNodo *aux;

    while (*p)
    {
        aux = *p;
        *p = aux->sig;
        free(aux);
    }
}

void mostrarListaString(const tLista *p, int descendencia, int descendencia_maxima)
{
    if(descendencia > descendencia_maxima){
        return;
    }
    while (*p)
    {
        printf("%s", (*p)->info);
        if ((*p)->sig != NULL)
            printf(" - ");
        p = &(*p)->sig;
    }
    printf("\t DESCENDENCIA: %i\n", descendencia);
}
//  ------------------------------FIN------------------------------------------------------------------------
