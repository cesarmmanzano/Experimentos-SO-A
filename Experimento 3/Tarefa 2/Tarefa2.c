/*******************************************************************************
*
* Este programa faz parte do curso sobre tempo real do Laboratorio Embry-Riddle
* 
* Seguem os comentarios originais:
*
* Experiment #5: Semaphores
*
*    Programmer: Eric Sorton
*          Date: 3/17/97
*           For: MSE599, Special Topics Class
*
*       Purpose: The purpose of this program is to demonstrate how semaphores
*		 can be used to protect a critical region.  Its sole purpose
*		 is to print a character string (namely the alphabet) to the
*		 screen.  Any number of processes can be used to cooperatively
*		 (or non-cooperatively) print the string to the screen.  An
*		 index is stored in shared memory, this index is the index into
*		 the array that identifies which character within the string
*		 should be printed next.  Without semaphores, all the processes
*		 access this index simultaneously and conflicts occur.  With
*		 semahpores, the character string is displayed neatly to the
*		 screen.
*
*		 The optional semaphore protection can be compiled into the
*		 program using the MACRO definition of PROTECT.  To compile
*		 the semaphore protection into the program, uncomment the
*		 #define below.
*
*
*       Proposito: O proposito deste programa e o de demonstrar como semaforos
*		podem ser usados para proteger uma regiao critica. O programa exibe
*		um string de caracteres (na realidade um alfabeto). Um número 
*		qualquer de processos pode ser usado para exibir o string, seja
*		de maneira cooperativa ou nao cooperativa. Um indice e armazenado
*		em memoria compartilhada, este indice e aquele usado para 
* 		identificar qual caractere deve ser exibido em seguida. Sem 
*		semaforos, todos os processos acessam esse indice concorrentemente 
*		causando conflitos. Com semaforos, o string de caracteres e exibido
*		de maneira correta (caracteres do alfabeto na ordem correta e apenas
*		um de cada caractere).
*
*		A protecao opcional com semaforo pode ser compilada no programa
*		usando a definicao de MACRO denominada PROTECT. Para compilar a
*		protecao com semaforo, retire o comentario do #define que segue.
*
*
*******************************************************************************/



#define PROTECT



/*
 * Includes Necessarios 
 */
#include <errno.h>              /* errno and error codes */
#include <sys/time.h>           /* for gettimeofday() */
#include <stdio.h>              /* for printf() */
#include <unistd.h>             /* for fork() */
#include <sys/types.h>          /* for wait() */
#include <sys/wait.h>           /* for wait() */
#include <signal.h>             /* for kill(), sigsuspend(), others */
#include <sys/ipc.h>            /* for all IPC function calls */
#include <sys/shm.h>            /* for shmget(), shmat(), shmctl() */
#include <sys/sem.h>            /* for semget(), semop(), semctl() */
#include <stdlib.h>

/*
 * Constantes Necessarias 
 */
#define SEM_KEY_PRODUTOR	0x1243
#define SEM_KEY_CONSUMIDOR	0x1244
#define SHM_KEY			0x1432
#define NO_OF_CHILDREN	8
#define MAX_SIZE_BUFFER 66

/*
 * As seguintes variaveis globais contem informacao importante. A variavel
 * g_sem_id e g_shm_id contem as identificacoes IPC para o semaforo e para
 * o segmento de memoria compartilhada que sao usados pelo programa. A variavel
 * g_shm_addr e um ponteiro inteiro que aponta para o segmento de memoria
 * compartilhada que contera o indice inteiro da matriz de caracteres que contem
 * o alfabeto que sera exibido.
*/

/* Semaforo */
int 	g_sem_id_produtor;
int 	g_sem_id_consumidor;

/* Memoria Compartilhada */
typedef struct {
	int consumidor;
	int produtor;
	char buffer[MAX_SIZE_BUFFER];
}shared_memory;

int 	g_shm_id;

/* Endereço */
shared_memory	*g_shm_addr;


