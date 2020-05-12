/* ========================= BIBLIOTECAS ========================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>          /* errno and error codes */
#include <wait.h>           /* Para wait() */
#include <sys/time.h>	    /* Para gettimeofday() */
#include <sys/sem.h>        /* Para semget(), semop(), semctl() */

/* ========================= CONSTANTES ========================= */

#define BARBERS 3
#define CUSTOMERS 27
#define CHAIRS 7

#define SEM_KEY_BARBER 1243
#define SEM_KEY_CUSTOMER 1244

#define PERMISSION 0666 

/* ========================= VARIÁVEIS E SIMILARES ========================= */

/* Threads e mutex*/
pthread_t barbers[BARBERS];
pthread_t customers[CUSTOMERS];
pthread_mutex_t mutex_barbers;
pthread_mutex_t mutex_customers;	

/* Semáforo */
struct sembuf g_lock_op[1];	
struct sembuf g_unlock_op[1];
int g_sem_id_barber;
int g_sem_id_customer;

/* Mensagens */
/* Barbeiro envia para cliente */
typedef struct {
	unsigned int barber_no;
    char msgBarber[MAXSTRINGSIZE];
    int arraySize;
} data_t_barber; 
data_t_barber barb;

/* Cliente envia para barbeiro */
typedef struct {
	unsigned int customer_no;
    char msgCustomer[MAXSTRINGSIZE];
    int arraySize;
} data_t_customer; 
data_t_customer cust;

/* Para controlar barbeiro */
bool work = true;

/* ========================= FUNÇÕES ========================= */

void *barber(void *);
void *customer(void *);

void semaphoreStruct();
void createSem(int*, int);
void removeSem(int);
void lockSem(int);
void unlockSem(int);

/* ========================= MAIN ========================= */

int main(int argc, char *argv[]){

    int num_barber[BARBERS], num_customer[CUSTOMERS];
    int i;

    /* Inicializa semaforo */
    semaphoreStruct();

    createSem(&g_sem_id_barber, SEM_KEY_BARBER);
    createSem(&g_sem_id_customer, SEM_KEY_CUSTOMER);

    unlockSem(g_sem_id_barber);
    unlockSem(g_sem_id_customer);

    /* Inicializa mutex */
	if(pthread_mutex_init(&mutex_barbers, NULL)){
        printf("Impossível inicializar mutex barbeiro");
        exit(-1);
    }

    if(pthread_mutex_init(&mutex_customers, NULL)){
        printf("Impossível inicializar mutex cliente");
        exit(-1);
    }

    /* Cria as threads barbeiro */
	for (i = 0; i < BARBERS; i++) {
		num_barber[i] = i+1;
		if(pthread_create(&barbers[i], NULL, barber, (void *)&num_barber[i])){
			printf("ERRO: Impossível criar a thread\n");
      			exit(-1);
		}
	}

    /* Cria as threads barbeiro */
	for (i = 0; i < CUSTOMERS; i++) {
		num_customer[i] = i+1;
		if(pthread_create(&customers[i], NULL, customer, (void *)&num_customer[i])){
			printf("ERRO: Impossível criar a thread\n");
      			exit(-1);
		}
	}

    /* Espera barbeiros e clientes */
    for (i = 0; i < CUSTOMERS; i++) {
		if(pthread_join(customers[i],NULL)){
            printf("Impossível esperar thread cliente");
            exit(-1);
        }
	}
    
    work = false;

    for (i = 0; i < BARBERS; i++) {
		if(pthread_join(barbers[i],NULL)){
            printf("Impossível esperar thread barbeiro");
            exit(-1);
        }
	}

    /* Destroi mutex */
    if(pthread_mutex_destroy(&mutex_barbers)){
        printf("Impossível destruir mutex barbeiro");
        exit(-1);
    }
    
    if(pthread_mutex_destroy(&mutex_customers)){
        printf("Impossível destruir mutex cliente");
        exit(-1);
    }

    /* Remove semáforos */
    removeSem(g_sem_id_barber);
    removeSem(g_sem_id_customer);

    return 0;
}

/* ========================= FUNÇÕES REFERENTES AO BARBEIRO E CLIENTES ========================= */

void *barber(void *barberId){

    int *num_barber = barberId;

    /* Atende cliente enquanto trabalhar */
    while(work){

        lockSem(g_sem_id_customer);
        
        unlockSem(g_sem_id_customer);
    }

    pthread_exit(NULL);

}

void *customer(void *customerId){
    
    int *num_customer = customerId;

    struct timeval start_time;
    struct timeval stop_time;

    pthread_exit(NULL);

}

/*Converte string para vetor */
void cut_hair(){
    // 
}

/* Imprime informações */
void apreciate_hair(){
    //
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
