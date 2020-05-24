/* ========================= BIBLIOTECAS ========================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>      /* for time() */
#include <errno.h>     /* errno and error codes */
#include <unistd.h>    /* for fork() */
#include <signal.h>    /* for kill(), sigsuspend(), outros */
#include <wait.h>      /* for wait() */
#include <sys/time.h>  /* for gettimeofday() */
#include <sys/types.h> /* for wait(), msgget(), msgctl() */
#include <sys/ipc.h>   /* for all IPC function calls */
#include <sys/msg.h>   /* for msgget(), msgctl() */
#include <sys/sem.h>   /* for semget(), semop(), semctl() */
#include <sys/shm.h>   /* for shmget(), shmat(), shmctl() */

/* ========================= CONSTANTES ========================= */

#define BARBERS 2
#define CUSTOMERS 20
#define CHAIRS 7

#define PERMISSION 0666

#define MAXSTRINGSIZE 5115

#define MICRO_PER_SECOND 1000000

/* ========================= VARIÁVEIS E SIMILARES ========================= */

/* Fila de mensagens */

/* Barbeiro envia para cliente */
typedef struct
{
    unsigned int barber_no;
    unsigned int customer_no;
    char msgBarber[MAXSTRINGSIZE];
    int arraySize;
} data_t_barber;

/* Cliente envia para barbeiro */
typedef struct
{
    unsigned int customer_no;
    char msgCustomer[MAXSTRINGSIZE];
    int arraySize;
} data_t_customer;

typedef struct
{
    long mtype;
    char mtext[5555];
} msgbuf_t;

int queue_id;

/* Memoria Compartilhada */
int g_shm_id;
int *g_shm_addr;

/* Semáforo */
struct sembuf g_lock_op[1];
struct sembuf g_unlock_op[1];
int g_sem_id_barber;
int g_sem_id_customer;

/* ========================= FUNÇÕES ========================= */

void barber(int);
void customer(int);
void cut_hair(int[], char[], int);
void apreciate_hair(int, int, float, int[], int[], int);

void arrayToString(int[], char[], int);
void bbsort(int[], int);
void clearString(char[], int);

void semaphoreStruct();
void createSem(int *, key_t);
void removeSem(int);
void lockSem(int);
void unlockSem(int);

/* ========================= MAIN ========================= */

int main()
{

    pid_t rtn;

    /* Vetores com os pids dos barbeiros e clientes */
    //pid_t barber_pid[BARBERS];
    //pid_t customer_pid[CUSTOMERS];

    key_t key_msg = ftok("/tmp", 'a');
    key_t key_shm = ftok("/tmp", 'b');

    key_t sem_key_barber = ftok("/tmp", 'c');
    key_t sem_key_customer = ftok("/tmp", 'd');

    int i;

    /* Cria fila de mensagens */
    if ((queue_id = msgget(key_msg, IPC_CREAT | PERMISSION)) == -1)
    {
        fprintf(stderr, "Impossivel criar a fila de mensagens!\n");
        exit(1);
    }

    /* Cria memória compartilhada */
    if ((g_shm_id = shmget(key_shm, sizeof(int), IPC_CREAT | PERMISSION)) == -1)
    {
        fprintf(stderr, "Impossivel criar o segmento de memoria compartilhada!\n");
        exit(1);
    }
    if ((g_shm_addr = (int *)shmat(g_shm_id, NULL, 0)) == (int *)-1)
    {
        fprintf(stderr, "Impossivel associar o segmento de memoria compartilhada!\n");
        exit(1);
    }
    *g_shm_addr = 0;

    /* Cria semaforos */
    semaphoreStruct();

    createSem(&g_sem_id_barber, sem_key_barber);
    createSem(&g_sem_id_customer, sem_key_customer);

    unlockSem(g_sem_id_barber);
    unlockSem(g_sem_id_customer);

    /* Declara os processos filhos (barbeiros e clientes) */
    rtn = 1;
    for (i = 0; i < BARBERS + CUSTOMERS; i++)
    {
        if (rtn != 0)
        {

            rtn = fork();
        }
        else
        {
            break;
        }
    }

    if (rtn == 0)
    { /* Filho */

        if (i <= BARBERS)
        {
            barber(i);
        }
        else
        {
            customer(i - 2);
        }
    }
    else
    { /* Pai */

        for (int j = 0; j < CUSTOMERS; j++)
        {
            wait(NULL);
        }

        for (int j = 0; j < BARBERS; j++)
        {
            wait(NULL);
        }

        /* Remove fila de mensagens */
        if (msgctl(queue_id, IPC_RMID, NULL) == -1)
        {
            fprintf(stderr, "Impossivel remover a fila de mensagens!\n");
            exit(1);
        }

        /* Remove memoria compartilhada */
        if (shmctl(g_shm_id, IPC_RMID, NULL) != 0)
        {
            fprintf(stderr, "Impossivel remover o segmento de memoria compartilhada!\n");
            exit(1);
        }

        /* Remove semáforos */
        removeSem(g_sem_id_barber);
        removeSem(g_sem_id_customer);
    }

    return 0;
}

