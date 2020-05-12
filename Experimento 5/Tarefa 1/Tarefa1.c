/* ========================= BIBLIOTECAS ========================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>          /* errno and error codes */
#include <unistd.h>         /* Para fork() */
#include <signal.h>         /* Para kill(), sigsuspend(), outros */
#include <wait.h>           /* Para wait() */
#include <sys/time.h>	    /* Para gettimeofday() */
#include <sys/types.h>	    /* Para wait(), msgget(), msgctl() */
#include <sys/ipc.h>	    /* Para msgget(), msgctl() */
#include <sys/msg.h>	    /* Para msgget(), msgctl() */
#include <sys/sem.h>        /* Para semget(), semop(), semctl() */
#include <sys/shm.h>        /* Para shmget(), shmat(), shmctl() */

/* ========================= CONSTANTES ========================= */

#define BARBERS 2
#define CUSTOMERS 1
#define CHAIRS 7

#define MESSAGE_MTYPE 1
#define MESSAGE_QUEUE_ID_BARBER 3102
#define MESSAGE_QUEUE_ID_CUSTOMER 3103

#define SEM_KEY_BARBER 1243
#define SEM_KEY_CUSTOMER 1244

#define SHM_KEY	1432
#define SHM_KEY 1432

#define PERMISSION 0666 

#define MAXSTRINGSIZE 4092

#define MICRO_PER_SECOND 1000000

/* ========================= VARIÁVEIS E SIMILARES ========================= */

/* Semáforo */
struct sembuf g_lock_op[1];	
struct sembuf g_unlock_op[1];
int g_sem_id_barber;
int g_sem_id_customer;

/* Fila de mensagens */
/* Barbeiro envia para cliente */
typedef struct {
	unsigned int barber_no;
    char msgBarber[MAXSTRINGSIZE];
    int arraySize;
} data_t_barber; 

/* Cliente envia para barbeiro */
typedef struct {
	unsigned int customer_no;
    char msgCustomer[MAXSTRINGSIZE];
    int arraySize;
} data_t_customer; 

typedef struct {
	long mtype;
	char mtext[5000];
} msgbuf_t;

/* Memoria Compartilhada */
int g_shm_id;
int *g_shm_addr;

/* Para controlar barbeiro */
bool work = true;
 
/* ========================= FUNÇÕES ========================= */

void barber(int, int, int);
void customer(int, int, int);
void cut_hair(int[], char[], int);
void apreciate_hair(int, int, float);

void randomArray(int[], int);
void arrayToString(int[], char[], int); 
void bbsort(int[], int);
void clearString(char[], int);

/* == IPC == */

void semaphoreStruct();
void createSem(int*, int);
void removeSem(int);
void lockSem(int);
void unlockSem(int);

void createMessageQueue(int*, key_t);
void removeMessageQueue(int);

void createSharedMemory(int, int*, int**);
void removeSharedMemory(int);

/* ========================= MAIN ========================= */

