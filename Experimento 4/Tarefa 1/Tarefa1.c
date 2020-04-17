/***********************************************************************************
*
* Este programa não faz parte do curso sobre tempo real do Laboratorio Embry-Riddle
* embora tenha sido inspirado pelos demais experimentos lah existentes.
*
* Experimento # 4 na disciplina de Sistemas Operacionais da PUC-Campinas
* Originalmente programado por Florian Weizenegger
*							Data: 25/08/2003
* 
*       Proposito: O proposito deste programa e o de permitir ao aluno perceber
*       o que vem a ser um thread, de maneira tal que consiga distingui-lo de
*       um processo. Além disso, são usados os principais comandos para criação
*	e manipulação de threads.
*	O problema dos produtores e consumidores sobre um buffer circular é
*	usado como assunto, permitindo que o aluno experimente duas implementações
*	diferentes para sua solução. Desta maneira, além dos threads propriamente
*	ditos, tambem locks e semaforos sao usados para garantir sincronizacao
*	de threads.
*
*************************************************************************************/

/* Lembrar do -pthread para compilar */
/* Para compilar: gcc Tarefa1.c -o tar1 -pthread */

/*
 * Includes Necessarios 
 */
#include <pthread.h>			/* para poder manipular threads */
#include <stdio.h>			/* para printf() */
#include <stdlib.h>
#include <stdbool.h>

/*
 * Constantes Necessarias 
 */
#define NUM_THREADS		10
#define SIZEOFBUFFER		50
#define NO_OF_ITERATIONS	100

/*
 * O tipo pthread_t permite a declaração de uma variável que recebe
 * um id quando o thread é criado. Posteriormente, esse id pode ser
 * usado em comandos de controle para threads.
 * Seguem dois vetores para ids, para um numero de threads igual a
 * constante NUM_THREADS
 */
pthread_t consumers[NUM_THREADS];
pthread_t producers[NUM_THREADS];

/*
 * Variaveis Necessarias 
 */
int buffer[SIZEOFBUFFER];		/* Este e um buffer circular */
int *start;				/* apontara para a primeira posicao do buffer */
int *rp;				/* eh o apontador para o proximo item do buffer a ser consumido */
int *wp;				/* eh o apontador para o proximo item do buffer a ser produzido */
int cont_p = 0;             		/* eh um contador para controlar o numero de itens produzidos */
int cont_c = 0;         		/* eh um contador para controlar o numero de itens consumidos */

/* ============================================================================================================== */


/*
 * Rotina para produzir um item toAdd no buffer 
 */
int myadd(int toAdd) {

	/* Verificação se o buffer esta cheio -> caso nao esteja entra no if */	
	if ((rp != (wp+1)) && (rp + SIZEOFBUFFER - 1 != wp)) {  	
			
		wp++;

		/* Verificacao se wp chegou a ultima posicao do buffer */
		if (wp == (start + SIZEOFBUFFER)) {
			wp = start; /* Realiza a circularidade no bufer */		
		}
			
		*wp = toAdd;
			
		return 1;
	} 
	else return 0;

}


/* ============================================================================================================== */


/*
 * Rotina para consumir um item do buffer e coloca-lo em retValue 
 */
int myremove() {

  //int retValue;

	/* Verificacao se o buffer nao esta vazio -> caso nao esteja entra no if */
	if(wp != rp) {	

		int retValue = *rp;
		rp++;	
	    	
		/* Verificacao se rp chegou a ultima posicao do buffer */
		if (rp == (start + SIZEOFBUFFER)) {
	      		rp = start; /* Realiza a circularidade no buffer */	
	    	}

	    	return retValue;
	} 
	else return 0;

}


/* ============================================================================================================== */


/*
 * A rotina produce e responsavel por chamar myadd para que seja 
 * colocado o valor 10 em uma posicao do buffer NO_OF_ITERATIONS vezes
 */
void *produce(void *threadid) {
  
  int *t_id = threadid;
  int sum = 0;
  int ret = 0;

  //printf("Produtor #%d iniciou...\n", *t_id);

  	while (cont_p < NO_OF_ITERATIONS) {
    		
		ret = myadd(10);

		if (ret) {
			/* 
			 * Pergunta 1: porque ret não está sendo comparado a algum valor?
			 * Pergunta 2: porque nao ha necessidade de um cast?
			 */
			 cont_p++;
			 sum += 10;
		 }
  	}

  printf("Soma produzida pelo Produtor #%d : %d\n", *t_id, sum);
  pthread_exit(NULL);

}


/* ============================================================================================================== */


/*
 * A rotina consume e responsavel por chamar myremove para que seja
 * retorando um dos valores existentes no buffer NO_OF_ITERATIONS vezes 
 */
void *consume(void *threadid) {
  
  int *t_id = threadid;
  int sum = 0;
  int ret = 0;
  cont_c = 0;
  //printf("Consumidor #%d iniciou...\n", *t_id);

  	while (cont_c < NO_OF_ITERATIONS) {
    		
		ret = myremove();

    		if (ret != 0) {
      			cont_c++;
      			sum += 10;
    		}
  	}

  	printf("Soma do que foi consumido pelo Consumidor #%d : %d\n", *t_id, sum);
  	pthread_exit(NULL);

}


/* ============================================================================================================== */


/*
 * Rotina Principal (que tambem e a thread principal, quando executada) 
 */
int main(int argc, char *argv[]) {

  int i;
  int temp1[NUM_THREADS], temp2[NUM_THREADS];

  start = &buffer[0];
  wp = start + SIZEOFBUFFER - 1;
  rp = start;
  
  	for (i = 0; i < NUM_THREADS; i++) {
		
		/* 
		 * Pergunta 3: para que serve cada um dos argumentos usados com pthread_create?
		 */

		/* Tenta criar um thread consumidor */
		/* Retorna 0  se foi criada com sucesso e 1 caso contrario */
		temp1[i] = i + 1;
	    	if (pthread_create(&consumers[i], NULL, consume, (void *)&temp1[i])) {
	      		printf("ERRO: impossivel criar um thread consumidor\n");
	      		exit(-1);
	    	}

    		/* Tenta criar um thread produtor */
		/* Retorna 0  se foi criada com sucesso e 1 caso contrario */
		temp2[i] = i + 1;
    		if (pthread_create(&producers[i], NULL, produce, (void *)&temp2[i])) {
      			printf("ERRO: impossivel criar um thread produtor\n");
      			exit(-1);
    		}
		
  	}
	
	for (i = 0; i < NUM_THREADS; i++) {
		//pthread_detach(consumers[i]);
		//pthread_detach(producers[i]);
		pthread_join(consumers[i], NULL);
		pthread_join(producers[i], NULL);
	}
	
  printf("\nTerminando a thread main()\n");
  pthread_exit(NULL);
  //exit(0);

  /* 
   * Pergunta 4: O que ocorre com as threads criadas, se ainda
   * estiverem sendo executadas e a thread que as criou termina
   * através de um pthread_exit()?
   */

  /*
   * Pergunta 5: Idem à questão anterior, se o termino se da atraves
   * de um exit()?
   */

}
