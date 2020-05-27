/* ========================= BIBLIOTECAS ========================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <wait.h>
#include <sys/time.h>
#include <sys/sem.h>

/* ========================= CONSTANTES ========================= */

#define BARBERS 3
#define CUSTOMERS 27
#define CHAIRS 7

#define PERMISSION 0666

#define MAXSTRINGSIZE 500

#define MICRO_PER_SECOND 1000000

/* ========================= VARIÁVEIS E SIMILARES ========================= */

/* Threads e mutex*/
pthread_t barbers[BARBERS];
pthread_t customers[CUSTOMERS];
pthread_mutex_t mutex;

/* Semáforo */
struct sembuf g_lock_op[CUSTOMERS];
struct sembuf g_unlock_op[CUSTOMERS];
int g_sem_id_barber;   /* Para barbeiros */
int g_sem_id_customer; /* Para clientes */
int g_sem_id_aprHair;  /* Para os prints */

/* Struct de Mensagens */
typedef struct
{
    unsigned int barber_no;

    char msgToCustomer[MAXSTRINGSIZE];

    char msgToBarber[MAXSTRINGSIZE];

    int arraySize;

    int currentCustomer;
} data_t[CUSTOMERS];
data_t infos_bc;

/* Para numero de cadeiras */
int numChairs = 0;

/* Quantos clientes foram atendidos */
int atendido = 0;

/* ========================= FUNÇÕES ========================= */

void *barber(void *);
void *customer(void *);

void cut_hair(int[], char[], int);
void apreciate_hair(int, int, float);

void semaphoreStruct();
void createSem(int *, key_t, int);
void removeSem(int);
void lockSem(int, int);
void unlockSem(int, int);

void arrayToString(int[], char[], int);
void bbsort(int array[], int size);
void clearString(char string[], int size);
void clearStruct();

/* ========================= MAIN ========================= */

int main(int argc, char *argv[])
{
    int num_barber[BARBERS], num_customer[CUSTOMERS];

    /* Inicializa semaforo */
    key_t sem_key_barber = ftok("/tmp", 'a');
    key_t sem_key_customer = ftok("/tmp", 'b');
    key_t sem_key_aprHair = ftok("/tmp", 'c');

    semaphoreStruct();

    createSem(&g_sem_id_barber, sem_key_barber, 1);
    createSem(&g_sem_id_customer, sem_key_customer, CUSTOMERS);
    createSem(&g_sem_id_aprHair, sem_key_aprHair, 1);

    //lockSem(g_sem_id_barber);
    //lockSem(g_sem_id_customer);
    unlockSem(g_sem_id_aprHair, 0);

    /* Inicializa mutex */
    if (pthread_mutex_init(&mutex, NULL))
    {
        printf("Impossível inicializar mutex barbeiro");
        exit(-1);
    }

    clearStruct();

    /* Cria as threads barbeiro */
    for (int i = 0; i < BARBERS; i++)
    {
        num_barber[i] = i;
        if (pthread_create(&barbers[i], NULL, barber, (void *)&num_barber[i]))
        {
            printf("ERRO: Impossível criar a thread\n");
            exit(-1);
        }
    }

    /* Cria as threads barbeiro */
    for (int i = 0; i < CUSTOMERS; i++)
    {
        num_customer[i] = i;
        if (pthread_create(&customers[i], NULL, customer, (void *)&num_customer[i]))
        {
            printf("ERRO: Impossível criar a thread\n");
            exit(-1);
        }
    }

    /* Espera barbeiros e clientes */
    for (int i = 0; i < CUSTOMERS; i++)
    {
        if (pthread_join(customers[i], NULL))
        {
            printf("Impossível esperar thread cliente\n");
            exit(-1);
        }
    }

    for (int i = 0; i < BARBERS; i++)
    {
        if (pthread_cancel(barbers[i]))
        {
            printf("Impossível cancelar thread barbeiro\n");
            exit(-1);
        }
    }

    /* Remove semáforos */
    removeSem(g_sem_id_barber);
    removeSem(g_sem_id_customer);
    removeSem(g_sem_id_aprHair);

    if (pthread_mutex_destroy(&mutex))
    {
        printf("Impossível destruir mutex cliente\n");
        exit(-1);
    }

    return 0;
}