int main(int argc, char *argv[]){
    
    printf("oi");

    pid_t rtn;

    /* Vetores com os pids dos barbeiros e clientes */
    pid_t barber_pid[BARBERS], customer_pid[CUSTOMERS];

    int queue_id_barber;
    int queue_id_customer;
    key_t key_barber = MESSAGE_QUEUE_ID_BARBER;
	key_t key_customer = MESSAGE_QUEUE_ID_CUSTOMER;

    int i;

    /* Construindo a estrutura de controle do semáforo */
    semaphoreStruct();

    /* Criando os semáforos */
    createSem(&g_sem_id_barber, SEM_KEY_BARBER);
    createSem(&g_sem_id_customer, SEM_KEY_CUSTOMER);

    /* Desbloqueando os semáforos */
    unlockSem(g_sem_id_barber);
    unlockSem(g_sem_id_customer);

    /* Cria fila de mensagens */
    createMessageQueue(&queue_id_barber, key_barber);
    createMessageQueue(&queue_id_customer, key_customer);

    /* Cria memoria compartilhada */
    createSharedMemory(SHM_KEY, &g_shm_id, &g_shm_addr);

    /* Declara os processos filhos (barbeiros e clientes) */
    rtn = 1;
    for(i = 0; i < BARBERS + CUSTOMERS; i++){
        if( rtn != 0 ) {

		    barber_pid[i] = rtn = fork();
            
            if(rtn == 0){
                
                barber(queue_id_barber, queue_id_customer, i + 1);
            }

	    } else {
		    break;
	    }
    }

    rtn = 1;
    for( i = 0; i < CUSTOMERS; i++){
        if( rtn != 0 ) {

		    customer_pid[i] = rtn = fork();

            if(rtn == 0){
                customer(queue_id_customer, queue_id_barber, i + 1);
            }

	    } else {
		    break;
	    }
    }

    /* Espera o término dos filhos */
    if(rtn != 0){

        for(i = 0; i < CUSTOMERS; i++){
                wait(NULL);
        }

        work = false; /* Para de trabalhar */
        
        /* Matando os processos*/
        for(i = 0; i < CUSTOMERS; i++){
            kill(customer_pid[i], SIGKILL);
        }

        for(i = 0; i < BARBERS; i++){
            kill(barber_pid[i], SIGKILL);
        }

    }

    /* Remove semáforos */
    removeSem(g_sem_id_barber);
    removeSem(g_sem_id_customer);

    /* Remove fila de mensagens */
    removeMessageQueue(queue_id_barber);
    removeMessageQueue(queue_id_customer);

    /* Remove memoria compartilhada */
    removeSharedMemory(g_shm_id);
    
    return 0;
}

/* ========================= FUNÇÕES REFERENTES AO BARBEIRO E CLIENTES ========================= */

void barber(int queue_id_barber, int queue_id_customer, int barber){

    printf("====================");
    
    char stringReceived[MAXSTRINGSIZE*5]; /* Recebe string do cliente */

    msgbuf_t message_send;    /* Mensagem que envia */
    msgbuf_t message_receive;  /* Mensagem que recebe */
    data_t_barber *data_ptr_send = (data_t_barber *)(message_send.mtext);
    data_t_customer *data_ptr_receive = (data_t_customer *)(message_receive.mtext);

    /* Enquanto estiver trabalhando, recebe mensagens do cliente */
    while(work){

        if( msgrcv(queue_id_barber, (struct msgbuf_t *)&message_receive, sizeof(data_t_barber), MESSAGE_MTYPE, 0) == -1 ) {
			fprintf(stderr, "Impossivel receber mensagem!\n");
			exit(1);
		}

    }

    /* Pega informações da mensagem */
    strcpy(stringReceived, data_ptr_receive->msgCustomer);
    int array[data_ptr_receive->arraySize];
    cut_hair(array, stringReceived, data_ptr_receive->arraySize);
    bbsort(array, data_ptr_receive->arraySize);
    arrayToString(array, stringReceived, data_ptr_receive->arraySize);

    /* Apronta dados para enviar mensagem ao cliente */
    message_send.mtype = MESSAGE_MTYPE;
	data_ptr_send->barber_no = barber;
	strcpy(data_ptr_send->msgBarber, stringReceived);
	data_ptr_send->arraySize = data_ptr_receive->arraySize;

    if( msgsnd(queue_id_customer, (struct msgbuf_t *)&message_send, sizeof(data_t_barber), 0) == -1 ) {
		fprintf(stderr, "Impossivel enviar mensagem!\n");
	    exit(1);
	}

    /* Início da RC */
    lockSem(g_sem_id_barber);
    
    *g_shm_addr = *g_shm_addr - 1;

    unlockSem(g_sem_id_barber);

    exit(0);
    
}

