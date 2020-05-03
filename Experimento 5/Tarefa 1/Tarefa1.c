/* ========================= BIBLIOTECAS ========================= */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#define CUSTOMERS 20
#define CHAIRS 7

#define MESSAGE_MTYPE 1
#define MESSAGE_QUEUE_ID_BARBER 3102
#define MESSAGE_QUEUE_ID_CUSTOMER 3103

#define PROTECT
#define SEM_KEY_BARBER      0x1243
#define SEM_KEY_CUSTOMER	0x1244

#define SHM_KEY	0x1432

#define SHM_KEY 0x1432

#define PERMISSION 0666

/* ========================= VARIÁVEIS E SIMILARES ========================= */

//Semáforo
struct sembuf g_lock_op[1];	
struct sembuf g_unlock_op[1];
int g_sem_id_barber;
int g_sem_id_customer;

//Fila de mensagens
typedef struct {
	unsigned int msg_no;
	struct timeval send_time;
} data_t_barber; 

typedef struct {
	unsigned int msg_no;
	struct timeval send_time;
} data_t_customer; 

typedef struct {
	long mtype;
	char mtext[5000];
} msgbuf_t;

//Memoria Compartilhada
int g_shm_id;
int *g_shm_addr;

/* ========================= FUNÇÕES ========================= */

void semaphoreStruct();
void createSem(int*, int);
void removeSem(int);
void lockSem(int);
void unlockSem(int);

void createMessageQueue(int*, key_t);
void removeMessageQueue(int);

void createSharedMemory(int, int*, int**);
void removeSharedMemory(int);

void barber();
void customer();
void cut_hair();
void apreciate_hair();

/* ========================= MAIN ========================= */

int main(){
    
    pid_t rtn_b, rtn_c;
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
    rtn_b = 1;
    for(i = 0; i < BARBERS; i++){
        if( rtn_b != 0 ) {
		    barber_pid[i] = rtn_b = fork();
	    } else {
		    break;
	    }
    }

    rtn_c = 1;
    for(i = 0; i < CUSTOMERS; i++){
        if( rtn_c != 0 ) {
		    customer_pid[i] = rtn_c = fork();
	    } else {
		    break;
	    }
    }

    /* Verifica se o processo é pai ou filho */
    if(rtn_b == 0){
        barber();
    }else{
        for(i = 0; i < BARBERS; i++){
            wait(NULL);
        }
    }

    if(rtn_c == 0){
        customer();
    }else{
        for(i = 0; i < CUSTOMERS; i++){
            wait(NULL);
        }
    }

    /* Matando os processos*/
    for(i = 0; i < BARBERS; i++){
        kill(barber_pid[i], SIGKILL);
    }

    for(i = 0; i < CUSTOMERS; i++){
        kill(customer_pid[i], SIGKILL);
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

  #ifdef PROTECT
        if( semop( g_sem_id, g_lock_op, 1 ) == -1 ) {
            fprintf(stderr,"chamada semop() falhou, impossivel bloquear o semaforo!\n");
            exit(1);
        }
  #endif

}

/* Desbloqueia semáforo */
void unlockSem(int g_sem_id){

  #ifdef PROTECT
        if( semop( g_sem_id, g_unlock_op, 1 ) == -1 ) {
            fprintf(stderr,"chamada semop() falhou, impossivel desbloquear o semaforo!\n");
            exit(1);
        }
  #endif

}

/* ========================= FUNÇÕES REFERENTES A FILA DE MENSAGENS ========================= */

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
		fprintf(stderr,"Impossivel remover a fila!\n");
		exit(1);
	}
                    
}

/* ========================= FUNÇÕES REFERENTES A MEMÓRIA COMPARTILHADA ========================= */

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

/* ========================= FUNÇÕES REFERENTES AO BARBEIRO E CLIENTES ========================= */

void barber(){

}

void customer(){

}