/* ========================= BARBEIROS E CLIENTES ========================= */

void *barber(void *barberId)
{
    int num_barber = *(int *)barberId;

    //int sizeString;
    int currentCustomer = -1;

    /* Atende cliente enquanto trabalhar */
    while (__STDC__)
    {
        currentCustomer = -1;
        //lockSem(g_sem_id_customer);

        pthread_mutex_lock(&mutex);
        for (int i = 0; i < CUSTOMERS; i++)
        {
            if (infos_bc[i].currentCustomer == 0)
            {
                infos_bc[i].currentCustomer = 1;
                infos_bc[i].barber_no = num_barber;
                currentCustomer = i;
                break;
            }
        }
        pthread_mutex_unlock(&mutex);

        if (currentCustomer != -1)
        {
            lockSem(g_sem_id_customer, currentCustomer);
            pthread_mutex_lock(&mutex);

            numChairs = numChairs - 1;

            /* Pega informações da struct */
            int arrayOrdered[infos_bc[currentCustomer].arraySize];
            strcpy(infos_bc[currentCustomer].msgToCustomer, infos_bc[currentCustomer].msgToBarber);
            cut_hair(arrayOrdered, infos_bc[currentCustomer].msgToCustomer, infos_bc[currentCustomer].arraySize);
            bbsort(arrayOrdered, infos_bc[currentCustomer].arraySize);
            arrayToString(arrayOrdered, infos_bc[currentCustomer].msgToCustomer, infos_bc[currentCustomer].arraySize);

            unlockSem(g_sem_id_barber, 0);
            pthread_mutex_unlock(&mutex);
        }
    }

    pthread_exit(NULL);
}

void *customer(void *customerId)
{
    int num_customer = *(int *)customerId;

    struct timeval start_time;
    struct timeval stop_time;

    int sizeString;

    /* Inicio atendimento */
    gettimeofday(&start_time, NULL);

    bool control = true;

    while (control)
    {
        pthread_mutex_lock(&mutex);

        /* Cliente não pode ser atendido */
        if (numChairs >= CHAIRS)
        {
            pthread_mutex_unlock(&mutex);
            usleep(100);
        }
        else
        {
            /* Cliente pode ser atendido */
            numChairs = numChairs + 1;

            unlockSem(g_sem_id_customer, num_customer);

            /* Coloca informações na struct */

            /* Gera tamanho da string */
            srand(time(NULL) * num_customer * 2);
            infos_bc[num_customer].arraySize = (rand() % 98) + 2;

            /* Gera vetor aleatorio e converte para string */
            int array[infos_bc[num_customer].arraySize];
            srand(time(NULL) * num_customer);
            for (int i = 0; i < infos_bc[num_customer].arraySize; i++)
            {
                array[i] = (rand() % 1021) + 2;
            }
            arrayToString(array, infos_bc[num_customer].msgToBarber, infos_bc[num_customer].arraySize);

            infos_bc[num_customer].currentCustomer = 0;

            pthread_mutex_unlock(&mutex);
            lockSem(g_sem_id_barber, 0);

            /* Imprimir informações */
            lockSem(g_sem_id_aprHair, 0);
            gettimeofday(&stop_time, NULL);
            float time = (stop_time.tv_usec - start_time.tv_usec) / (float)MICRO_PER_SECOND;
            apreciate_hair(infos_bc[num_customer].barber_no, num_customer, time);

            control = false;
        }
    }

    pthread_exit(NULL);
}