void customer(int queue_id_customer, int queue_id_barber, int customer){

    int sizeString = (rand() % 1021) + 2; /* Tamanho da string que será passada ao barbeiro */
    int array[sizeString]; /* Armazena valores gerados */
    char stringtoBarber[sizeString*5]; /* String que será passada ao barbeiro */
    char stringOrdered[sizeString*5]; /* String que conterá a string organizada */
    int arrayOrdered[sizeString]; /* Vetor de inteiros ordenado*/

    msgbuf_t message_send; /* Mensagem que envia */
    msgbuf_t message_receive; /* Mensagem que recebe */
    data_t_barber *data_ptr_receive = (data_t_barber *)(message_send.mtext); /* Ponteiro para os dados */
    data_t_customer *data_ptr_send = (data_t_customer *)(message_receive.mtext); /* Ponteiro para os dados */

    struct timeval start_time; /* Instante em que entra e senta na sala */
    struct timeval stop_time; /* Instante em que inicia o corte */

    /* Gera vetor aleatorio e converte para string */
    for(int i = 0; i < sizeString; i++){
        array[i] = (rand() % 1021) + 2;  
    } 
    arrayToString(array, stringtoBarber, sizeString);

    /* Pega tempo atual */
    gettimeofday(&start_time, NULL);

    /* Início da RC */
    lockSem(g_sem_id_customer);
    
    if(*g_shm_addr >= 7){
        
        /* Cliente não atendido */
        unlockSem(g_sem_id_customer);
        exit(0);

    }else{
        
        /*Cliente atendido*/
        *g_shm_addr = *g_shm_addr + 1;
        unlockSem(g_sem_id_customer);

        // Apronta os dados para enviar mensagem
        message_send.mtype = MESSAGE_MTYPE;
	    data_ptr_send->customer_no = customer;
	    strcpy(data_ptr_send->msgCustomer, stringtoBarber);
	    data_ptr_send->arraySize = sizeString;

        if( msgsnd(queue_id_barber, (struct msgbuf_t *)&message_send, sizeof(data_t_customer), 0) == -1 ) {
		    fprintf(stderr, "Impossivel enviar mensagem!\n");
		    exit(1);
	    }

    }

    /* Cliente recebe mensagem do barbeiro -> acabou de cortar o cabelo */
    if( msgrcv(queue_id_customer, (struct msgbuf_t *)&message_receive, sizeof(data_t_barber), MESSAGE_MTYPE, 0) == -1 ) {
		fprintf(stderr, "Impossivel receber mensagem!\n");
		exit(1);
	}
    gettimeofday(&stop_time, NULL);

    strcpy(stringOrdered, data_ptr_receive->msgBarber);
    cut_hair(arrayOrdered, stringOrdered, data_ptr_receive->arraySize);

    float time;
    time = (float)(stop_time.tv_sec  - start_time.tv_sec);
	time += (stop_time.tv_usec - start_time.tv_usec)/(float)MICRO_PER_SECOND; 

    apreciate_hair(data_ptr_receive->barber_no, customer, time);

    exit(0);

}

/*Converte string para vetor */
void cut_hair(int array[], char string[], int size){

    int i, k;
    unsigned int j = 0;

    char temp[5];

    for(i = 0; i < size; i++){
		printf("\n");
		k = 0;
		clearString(temp, 5);
		for(j = j; j < strlen(string); j++){
			if(string[j] != ' '){
				temp[k] = string[j];
				k++;
			}else{
				array[i] = atoi(temp);
				j++;
				break;
			}

		}

	}

}

/* Imprime informações */
void apreciate_hair(int barber, int customer, float time){
    printf("\n");
    printf("Cliente #%d foi atendido pelo barbeiro #%d", customer, barber);
    printf("\nTempo aproximado para o cliente ser atendido: %.2f", time);
    printf("\nString a ser ordenada: ");
    printf("\nString Ordenada:");
    printf("\n");
}

/* ========================= FUNÇÕES AUXILIARES ========================= */

/* Gera um vetor aleatorio de inteiros */
void randomArray(int array[], int size){

    for(int i = 0; i < size; i++){
        array[i] = (rand() % 1021) + 2;
        //printf("%d ", array[i]);
    } 
    //printf("\n\n");

}

