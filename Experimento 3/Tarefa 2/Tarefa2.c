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
#define SEM_KEY_BUFFER		0x1243
#define SEM_KEY_PRODUTOR	0x1244
#define SEM_KEY_CONSUMIDOR	0x1245

#define SHM_KEY_BUFFER		0x1432
#define SHM_KEY_PRODUTOR	0x1433
#define SHM_KEY_CONSUMIDOR	0x1434

#define NO_OF_CHILDREN	8

#define MAX_SIZE_BUFFER 500
/*
 * As seguintes variaveis globais contem informacao importante. A variavel
 * g_sem_id e g_shm_id contem as identificacoes IPC para o semaforo e para
 * o segmento de memoria compartilhada que sao usados pelo programa. A variavel
 * g_shm_addr e um ponteiro inteiro que aponta para o segmento de memoria
 * compartilhada que contera o indice inteiro da matriz de caracteres que contem
 * o alfabeto que sera exibido.
*/

int	g_sem_id_buffer, g_sem_id_produtor_consumidor;

int	g_shm_id_buffer, g_shm_id_produtor, g_shm_id_consumidor;

int	*g_shm_addr_produtor, *g_shm_addr_consumidor;
char 	*g_shm_addr_buffer;

/*Arquivo para salvarmos o resultado*/

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
char buffer[MAX_SIZE_BUFFER];
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

	
	/*==================================================================================================*/


	/*
	 * Construindo a estrutura de controle do semaforo
	 */
	g_sem_op1[0].sem_num =  0;
	g_sem_op1[0].sem_op  = -1;	//Bloquear
	g_sem_op1[0].sem_flg =  0;

	g_sem_op2[0].sem_num =  0;
	g_sem_op2[0].sem_op  =  1;	//Desbloquear
	g_sem_op2[0].sem_flg =  0;


	/*=========================== SEMÁFOROS ===========================*/

	/*
	 * Criando o semaforo
	 */	
	if( ( g_sem_id_buffer = semget( SEM_KEY_BUFFER, 1, IPC_CREAT | 0666 ) ) == -1 ) {
		fprintf(stderr,"chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
		exit(1);
	}

	if( ( g_sem_id_produtor_consumidor = semget( SEM_KEY_CONSUMIDOR, 1, IPC_CREAT | 0666 ) ) == -1 ) {
		fprintf(stderr,"chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
		exit(1);
	}

	
	
	/* Destrava os semáforos */
	if( semop( g_sem_id_buffer, g_sem_op2, 1 ) == -1 ) {
		fprintf(stderr,"chamada semop() falhou, impossivel inicializar o semaforo!");
		exit(1);
	}
	
	if( semop( g_sem_id_produtor_consumidor, g_sem_op2, 1 ) == -1 ) {
		fprintf(stderr,"chamada semop() falhou, impossivel inicializar o semaforo!");
		exit(1);
	}


	/*=========================== MEMORIA COMPARTILHADA ===========================*/


	/*
	 * Criando o segmento de memoria compartilhada
	 */
	if( (g_shm_id_buffer = shmget( SHM_KEY_BUFFER, 5*sizeof(g_letters_and_numbers), IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar o segmento de memoria compartilhada!\n");
		exit(1);
	}

	if( (g_shm_addr_buffer = (char *)shmat(g_shm_id_buffer, NULL, 0)) == (char *)-1 ) {
		fprintf(stderr,"Impossivel associar o segmento de memoria compartilhada!\n");
		exit(1);
	}

	if( (g_shm_id_produtor = shmget( SHM_KEY_PRODUTOR, sizeof(int), IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar o segmento de memoria compartilhada!\n");
		exit(1);
	}

	if( (g_shm_addr_produtor = (int *)shmat(g_shm_id_produtor, NULL, 0)) == (int *)-1 ) {
		fprintf(stderr,"Impossivel associar o segmento de memoria compartilhada!\n");
		exit(1);
	}
	if( (g_shm_id_consumidor = shmget( SHM_KEY_CONSUMIDOR, sizeof(int), IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar o segmento de memoria compartilhada!\n");
		exit(1);
	}

	if( (g_shm_addr_consumidor = (int *)shmat(g_shm_id_consumidor, NULL, 0)) == (int *)-1 ) {
		fprintf(stderr,"Impossivel associar o segmento de memoria compartilhada!\n");
		exit(1);
	}

	*g_shm_addr_buffer = 0;
	*g_shm_addr_consumidor = 0;
	*g_shm_addr_produtor = 0;

	
	/*==================================================================================================*/


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

		if(count <= 4){
			Produtor( count );
		}
/*		
		else{
			//Consumidor();
		}
*/

        } else {
                usleep(15000);

                /*
                 * Matando os filhos 
                 */
		int child;
		for(child = 0; child < NO_OF_CHILDREN; child++){
			kill(pid[child], SIGKILL);
		}

                /*
                 * Removendo a memoria compartilhada
                 */
                if( shmctl(g_shm_id_buffer,IPC_RMID,NULL) != 0 ) {
                        fprintf(stderr,"Impossivel remover o segmento de memoria compartilhada!\n");
                        exit(1);
                }
		
		if( shmctl(g_shm_id_produtor,IPC_RMID,NULL) != 0 ) {
                        fprintf(stderr,"Impossivel remover o segmento de memoria compartilhada!\n");
                        exit(1);
                }

		if( shmctl(g_shm_id_consumidor,IPC_RMID,NULL) != 0 ) {
                        fprintf(stderr,"Impossivel remover o segmento de memoria compartilhada!\n");
                        exit(1);
                }

                /*
                 * Removendo o semaforo
                 */
                if( semctl( g_sem_id_buffer, 0, IPC_RMID, 0) != 0 ) {
                        fprintf(stderr,"Impossivel remover o conjunto de semaforos!\n");
                        exit(1);
                }

		if( semctl( g_sem_id_produtor_consumidor, 0, IPC_RMID, 0) != 0 ) {
                        fprintf(stderr,"Impossivel remover o conjunto de semaforos!\n");
                        exit(1);
                }
		fprintf(stderr, "\n");
                exit(0);
        }
}


/*==================================================================================================*/


/* Produzir os caracteres */
void Produtor( int count ){

	struct timeval tv;
	int number;

	int tmp_index_buffer;
	int tmp_index_produtor;
	int i;

	char caracteres_produzidos[5*sizeof(g_letters_and_numbers)];

	/*
	 * Este tempo permite que todos os filhos sejam inciados
	 */
	usleep(200);

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
		number = ((tv.tv_usec / 47) % 5) + 1;	
	

#ifdef PROTECT
		if( semop( g_sem_id_buffer, g_sem_op1, 1 ) == -1 ) {      		
                        fprintf(stderr,"chamada semop() falhou, impossivel liberar o recurso!");
                       	exit(1);
               	}

		if( semop( g_sem_id_produtor_consumidor, g_sem_op1, 1 ) == -1 ) {      		
                        fprintf(stderr,"chamada semop() falhou, impossivel liberar o recurso!");
                       	exit(1);
               	}
#endif
		
		/*
		 * Lendo o indice do segmento de memoria compartilhada
		 */
		tmp_index_buffer = *g_shm_addr_buffer;
		tmp_index_produtor = *g_shm_addr_produtor;
		
	       /*
            	* Repita o numero especificado de vezes, esteja certo de nao
            	* ultrapassar os limites do vetor, o comando if garante isso
		*/
		for( i = 0; i < number; i++ ) {
			if( ! (tmp_index_produtor + i > sizeof(g_letters_and_numbers)) ) {
				//Coloca no buffer e atualiza índice
				buffer[tmp_index_buffer + i] = g_letters_and_numbers[tmp_index_produtor + i];
				caracteres_produzidos[i] = g_letters_and_numbers[tmp_index_produtor + i];
				*g_shm_addr_buffer = tmp_index_buffer + i;
				fprintf(stderr,"%c", buffer[tmp_index_buffer + i]);
				usleep(1);
			}
		}
		
		
		/*
		 * Atualizando o indice na memoria compartilhada
		 */
		*g_shm_addr_produtor = tmp_index_produtor + i;

		/*
         	 * Se o indice e maior que o tamanho do alfabeto, exibe um
         	 * caractere return para iniciar a linha seguinte e coloca
         	 * zero no indice
		 */
		if( tmp_index_buffer + i > sizeof(buffer) ) {
			/* Printar conteúdo do buffer se estiver cheio */
			for( i = 0; i < number; i++ ) {
				if( ! (tmp_index_buffer + i > sizeof(buffer)) ) {	
					fprintf(stderr, "\nBuffer cheio...\n");
					fprintf(stderr,"%c", buffer[tmp_index_buffer + i]);
				}
			}
			*g_shm_addr_buffer = 0;
		}
	
		if( tmp_index_produtor + i > sizeof(g_letters_and_numbers) ) {
			fprintf(stderr,"\n");
			*g_shm_addr_produtor = 0;
		}
		
		/* Printa o conteudo que cada filho produziu 
			fprintf(stderr, "\nCaracteres produzidos pelo filho %i\n", count);
			int j;
			for(j = 0; j < sizeof(caracteres_produzidos); j++){
				fprintf(stderr,"%c", caracteres_produzidos[j]);
			}
		*/


#ifdef PROTECT
		if( semop( g_sem_id_buffer, g_sem_op2, 1 ) == -1 ) {      		
                        fprintf(stderr,"chamada semop() falhou, impossivel liberar o recurso!");
                       	exit(1);
               	}

		if( semop( g_sem_id_produtor_consumidor, g_sem_op2, 1 ) == -1 ) {      		
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

	int tmp_index_buffer;
	int tmp_index_consumidor;
	int i;

	/*
	 * Este tempo permite que todos os filhos sejam inciados
	 */
	usleep(200);

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
		number = ((tv.tv_usec / 47) % 5) + 1;	

#ifdef PROTECT
		if( semop( g_sem_id_buffer, g_sem_op1, 1 ) == -1 ) {      		
                        fprintf(stderr,"chamada semop() falhou, impossivel liberar o recurso!");
                       	exit(1);
               	}

		if( semop( g_sem_id_produtor_consumidor, g_sem_op1, 1 ) == -1 ) {      		
                        fprintf(stderr,"chamada semop() falhou, impossivel liberar o recurso!");
                       	exit(1);
               	}
#endif
		
		tmp_index_buffer = *g_shm_addr_buffer;
		tmp_index_consumidor = *g_shm_addr_consumidor;


#ifdef PROTECT
		if( semop( g_sem_id_buffer, g_sem_op2, 1 ) == -1 ) {      		
                        fprintf(stderr,"chamada semop() falhou, impossivel liberar o recurso!");
                       	exit(1);
               	}

		if( semop( g_sem_id_produtor_consumidor, g_sem_op2, 1 ) == -1 ) {      		
                        fprintf(stderr,"chamada semop() falhou, impossivel liberar o recurso!");
                       	exit(1);
               	}
#endif

	}
}