/*
 * As seguintes duas estruturas contem a informacao necessaria para controlar
 * semaforos em relacao a "fecharem", se nao permitem acesso, ou 
 * "abrirem", se permitirem acesso. As estruturas sao incializadas ao inicio
 * do programa principal e usadas na rotina PrintAlphabet(). Como elas sao
 * inicializadas no programa principal, antes da criacao dos processos filhos,
 * elas podem ser usadas nesses processos sem a necessidade de nova associacao
 * ou mudancas.
*/
struct sembuf	g_sem_op1[1];	
struct sembuf	g_sem_op2[1];	

/*
 * O seguinte vetor de caracteres contem o alfabeto que constituira o string
 * que sera exibido.
*/
char g_letters_and_numbers[] = " ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 1234567890";

/*
 * Funcoes
 */
void Produtor( int );
void Consumidor( int );

/*
 * Programa Principal
 */
int main( int argc, char *argv[] )
{
      /*
       * Variaveis necessarias
       */
      pid_t rtn;
      int count;
        
      /*
       * Para armazenar os ids dos processos filhos, permitindo o posterior
       * uso do comando kill
       */
      int pid[NO_OF_CHILDREN];

	
	/* ================================================================================================== */


	/*
	 * Construindo a estrutura de controle do semaforo
	 */
	g_sem_op1[0].sem_num =  0;
	g_sem_op1[0].sem_op  = -1;	//Bloquear
	g_sem_op1[0].sem_flg =  0;

	g_sem_op2[0].sem_num =  0;
	g_sem_op2[0].sem_op  =  1;	//Desbloquear
	g_sem_op2[0].sem_flg =  0;


	/* ============================= SEMÁFOROS ============================= */

	if( ( g_sem_id_produtor = semget( SEM_KEY_PRODUTOR, 1, IPC_CREAT | 0666 ) ) == -1 ) {
		fprintf(stderr,"chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
		exit(1);
	}
	if( ( g_sem_id_consumidor = semget( SEM_KEY_CONSUMIDOR, 1, IPC_CREAT | 0666 ) ) == -1 ) {
		fprintf(stderr,"chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
		exit(1);
	}
	
	/* Destrava os semáforos */
	if( semop( g_sem_id_produtor, g_sem_op2, 1 ) == -1 ) {
		fprintf(stderr,"chamada semop() falhou, impossivel inicializar o semaforo!");
		exit(1);
	}
	if( semop( g_sem_id_consumidor, g_sem_op2, 1 ) == -1 ) {
		fprintf(stderr,"chamada semop() falhou, impossivel inicializar o semaforo!");
		exit(1);
	}

	/* =========================== MEMORIA COMPARTILHADA =========================== */
	
	if( (g_shm_id = shmget( SHM_KEY, sizeof(shared_memory), IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar o segmento de memoria compartilhada!\n");
		exit(1);
	}
	if( (g_shm_addr = (shared_memory *)shmat(g_shm_id, NULL, 0)) == (shared_memory *)-1 ) {
		fprintf(stderr,"Impossivel associar o segmento de memoria compartilhada!\n");
		exit(1);
	}

	(*g_shm_addr).produtor = 0;
	(*g_shm_addr).consumidor = 0;

	/* ================================================================================================== */


       /*
        * Criando os filhos
        */
       rtn = 1;
       for( count = 0; count < NO_OF_CHILDREN; count++ ) {
               if( rtn != 0 ) {
                       	pid[count] = rtn = fork();
               } else {
			break;
               }
       }

       /*
        * Verificando o valor retornado para determinar se o processo e 
        * pai ou filho 
        */
       if( rtn == 0 ) {
                /*
                 * Eu sou um filho
                 */
		if(count % 2 == 0){
			/* Metade dos filhos produtores */
			Produtor( count );
		}else{
			/* Metade dos filhos consumidores */
			Consumidor( count );
		}


        } else {
                usleep(50000);

                /*
                 * Matando os filhos 
                 */
		for( int child = 0; child < NO_OF_CHILDREN; child++ ){
			kill(pid[child], SIGKILL);
		}
		

                /*
                 * Removendo as memorias compartilhadas
                 */
		
		if( shmctl(g_shm_id, IPC_RMID, NULL) != 0 ) {
                        fprintf(stderr,"Impossivel remover o segmento de memoria compartilhada!\n");
                        exit(1);
                }

                /*
                 * Removendo os semaforos
                 */

		if( semctl( g_sem_id_produtor, 0, IPC_RMID, 0) != 0 ) {
                        fprintf(stderr,"Impossivel remover o conjunto de semaforos!\n");
                        exit(1);
                }

		if( semctl( g_sem_id_consumidor, 0, IPC_RMID, 0) != 0 ) {
                        fprintf(stderr,"Impossivel remover o conjunto de semaforos!\n");
                        exit(1);
                }

		fprintf(stderr, "\n");

                exit(0);
        }
}


/* ================================================================================================== */


/* Produzir os caracteres */
void Produtor( int count ){

	struct timeval tv;
	int number;

	int tmp_index_produtor;
	int i, j;
	
	char caracteres_produzidos[5];

	/*
	 * Este tempo permite que todos os filhos sejam inciados
	 */
	usleep(1000);

	/*
	 * Entrando no loop principal
	 */
	while(1) {

		/*
                 * Conseguindo o tempo corrente, os microsegundos desse tempo
             	 * sao usados como um numero pseudo-randomico. Em seguida,
            	 * calcula o numero randomico atraves de um algoritmo simples
		 */
		if( gettimeofday( &tv, NULL ) == -1 ) {
			fprintf(stderr,"Impossivel conseguir o tempo atual, terminando.\n");
			exit(1);
		}
		/* number contem numero de caracteres que serão produzidos */	
		number = ((tv.tv_usec / 47) % 5) + 1;
	

#ifdef PROTECT
	if( semop( g_sem_id_produtor, g_sem_op1, 1 ) == -1 ) {      		
		fprintf(stderr,"chamada semop() falhou, impossivel liberar o recurso!");
		exit(1);
	}
	if( semop( g_sem_id_consumidor, g_sem_op1, 1 ) == -1 ) {      		
		fprintf(stderr,"chamada semop() falhou, impossivel liberar o recurso!");
		exit(1);
	}
#endif

		tmp_index_produtor = (*g_shm_addr).produtor;

		for( i = 0; i < number; i++ ) {
			/* Verificar se pode produzir */
			if(tmp_index_produtor + i < MAX_SIZE_BUFFER){
				/* Pode produzir */
				/* Garante que não ultrapasse o limite do vetor */
				if( ! (tmp_index_produtor > sizeof(g_letters_and_numbers)) ){
					/* Coloca caracteres no buffer */
					(*g_shm_addr).buffer[tmp_index_produtor + i] = g_letters_and_numbers[tmp_index_produtor + i];
					caracteres_produzidos[i] =  g_letters_and_numbers[tmp_index_produtor + i];
					usleep(1);
				}
			}else{
				/* Nao pode produzir -> break */
				break;
			}
		}
		
		/* Atualiza indice da memoria */
		(*g_shm_addr).produtor = tmp_index_produtor + i;
		
		/* Verifica se buffer esta cheio */
		if(tmp_index_produtor + i >= MAX_SIZE_BUFFER){
			/* Reinicia indice */
			(*g_shm_addr).produtor = 0;
			(*g_shm_addr).consumidor = 0;

			/* Printa buffer */
			fprintf(stderr, "\n\nProdutor - Buffer Cheio\n");
			for( j = 0; j < MAX_SIZE_BUFFER; j++ ){
				fprintf(stderr, "%c", (*g_shm_addr).buffer[j]);
			}
		}
		
		/* Imprime caracteres produzidos por cada filho */
		fprintf(stderr, "\nCaracteres produzidos pelo filho %i: ", count);
		for( j = 0; j < number; j++ ){
			fprintf(stderr,"%c", caracteres_produzidos[j]);
		}
		
		/* Tempo de dormencia equivalente a caracteres produzidos */
		usleep(number);

#ifdef PROTECT
	if( semop( g_sem_id_produtor, g_sem_op2, 1 ) == -1 ) {      		
		fprintf(stderr,"chamada semop() falhou, impossivel liberar o recurso!");
		exit(1);
	}
	if( semop( g_sem_id_consumidor, g_sem_op2, 1 ) == -1 ) {      		
		fprintf(stderr,"chamada semop() falhou, impossivel liberar o recurso!");
		exit(1);
	}
#endif


	}
}


/* Consumir os caracteres (trocar cada caractere por '#') */
void Consumidor( int count ){
	
	struct timeval tv;
	int number;

	int tmp_index_consumidor;
	int tmp_index_produtor;
	int i, j;

	/*
	 * Este tempo permite que todos os filhos sejam inciados
	 */
	usleep(1000);

	/*
	 * Entrando no loop principal
	 */
	while(1) {

		/*
                 * Conseguindo o tempo corrente, os microsegundos desse tempo
             	 * sao usados como um numero pseudo-randomico. Em seguida,
            	 * calcula o numero randomico atraves de um algoritmo simples
		 */
		if( gettimeofday( &tv, NULL ) == -1 ) {
			fprintf(stderr,"Impossivel conseguir o tempo atual, terminando.\n");
			exit(1);
		}
		/* number contem o numero de caracteres que serão consumidos */	
		number = ((tv.tv_usec / 47) % 5) + 1;	

#ifdef PROTECT
	if( semop( g_sem_id_produtor, g_sem_op1, 1 ) == -1 ) {      		
		fprintf(stderr,"chamada semop() falhou, impossivel liberar o recurso!");
		exit(1);
	}
	if( semop( g_sem_id_consumidor, g_sem_op1, 1 ) == -1 ) {      		
		fprintf(stderr,"chamada semop() falhou, impossivel liberar o recurso!");
		exit(1);
	}
#endif
		
		tmp_index_produtor = (*g_shm_addr).produtor;
		tmp_index_consumidor = (*g_shm_addr).consumidor;

		for( i = 0; i < number; i++ ) {
			/* Verificar se pode consumir */
			if(tmp_index_produtor + i != 0){
				/* Pode consumir */
				/* Troca posição do vetor por '#' */
				if(! (tmp_index_consumidor + i > MAX_SIZE_BUFFER )) {
					(*g_shm_addr).buffer[tmp_index_consumidor + i] = '#';
					usleep(1);
				}
			}else{
				/* Nao pode consumir -> break */
				break;
			}
		}
			
		/* Atualiza indice da memoria */
		(*g_shm_addr).consumidor = tmp_index_consumidor + i;
		
		/* Verifica se buffer esta cheio */
		if(tmp_index_consumidor + i >= MAX_SIZE_BUFFER){
			/* Reinicia indice */
			(*g_shm_addr).produtor = 0;
			(*g_shm_addr).consumidor = 0;

			/* Printa buffer */
			fprintf(stderr, "\n\nConsumidor - Buffer Cheio\n");
			for( j = 0; j < MAX_SIZE_BUFFER; j++ ){
				fprintf(stderr, "%c", (*g_shm_addr).buffer[j]);
			}
			fprintf(stderr, "\n");
		}	
		
		/* Tempo de dormencia equivalente a caracteres consumidos */
		usleep(number);

#ifdef PROTECT
	if( semop( g_sem_id_produtor, g_sem_op2, 1 ) == -1 ) {      		
		fprintf(stderr,"chamada semop() falhou, impossivel liberar o recurso!");
		exit(1);
	}
	if( semop( g_sem_id_consumidor, g_sem_op2, 1 ) == -1 ) {      		
		fprintf(stderr,"chamada semop() falhou, impossivel liberar o recurso!");
		exit(1);
	}
#endif

	}
}
