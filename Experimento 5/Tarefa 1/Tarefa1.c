/* Bibliotecas Necessárias */
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

/* ================================================== */

/* Constantes Necessárias */
#define BARBERS 2
#define CLIENTS 20
#define CHAIRS  7

#define MESSAGE_MTYPE       1
#define MESSAGE_QUEUE_ID    3102

#define PROTECT
#define SEM_KEY_BARBER	0x1243
#define SEM_KEY_CLIENT	0x1244

#define SHM_KEY			0x1432

#define PERMISSION 0666

/* ================================================== */

/* Variaveis e similares */
//Semáforo
struct sembuf	g_lock_op[1];	
struct sembuf	g_unlock_op[1];
int g_sem_id_barber;
int g_sem_id_client;

//Fila de mensagens
typedef struct {
	long mtype;
	char mtext[5000];
} msgbuf_t;

/* ================================================== */

/* Funções */
void semaphoreStruct();
void createSem(int*, int);
void destroySem(int);
void lockSem(int);
void unlockSem(int);

/* ================================================== */

int main(){
    
    pid_t rtn_b, rtn_c;
    pid_t barber[BARBERS], client[CLIENTS];
    int i;

    /* Construindo a estrutura de controle do semáforo */
    semaphoreStruct();

    /* Criando os semáforos */
    createSem(&g_sem_id_barber, SEM_KEY_BARBER);
    createSem(&g_sem_id_client, SEM_KEY_CLIENT);

    /* Desbloqueando os semáforos */
    unlockSem(g_sem_id_barber);
    unlockSem(g_sem_id_client);

    /* Declara os processos filhos (barbeiros e clientes) */
    rtn_b = 1;
    for(i = 0; i < BARBERS; i++){
        if( rtn_b != 0 ) {
			barber[i] = rtn_b = fork();
		} else {
			break;
		}
    }

    rtn_c = 1;
    for(i = 0; i < CLIENTS; i++){
        if( rtn_c != 0 ) {
			client[i] = rtn_c = fork();
		} else {
			break;
		}
    }

    /* Verifica se o processo é pai ou filho */
    if(rtn_b == 0){
        //chama função barbeiro
        printf("barbeiro\n");
    }else{
        for(i = 0; i < BARBERS; i++){
            wait(NULL);
        }
    }

    if(rtn_c == 0){
        //chama função cliente
        printf("cliente\n");
    }else{
        for(i = 0; i < CLIENTS; i++){
            wait(NULL);
        }
    }

    /* Matando os processos*/
    for(i = 0; i < BARBERS; i++){
        kill(barber[i], SIGKILL);
    }

    for(i = 0; i < CLIENTS; i++){
        kill(client[i], SIGKILL);
    }

    /* Destroi/Remove semáforo */
    destroySem(g_sem_id_barber);
    destroySem(g_sem_id_client);
    
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

/* Criando o semáforo*/
void createSem(int *g_sem_id, int sem_key){

    if( ( *g_sem_id = semget( sem_key, 1, IPC_CREAT | PERMISSION ) ) == -1 ) {
		fprintf(stderr,"chamada a semget() falhou, impossivel criar o conjunto de semaforos!\n");
		exit(1);
	}

}

/* Remove semáforo */
void destroySem(int g_sem_id){

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