/* ========================= BIBLIOTECAS ========================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>         
#include <wait.h>           
#include <sys/time.h>	   
#include <sys/sem.h>        

/* ========================= CONSTANTES ========================= */

#define BARBERS 3
#define CUSTOMERS 27
#define CHAIRS 7

#define SEM_KEY_BARBER 1243
#define SEM_KEY_CUSTOMER 1244

#define PERMISSION 0666 

#define MAXSTRINGSIZE 5115

/* ========================= VARIÁVEIS E SIMILARES ========================= */

/* Threads e mutex*/
pthread_t barbers[BARBERS];
pthread_t customers[CUSTOMERS];
pthread_mutex_t mutex;	

/* Semáforo */
struct sembuf g_lock_op[1];	
struct sembuf g_unlock_op[1];
int g_sem_id_barber;
int g_sem_id_customer;

/* Struct de Mensagens */
typedef struct {
	unsigned int barber_no;
    char msgBarber[MAXSTRINGSIZE];

    unsigned int customer_no;
    char msgCustomer[MAXSTRINGSIZE];
    
    int arraySize;
} data_t; 
data_t infos_bc;

/* Para numero de cadeiras */
int numChairs = 0;

/* Quantos clientes foram atendidos */
int atendido = 0;

/* ========================= FUNÇÕES ========================= */

void *barber(void *);
void *customer(void *);

void cut_hair(int[], char[], int);
void apreciate_hair();

void semaphoreStruct();
void createSem(int*, int);
void removeSem(int);
void lockSem(int);
void unlockSem(int);

void arrayToString(int[], char[], int);
void bbsort(int array[], int size);
void clearString(char string[], int size);

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
	if(pthread_mutex_init(&mutex, NULL)){
        printf("Impossível inicializar mutex barbeiro");
        exit(-1);
    }

    /* Cria as threads barbeiro */
	for (i = 0; i < BARBERS; i++) {
		num_barber[i] = i;
		if(pthread_create(&barbers[i], NULL, barber, (void *)&num_barber[i])){
			printf("ERRO: Impossível criar a thread\n");
      			exit(-1);
		}
	}

    /* Cria as threads barbeiro */
	for (i = 0; i < CUSTOMERS; i++) {
		num_customer[i] = i;
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

    int num_barber = *(int *)barberId;

    /* Atende cliente enquanto trabalhar */
    while(atendido < CUSTOMERS){
        lockSem(g_sem_id_customer);
        pthread_mutex_lock(&mutex);
        numChairs = numChairs - 1;
        /* Pega informações da struct */
        //
        //
        //
        //
        pthread_mutex_unlock(&mutex);
        //cut_hair();
        unlockSem(g_sem_id_barber);
    }
    pthread_exit(NULL);

}

void *customer(void *customerId){
    
    int num_customer = *(int *)customerId;

    struct timeval start_time;
    struct timeval stop_time;

    /* Gera tamanho da string */
    srand(time(NULL)*getpid()*num_customer);
    int sizeString = ((rand() % 1021) + 2);

    int array[sizeString];
    /* Gera vetor aleatorio e converte para string */
    srand (time(NULL)*getpid());
    for(int i = 0; i < sizeString; i++){
        array[i] = (rand() % 1021) + 2;  
    } 

    /* String enviada ao barbeiro */
    char stringToBarber[sizeString*5];
    arrayToString(array, stringToBarber, sizeString);

    /* Inicio atendimento */
    gettimeofday(&start_time, NULL);

    bool control = true;
    gettimeofday(&stop_time, NULL);

    while(control){
    
        pthread_mutex_lock(&mutex);

        /* Cliente não pode ser atendido */
        if(numChairs >= CHAIRS){
                
            usleep(100);
            pthread_mutex_unlock(&mutex);

        }else{ 
        
            /* Cliente pode ser atendido */
            numChairs++;
            atendido++;

            /*Coloca informações na struct */
            //
            //
            //

            unlockSem(g_sem_id_customer);
            pthread_mutex_unlock(&mutex);
            lockSem(g_sem_id_barber);
            apreciate_hair();


            control = false;
        }

        pthread_exit(NULL);
    }

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
void apreciate_hair(){
    //
}

/* ========================= FUNÇÕES AUXILIARES ========================= */

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

/* Ordena vetor */
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

/* Limpa string */
void clearString(char string[], int size){
    
    for(int i = 0; i < size; i++){
		string[i] = '\0';
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