/* Converte string para vetor */
void cut_hair(int array[], char string[], int size)
{
    int i, k;
    unsigned int j = 0;

    char temp[5];

    for (i = 0; i < size; i++)
    {
        k = 0;
        clearString(temp, 5);
        for (j = j; j < strlen(string); j++)
        {
            if (string[j] != ' ')
            {
                temp[k] = string[j];
                k++;
            }
            else
            {
                array[i] = atoi(temp);
                j++;
                break;
            }
        }
    }
}

/* Imprime informações */
void apreciate_hair(int num_barber, int num_customer, float time)
{
    printf("\n");
    printf("\n======================================================\n");
    printf("Cliente #%d atendido pelo barbeiro #%d\n", num_customer + 1, num_barber + 1);
    printf("Tempo de atendimento: %.8f\n", time);
    printf("String não ordenada: ");
    puts(infos_bc[num_customer].msgToBarber);
    printf("\nString ordenada: ");
    puts(infos_bc[num_customer].msgToCustomer);

    printf("\n======================================================\n");
    printf("\n");
    unlockSem(g_sem_id_aprHair, 0);
}

/* ========================= FUNÇÕES AUXILIARES ========================= */

/* Converte um vetor de inteiros para string */
void arrayToString(int array[], char string[], int size)
{
    int n = 0;
    for (int i = 0; i < size; i++)
    {
        int x = sprintf(&string[n], "%d", array[i]);
        strcat(&string[n + x], " ");
        n = n + x;
        n++;
    }
}

/* Ordena vetor */
void bbsort(int array[], int size)
{
    int temp;

    for (int i = 0; i < (size - 1); i++)
    {

        for (int j = (size - 2); j >= i; j--)
        {

            if (array[j + 1] < array[j])
            {
                temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
            }
        }
    }
}

/* Limpa string */
void clearString(char string[], int size)
{
    for (int i = 0; i < size; i++)
    {
        string[i] = '\0';
    }
}

void clearStruct()
{
    for (int i = 0; i < CUSTOMERS; i++)
    {
        clearString(infos_bc[i].msgToBarber, MAXSTRINGSIZE);
        clearString(infos_bc[i].msgToCustomer, MAXSTRINGSIZE);
        infos_bc[i].currentCustomer = -1;
    }
}

/* ========================= SEMÁFORO ========================= */

/* Construindo a estrutura de controle do semáforo */
void semaphoreStruct()
{
    for (int i = 0; i < CUSTOMERS; i++)
    {
        g_lock_op[i].sem_num = i;
        g_lock_op[i].sem_op = -1;
        g_lock_op[i].sem_flg = 0;

        g_unlock_op[i].sem_num = i;
        g_unlock_op[i].sem_op = 1;
        g_unlock_op[i].sem_flg = 0;
    }
}

/* Cria o semáforo*/
void createSem(int *g_sem_id, key_t sem_key, int size)
{
    if ((*g_sem_id = semget(sem_key, size, IPC_CREAT | PERMISSION)) == -1)
    {
        fprintf(stderr, "chamada a semget() falhou, impossivel criar o conjunto de semaforos!\n");
        exit(1);
    }
}

/* Remove semáforo */
void removeSem(int g_sem_id)
{
    if (semctl(g_sem_id, 0, IPC_RMID, 0) != 0)
    {
        fprintf(stderr, "Impossivel remover o conjunto de semaforos!\n");
        exit(1);
    }
}

/* Bloqueia semaforo */
void lockSem(int g_sem_id, int num)
{
    g_lock_op[num].sem_num = num;
    if (semop(g_sem_id, g_lock_op, 1) == -1)
    {
        fprintf(stderr, "chamada semop() falhou, impossivel bloquear o semaforo!\n");
        exit(1);
    }
}

/* Desbloqueia semáforo */
void unlockSem(int g_sem_id, int num)
{
    g_unlock_op[num].sem_num = num;
    if (semop(g_sem_id, g_unlock_op, 1) == -1)
    {
        fprintf(stderr, "chamada semop() falhou, impossivel desbloquear o semaforo!\n");
        exit(1);
    }
}