/* ========================= BARBEIROS E CLIENTES ========================= */

void barber(int barber)
{

    //usleep(1000000);
    msgbuf_t message_send;                                                          /* Mensagem que envia */
    msgbuf_t message_receive;                                                       /* Mensagem que recebe */
    data_t_barber *data_ptr_send = (data_t_barber *)(message_send.mtext);           /* Ponteiro para os dados */
    data_t_customer *data_ptr_receive = (data_t_customer *)(message_receive.mtext); /* Ponteiro para os dados */

    /* Recebe mensagens do cliente */
    if (msgrcv(queue_id, (struct msgbuf_t *)&message_receive, sizeof(data_t_customer), 1, 0) == -1)
    {
        fprintf(stderr, "Impossivel receber mensagem!\n");
        exit(1);
    }

    char stringReceived[data_ptr_receive->arraySize]; /* String do cliente */

    /* Pega informações da mensagem */
    strcpy(stringReceived, data_ptr_receive->msgCustomer);
    int array[data_ptr_receive->arraySize];
    cut_hair(array, stringReceived, data_ptr_receive->arraySize);
    bbsort(array, data_ptr_receive->arraySize);
    arrayToString(array, stringReceived, data_ptr_receive->arraySize);

    /* Apronta dados para enviar mensagem ao cliente */
    message_send.mtype = data_ptr_receive->customer_no + 50;
    data_ptr_send->barber_no = barber;
    data_ptr_send->customer_no = data_ptr_receive->customer_no;
    strcpy(data_ptr_send->msgBarber, stringReceived);
    data_ptr_send->arraySize = data_ptr_receive->arraySize;

    /* RC */
    *g_shm_addr = *g_shm_addr - 1;

    if (msgsnd(queue_id, (struct msgbuf_t *)&message_send, sizeof(data_t_barber), 0) == -1)
    {
        fprintf(stderr, "Impossivel enviar mensagem!\n");
        exit(1);
    }
}

