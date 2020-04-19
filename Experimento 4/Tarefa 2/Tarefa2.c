/* ============================================================================================================== */

/* gcc Tarefa2.c -o tar2 -pthread */

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
#include <locale.h>

/* Constantes necessárias */
#define PHILOSOPHERS 5
#define NO_OF_ITERATIONS 365
#define THINKING_TIME 25

/* Estados das filosofas */
#define THINKING 0
#define HUNGRY 1
#define EATING 2

/* Struct que armazena informações sobre as filosofas */
typedef struct {
  	int eat[PHILOSOPHERS];		/* Vetor para contar quantas vezes as filosofas ja comeram*/
	int state[PHILOSOPHERS];	/* Vetor para armazenar o estado de cada filosofa */	
}philo;
philo info_philosophers;

/* Threads e mutex */
pthread_t philosophers[PHILOSOPHERS];	/* Cada filosofa é uma thread */
pthread_mutex_t cutlery[PHILOSOPHERS];	/* Trava os talheres*/
pthread_mutex_t mutex;			/* Trava thread */


/* ============================================================================================================== */


/* Funções */
void *PhilosophersDinner(void * philosopherid);
void PrintState();
void CheckEat();


/* ============================================================================================================== */


int main(){
  
  setlocale(LC_ALL, "Portuguese");

  int i;
  int num_philosopher[PHILOSOPHERS];
  
  printf("\n");

	/* Inicializa estado de cada filosofa (todas iniciam pensando)*/
	for(i = 0; i < PHILOSOPHERS; i++){
		info_philosophers.state[i] = THINKING;
	}
	
	/* Inicializa quantidade de vezes que cada filosofa comeu */
	for(i = 0; i < PHILOSOPHERS; i++){
		info_philosophers.eat[i] = 0;
	}

  	/* Inicializar mutex */
	for(i = 0; i < PHILOSOPHERS; i++){
		pthread_mutex_init(&cutlery[i], NULL);
	}
	pthread_mutex_init(&mutex, NULL);
	
  	/* Criar as 5 threads */
	for (i = 0; i < PHILOSOPHERS; i++) {
		num_philosopher[i] = i;
		if(pthread_create(&philosophers[i], NULL, PhilosophersDinner, (void *)&num_philosopher[i])){
			printf("ERRO: Impossível criar a thread\n");
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
  printf("\n");
  exit(0);
}


/* ============================================================================================================== */


void *PhilosophersDinner(void *philosopherid){
  int *p_id = philosopherid;

	/* Espera cada filosofa comer 365 vezes */
	while(info_philosophers.eat[*p_id] < NO_OF_ITERATIONS){
		/* Cada filosofa pensa e pega os talheres */
		
		/* Pensa */
		info_philosophers.state[*p_id] = THINKING;
		usleep(THINKING_TIME);
		
		/* Pega os talheres */
		//TakeCutlery();
		//PutCutlery();

		break;
	}
	
	/* Quando a filósofa acabar de comer, imprime */
	printf("Filósofa %d acabou de comer\n", *p_id + 1);		

	/* Encerra thread */
	pthread_exit(NULL);
}


/* ============================================================================================================== */


/* Função que verifica se a filosofa pode comer */
void CheckEat(){
	PrintState();
}


/* ============================================================================================================== */


/* ============================================================================================================== */


/* Função que printa o estado de cada filosofa */
void PrintState(){
	printf("\nEstado de cada filosofa");
	
	for(int i = 0; i < PHILOSOPHERS; i++){
		if(info_philosophers.state[i] == THINKING) printf("Filósofa %d está pensando", i + 1);
		if(info_philosophers.state[i] == HUNGRY) printf("Filósofa %d está com fome", i + 1);
		if(info_philosophers.state[i] == EATING) printf("Filósofa %d está comendo", i + 1);
	}
}


/* ============================================================================================================== */
