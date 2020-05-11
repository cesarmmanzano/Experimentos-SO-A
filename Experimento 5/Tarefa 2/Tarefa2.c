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

#define PERMISSION 0666 

/* ========================= VARIÁVEIS E SIMILARES ========================= */

/* Threads */
pthread_t barbers[BARBERS];
pthread_t customers[CUSTOMERS];
pthread_mutex_t mutex_barbers;
pthread_mutex_t mutex_customers;		

/* ========================= FUNÇÕES ========================= */

void *barber(void *);
void *customer(void *);

/* ========================= MAIN ========================= */

int main(){

    int num_barber[BARBERS], num_customer[CUSTOMERS];
    int i;

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
    for (i = 0; i < BARBERS; i++) {
		if(pthread_join(barbers[i],NULL)){
            printf("Impossível esperar thread barbeiro");
            exit(-1);
        }
	}

    for (i = 0; i < CUSTOMERS; i++) {
		if(pthread_join(customers[i],NULL)){
            printf("Impossível esperar thread cliente");
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

    return 0;
}

/* ========================= FUNÇÕES REFERENTES AO BARBEIRO E CLIENTES ========================= */

void barber(void *barberId){

    int *num_barber = barberId;

    pthread_exit(NULL);

}

void *customer(void *customerId){
    
    int *num_customer = customerId;

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