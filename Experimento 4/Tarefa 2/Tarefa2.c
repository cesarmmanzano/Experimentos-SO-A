/*
 *	Programe a solução do jantar das filósofas, fazendo com que cada filósofa seja
 * 	uma thread. Para estabelecer exclusão mútua use mutex. Para isso, utilize as
 *	chamadas pthread_mutex_init(), pthread_mutex_lock(), pthread_mutex_unlock() e
 *	pthread_mutex_destroy(). No lugar de pensando programe uma espera de 25
 *	microsegundos. A cada teste para saber se pode comer, apresentar qual é a
 *	filósofa que chamou testa e como se encontram os estados das cinco filósofas.
 *	Cada filósofa deve terminar depois de ter comido 365 vezes. Use pthread_join()
 *	para aguardar o término das filósofas.
 */


/* ============================================================================================================== */


/* Bibliotecas necessárias */
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/* Constantes necessárias */
/* Define numero de filosofos e quantas vezes cada um comerá */
#define PHILOSOPHERS 5
#define NO_OF_ITERATIONS 365

/* Estados das filosofas */
#define THINKING 0
#define HUNGRY 1
#define EATING 2

/* Vetor para contar quantas vezes os filosofos ja comeram*/
int eat[PHILOSOPHERS];

/* Threads e mutex */
pthread_t philosophers[PHILOSOPHERS];
pthread_mutex_t cutlery[PHILOSOPHERS];
pthread_mutex_t mutex;


/* ============================================================================================================== */


/* Funções */
void *philosophers_dinner(void * philosopherid);

/* ============================================================================================================== */


int main(){
  int i;
  int num_philosopher[PHILOSOPHERS];

  	/* Inicializar mutex */
	for(i = 0; i < PHILOSOPHERS; i++){
		pthread_mutex_init(&cutlery[i], NULL);
	}
	pthread_mutex_init(&mutex, NULL);
	
  	/* Criar as 5 threads */
	for (i = 0; i < PHILOSOPHERS; i++) {
		num_philosopher[i] = i;
		if(pthread_create(&philosophers[i], NULL, philosophers_dinner, (void *)&num_philosopher[i])){
			printf("ERRO: impossivel criar a thread\n");
      			exit(-1);
		}
	}

  	/* Esperar o termino das 5 threads */
	for (i = 0; i < PHILOSOPHERS; i++) {
		 pthread_join(philosophers[i],NULL);
	}

  	/* Destruir mutex */
	for (i = 0; i < PHILOSOPHERS; ++i) {
       		pthread_mutex_destroy(&cutlery[i]);
    	}
    	pthread_mutex_destroy(&mutex);
	
  /* Termina o programa */
  exit(0);
}


/* ============================================================================================================== */


void *philosophers_dinner(void *philosopherid){
  int *p_id = philosopherid;
	
	/* Enquanto a filosofa nao comer 365 vezes */
	while(eat[*p_id] < NO_OF_ITERATIONS){
		break;
	}
}


/* ============================================================================================================== */