/* Converte um vetor de inteiros para string */
void arrayToString(int array[], char string[], int size){

    int n = 0;
    for(int i = 0; i < size; i++) {
        int x = sprintf(&string[n], "%d", array[i]);
        strcat(&string[n+x], " ");
		n = n + x;
		n++;
    }

}

void bbsort(int array[], int size){
    
    int temp;

    for(int i = 0; i < (size - 1); i++){

        for(int j = (size - 2); j >= i; j--){

            if(array[j+1] > array[j]){
                temp = array[j];
                array[j] = array[j+1];
                array[j+1] = temp;
            }

        }

    }
}

void clearString(char str[], int size){
	
	for(int i = 0; i < size; i++){
		str[i] = '\0';
	}

}

/* ========================= FUNÇÕES REFERENTES AOS SEMÁFOROS ========================= */

/* Construindo a estrutura de controle do semáforo */
void semaphoreStruct(){

    g_lock_op[0].sem_num = 0;
    g_lock_op[0].sem_op = -1;
    g_lock_op[0].sem_flg = 0;

    g_unlock_op[0].sem_num = 0;
    g_unlock_op[0].sem_op = 1;
    g_unlock_op[0].sem_flg = 0;

}

/* Cria o semáforo*/
void createSem(int *g_sem_id, int sem_key){

	if( ( *g_sem_id = semget( sem_key, 1, IPC_CREAT | PERMISSION ) ) == -1 ) {
		fprintf(stderr,"chamada a semget() falhou, impossivel criar o conjunto de semaforos!\n");
		exit(1);
	}

}

/* Remove semáforo */
void removeSem(int g_sem_id){

    if( semctl( g_sem_id, 0, IPC_RMID, 0) != 0 ) {
        fprintf(stderr,"Impossivel remover o conjunto de semaforos!\n");
        exit(1);
    }

}

/* Bloqueia semaforo */
void lockSem(int g_sem_id){

        if( semop( g_sem_id, g_lock_op, 1 ) == -1 ) {
            fprintf(stderr,"chamada semop() falhou, impossivel bloquear o semaforo!\n");
            exit(1);
        }

}

/* Desbloqueia semáforo */
void unlockSem(int g_sem_id){

        if( semop( g_sem_id, g_unlock_op, 1 ) == -1 ) {
            fprintf(stderr,"chamada semop() falhou, impossivel desbloquear o semaforo!\n");
            exit(1);
        }

}

/* ========================= FUNÇÕES REFERENTES À FILA DE MENSAGENS ========================= */

/* Cria fila de Mensagens */
void createMessageQueue(int *queue_id, key_t key){

    if( (*queue_id = msgget(key, IPC_CREAT | PERMISSION)) == -1 ) {
		fprintf(stderr,"Impossivel criar a fila de mensagens!\n");
		exit(1);
	}

}

/* Remove fila de mensagens */
void removeMessageQueue(int queue_id){

    if( msgctl(queue_id,IPC_RMID,NULL) == -1 ) {
		fprintf(stderr,"Impossivel remover a fila de mensagens!\n");
		exit(1);
	}
                    
}

/* ========================= FUNÇÕES REFERENTES À MEMÓRIA COMPARTILHADA ========================= */

/* Cria memória compartilhada */
void createSharedMemory(int shm_key, int *g_shm_id, int **g_shm_addr){

    if( (*g_shm_id = shmget( shm_key, sizeof(int), IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar o segmento de memoria compartilhada!\n");
		exit(1);
	}
	if( (*g_shm_addr = (int *)shmat(*g_shm_id, NULL, 0)) == (int *)-1 ) {
		fprintf(stderr,"Impossivel associar o segmento de memoria compartilhada!\n");
		exit(1);
	}
    **g_shm_addr = 0;

}

/* Remove memória compartilhada */
void removeSharedMemory(int g_shm_id){

    if( shmctl(g_shm_id, IPC_RMID, NULL) != 0 ) {
        fprintf(stderr,"Impossivel remover o segmento de memoria compartilhada!\n");
        exit(1);
    }

}