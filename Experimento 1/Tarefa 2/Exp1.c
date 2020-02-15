/*******************************************************************************
* Este programa esta baseado no segundo experimento do curso sobre tempo real 
* do Laboratorio Embry-Riddle
* 
* Seguem os comentarios originais:
*
* Experiment #2: Multi-Tasking, Measuring Drift
*
*    Programmer: Eric Sorton
*          Date: 1/27/97
*           For: MSE599, Special Topics Class
*
*       Purpose: When a basic sleep call is used to determine the precise time
*                when an action will occur the problem of drift occurs. 
*                The measurement of time is imprecise.  Similarly, the time in 
*                which the sleep call returns is imprecise.  Over time, this 
*                will cause the ocurrence of time to drift.  Just as if a clock 
*                loses 1 second every day, over one day, it is significant, but 
*                over a year, it loses 365 seconds, which is over 6 minutes.  
*                This is an example of drift.
*
*       Proposito: Quando uma chamada básica sleep e usada para determinar o
*                instante exato em que alguma acao vai ocorrer, ocorre o problema
*                do desvio. A medicao de tempo e imprecisa. Similarmente, o tempo
*                que demora o retorno da chamada sleep tambem e impreciso. Ao
*                longo do tempo, isto ocasionara um desvio de tempo. Algo como se
*                um relogio perdesse um segundo a cada dia. Ao longo de um dia, 
*                essa diferenca e insignificante, mas, ao longo de um ano, sao 
*                perdidos 365 segundos, o que e superior a 6 minutos. Este e um
*                exemplo de desvio.
*
*******************************************************************************/

/*
 * Includes Necessarios, verifique se as bibliotecas no diretorio sys/ estao
 * lah. Caso nao estejam, verifique onde estao e altere o include
 */

#include <sys/time.h>		/* for gettimeofday() */
#include <unistd.h>		/* for gettimeofday() and fork() */
#include <stdio.h>		/* for printf() */
#include <sys/types.h>		/* for wait() */
#include <sys/wait.h>		/* for wait() */
#include <stdlib.h>
/*
 * Pergunta 1: o que o compilador gcc faz com o arquivo .h, cujo nome aparece após o include?
	Resposta: Resposta: Os arquivos .h são bibliotecas que, em sua maioria, estão em uma pasta de includes. Na fase de pré-processamento o compilador adiciona os arquivos .h no programa.
 */

/*
 * Pergunta 2: apresentar (parcialmente) e explicar o que há em <stdio.h>
	Resposta: A bilbioteca <stdio.h> possui diversas funções reponsáveis pela entrada e saída de dados do programas, como printf() (para a impressão de informações e dados na tela) e scanf() (para a entrada de dados no programa). A biblioteca também possui várias definições para variáveis e constantes.
 */

/*
 * Pergunta 3: qual eh a funcao da diretiva include (linha que começa com #), com relação ao compilador?
	Resposta: Ao usarmos a diretiva include, o compilador adiciona os arquivos, bibliotecas que aparecem após a diretiva.
 */

/*
 * NO_OF_ITERATIONS e o numero de vezes que vai se repetir o loop existente
 * em cada futuro processo filho. 
 */

#define NO_OF_ITERATIONS 1000

/*
 * NO_OF_CHILDREN eh o numero de filhos a serem criados, cada qual responsavel
 * pela medida do desvio. 
 */

#define NO_OF_CHILDREN	5

/*
 * SLEEP_TIME corresponde a quantidade de tempo para ficar bloqueado.
 */

#define SLEEP_TIME 1000

/*
 * MICRO_PER_SECOND define o numero de microsegundos em um segundo
 */

#define MICRO_PER_SECOND 1000000

/*
 * Programa Principal. Contem tanto o codigo para o processo pai como
 * o codigo dos futuros filhos
 */

int main( int argc, char *argv[] )
{
/*
 * Pergunta 4: o que são e para que servem argc e argv? Não esqueca de
 * considerar o * antes de argv.
	Resposta: O argc indica o número de argumentos que foram passados ao chamar o programa. Já o argv é um vetor que contém esses argumentos. Cada string do argv é um dos parâmetros da linha de comando.
 */
	/*
	 * start_time e stop_time conterao o valor de tempo antes e depois
	 * que as trocas de contexto comecem
         */

	struct timeval start_time;
	struct timeval stop_time;

      /*
       * Outras variaveis importantes
       */

      float drift;
      int count;
      int child_no;

	//Variáveis para o auxílio na chamada execvp
	int sleep_time = SLEEP_TIME;
	int no_iterations = NO_OF_ITERATIONS;
	int micro_second = MICRO_PER_SECOND;
	
	//Variáveis para itoa()
	char buffer_count[20];
	char buffer_sleep[20];
	char buffer_iterations[20];
	char buffer_microSecond[20]; 
	

	/*
	 * Criacao dos processos filhos
	 */

	pid_t rtn = 1;
	for( count = 0; count < NO_OF_CHILDREN; count++ ) {
		if( rtn != 0 ) {
			rtn = fork();
		} else {
			break;
		}
	}


	/*
	 * Verifica-se rtn para determinar se o processo eh pai ou filho
	 */

	if( rtn == 0 ) {

	/*
         * Portanto, sou filho. Faco coisas de filho. 
         */	
		sprintf(buffer_sleep, "%d", sleep_time);
		sprintf(buffer_iterations, "%d", no_iterations);
		sprintf(buffer_microSecond, "%d", micro_second);
		sprintf(buffer_count, "%d", count);

		char *args[] = {"./filho", buffer_sleep, buffer_iterations, buffer_microSecond, buffer_count, NULL};
		execvp(args[0], args);

	} else {
		/*
		 * Sou pai, aguardo o termino dos filhos
		 */
		for( count = 0; count < NO_OF_CHILDREN; count++ ) {
			wait(NULL);
		}
	}

	exit(0);
}