void customer(int customer)
{

    srand(time(NULL) * customer * getpid());
    int sizeString = (rand() % 1021) + 2; /* Tamanho da string que será passada ao barbeiro */
    int array[sizeString];                /* Armazena valores gerados */
    char stringtoBarber[sizeString * 5];  /* String que será passada ao barbeiro */
    char stringOrdered[sizeString * 5];   /* String que conterá a string organizada */
    int arrayOrdered[sizeString];         /* Vetor de inteiros ordenado*/

    msgbuf_t message_send;                                                       /* Mensagem que envia */
    msgbuf_t message_receive;                                                    /* Mensagem que recebe */
    data_t_barber *data_ptr_receive = (data_t_barber *)(message_send.mtext);     /* Ponteiro para os dados */
    data_t_customer *data_ptr_send = (data_t_customer *)(message_receive.mtext); /* Ponteiro para os dados */

    struct timeval start_time; /* Instante em que entra e senta na sala */
    struct timeval stop_time;  /* Instante em que inicia o corte */

    /* Gera vetor aleatorio e converte para string */
    srand(time(NULL) * getpid());
    for (int i = 0; i < sizeString; i++)
    {
        array[i] = (rand() % 1021) + 2;
    }
    arrayToString(array, stringtoBarber, sizeString);

    /* Pega tempo atual */
    gettimeofday(&start_time, NULL);

    /* Início da RC */
    if (*g_shm_addr >= CHAIRS)
    {

        /* Cliente não atendido */
        printf("Cliente #%d nao foi atendido\n", customer);
        exit(0);
    }
    else
    {

        /* Cliente atendido */
        *g_shm_addr = *g_shm_addr + 1;

        /* Apronta os dados para enviar mensagem */
        message_send.mtype = customer + 50;
        data_ptr_send->customer_no = customer;
        strcpy(data_ptr_send->msgCustomer, stringtoBarber);
        data_ptr_send->arraySize = sizeString;

        if (msgsnd(queue_id, (struct msgbuf_t *)&message_send, sizeof(data_t_customer), 0) == -1)
        {
            fprintf(stderr, "Impossivel enviar mensagem!\n");
            exit(1);
        }

        /* Cliente recebe mensagem do barbeiro -> acabou de cortar o cabelo */
        if (msgrcv(queue_id, (struct msgbuf_t *)&message_receive, sizeof(data_t_barber), customer + 50, 0) == -1)
        {
            fprintf(stderr, "Impossivel receber mensagem!\n");
            exit(1);
        }
        gettimeofday(&stop_time, NULL);

        strcpy(stringOrdered, data_ptr_receive->msgBarber);
        cut_hair(arrayOrdered, stringOrdered, data_ptr_receive->arraySize);

        float time;
        time = (float)(stop_time.tv_sec - start_time.tv_sec);
        time += (stop_time.tv_usec - start_time.tv_usec) / (float)MICRO_PER_SECOND;

        apreciate_hair(data_ptr_receive->barber_no, data_ptr_receive->customer_no, time, array, arrayOrdered, sizeString);
    }
}

/*Converte string para vetor */
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
void apreciate_hair(int barber, int customer, float time, int array[], int arrayOrdered[], int size)
{
    printf("\n");
    //printf("\n======================================================\n");
    printf("Cliente #%d foi atendido pelo barbeiro #%d", customer, barber);
    printf("\nTempo aproximado para o cliente ser atendido: %.10f", time);
    printf("\nString a ser ordenada:\n");
    for (int i = 0; i < 5; i++)
    {
        printf("%d ", array[i]);
    }
    printf("\nString Ordenada:\n");
    for (int i = 0; i < 5; i++)
    {
        printf("%d ", arrayOrdered[i]);
    }
    printf("\n");
    //printf("\n======================================================\n");
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

            if (array[j + 1] > array[j])
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

/* ========================= SEMÁFOROS ========================= */

/* Construindo a estrutura de controle do semáforo */
void semaphoreStruct()
{

    g_lock_op[0].sem_num = 0;
    g_lock_op[0].sem_op = -1;
    g_lock_op[0].sem_flg = 0;

    g_unlock_op[0].sem_num = 0;
    g_unlock_op[0].sem_op = 1;
    g_unlock_op[0].sem_flg = 0;
}

/* Cria o semáforo*/
void createSem(int *g_sem_id, key_t sem_key)
{

    if ((*g_sem_id = semget(sem_key, 1, IPC_CREAT | PERMISSION)) == -1)
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
void lockSem(int g_sem_id)
{

    if (semop(g_sem_id, g_lock_op, 1) == -1)
    {
        fprintf(stderr, "chamada semop() falhou, impossivel bloquear o semaforo!\n");
        exit(1);
    }
}

/* Desbloqueia semáforo */
void unlockSem(int g_sem_id)
{

    if (semop(g_sem_id, g_unlock_op, 1) == -1)
    {
        fprintf(stderr, "chamada semop() falhou, impossivel desbloquear o semaforo!\n");
        exit(1);
    }
}