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
}info_p;
info_p info_philosophers;

/* Threads e mutex */
pthread_t philosophers[PHILOSOPHERS];	/* Cada filosofa é uma thread */
pthread_mutex_t forks[PHILOSOPHERS];	/* Trava os talheres*/
pthread_mutex_t mutex;			/* Trava para entrada na regiao critica */


/* ============================================================================================================== */


/* Funções */
void *PhilosophersDinner(void * philosopherid);
void TakeForks(int i);
void PutForks(int i);
void CheckEat(int i);


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
		pthread_mutex_init(&forks[i], NULL);
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
       		pthread_mutex_destroy(&forks[i]);
    	}
    	pthread_mutex_destroy(&mutex);
	
  /* Termina o programa */
  printf("\n");
  exit(0);
}


/* ============================================================================================================== */


void *PhilosophersDinner(void *philosopher_id){
  int *p_id = philosopher_id;

	/* Espera cada filosofa comer 365 vezes */
	while(info_philosophers.eat[*p_id] < NO_OF_ITERATIONS){
		
		/* Pensa */
		info_philosophers.state[*p_id] = THINKING;
		usleep(THINKING_TIME);
		
		/* Pega e coloca os talheres */
		TakeForks(*p_id);
		PutForks(*p_id);

		//break;
	}
	
	/* Quando a filósofa acabar de comer, imprime */
	printf("\n\n=========== Filósofa %d acabou de comer ===========\n", *p_id + 1);		

	/* Encerra thread */
	pthread_exit(NULL);
}

/* ============================================================================================================== */


/* Função que pega os garfos */
void TakeForks(int i){

	/* Trava por conta da RC, verifica se pode comer e depois destrava */
	pthread_mutex_lock(&mutex);
	info_philosophers.state[i] = HUNGRY; /* Esta esperando sua vez de comer */
	CheckEat(i);
	pthread_mutex_unlock(&mutex);
	
	/* Trava o garfo */
	pthread_mutex_lock(&forks[i]);
}


/* ============================================================================================================== */


/* Função que coloca os garfos */
void PutForks(int i){

	/* Trava por conta da RC, verifica se o vizinho da direita e da esquerda podem comer e depois destrava */
	pthread_mutex_lock(&mutex);
	info_philosophers.state[i] = THINKING; /* Filósofa volta a pensar */
	CheckEat((i + 4) % PHILOSOPHERS);
	CheckEat((i + 1) % PHILOSOPHERS);
	pthread_mutex_unlock(&mutex);
}


/* ============================================================================================================== */


/* Função que verifica se a filósofa pode comer */
void CheckEat(int i){
	
	/* Ao iniciar a função, printa o estado de cada filósofa */
	printf("\n\nEstado de cada filósofa");
	
	for(int j = 0; j < PHILOSOPHERS; j++){
		if(info_philosophers.state[j] == THINKING) printf("\nFilósofa %d está pensando", j + 1);
		if(info_philosophers.state[j] == HUNGRY) printf("\nFilósofa %d está com fome", j + 1);
		if(info_philosophers.state[j] == EATING) printf("\nFilósofa %d está comendo", j + 1);
	}

	/* Verifica se a filósofa pode comer */
	if(info_philosophers.state[i] == HUNGRY && info_philosophers.state[(i + 4) % PHILOSOPHERS] != EATING && info_philosophers.state[(i + 1) % PHILOSOPHERS] != EATING){
		
		/* Se puder, muda estado e incrementa quantidade de vezes que comeu */
		info_philosophers.eat[i]++;
		info_philosophers.state[i] = EATING;
		
		/* Destrava garfo */
		pthread_mutex_unlock(&forks[i]);
	}
}
