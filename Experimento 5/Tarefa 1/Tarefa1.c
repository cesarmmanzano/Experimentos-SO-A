/* ========================= BIBLIOTECAS ========================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>           /* Pata time() */
#include <errno.h>          /* errno and error codes */
#include <unistd.h>         /* Para fork() */
#include <signal.h>         /* Para kill(), sigsuspend(), outros */
#include <wait.h>           /* Para wait() */
#include <sys/time.h>	    /* Para gettimeofday() */
#include <sys/types.h>	    /* Para wait(), msgget(), msgctl() */
#include <sys/ipc.h>	    /* Para msgget(), msgctl() */
#include <sys/msg.h>	    /* Para msgget(), msgctl() */
//#include <sys/sem.h>        /* Para semget(), semop(), semctl() */
#include <sys/shm.h>        /* Para shmget(), shmat(), shmctl() */

/* ========================= CONSTANTES ========================= */

#define BARBERS 2
#define CUSTOMERS 1
#define CHAIRS 7

#define MESSAGE_MTYPE_B 1
#define MESSAGE_MTYPE_C 2
#define MESSAGE_QUEUE_ID 3102

#define SHM_KEY	0x1432

#define PERMISSION 0666 

#define MAXSTRINGSIZE 5120

#define MICRO_PER_SECOND 1000000

/* ========================= VARIÁVEIS E SIMILARES ========================= */

/* Fila de mensagens */
/* Barbeiro envia para cliente */
typedef struct {
	unsigned int barber_no;
    char msgBarber[MAXSTRINGSIZE];
    int arraySize;
} data_t_barber; 

/* Cliente envia para barbeiro */
typedef struct {
	unsigned int customer_no;
    char msgCustomer[MAXSTRINGSIZE];
    int arraySize;
} data_t_customer; 

typedef struct {
	long mtype;
	char mtext[5555];
} msgbuf_t;

/* Memoria Compartilhada */
int g_shm_id;
int *g_shm_addr;
 
/* ========================= FUNÇÕES ========================= */

void barber(int, int);
void customer(int, int);
void cut_hair(int[], char[], int);
void apreciate_hair(int, int, float, int[], int[], int);

void arrayToString(int[], char[], int); 
void bbsort(int[], int);
void clearString(char[], int);

/* ========================= MAIN ========================= */

int main() {
    
    pid_t rtn;

    /* Vetores com os pids dos barbeiros e clientes */
    pid_t barber_pid[BARBERS];
    //pid_t customer_pid[CUSTOMERS];

    int queue_id;
    key_t key_msg = MESSAGE_QUEUE_ID;

    int i;

    /* Cria fila de mensagens */
    if( (queue_id = msgget(key_msg, IPC_CREAT | PERMISSION)) == -1 ) {
		fprintf(stderr,"Impossivel criar a fila de mensagens!\n");
		exit(1);
	}

    /* Cria memoria compartilhada */
    if( (g_shm_id = shmget(SHM_KEY, sizeof(int), IPC_CREAT | PERMISSION)) == -1 ) {
		fprintf(stderr,"Impossivel criar o segmento de memoria compartilhada!\n");
		exit(1);
	}
	if( (g_shm_addr = (int *)shmat(g_shm_id, NULL, 0)) == (int *)-1 ) {
		fprintf(stderr,"Impossivel associar o segmento de memoria compartilhada!\n");
		exit(1);
	}
    *g_shm_addr = 0;

    /* Declara os processos filhos (barbeiros e clientes) */
    rtn = 1;
    for(i = 0; i < BARBERS; i++){
        if( rtn != 0 ) {

		    barber_pid[i] = rtn = fork();
            
            if(rtn == 0){
                barber(queue_id, i + 1);
            }

	    } else {
		    break;
	    }
    }

    rtn = 1;
    for( i = 0; i < CUSTOMERS; i++){
        if( rtn != 0 ) {

		    rtn = fork();

            if(rtn == 0){
                customer(queue_id, i + 1);
            }

	    } else {
		    break;
	    }
    }

    /* Espera o término dos filhos */
    if(rtn != 0){

        for(i = 0; i < CUSTOMERS; i++){
                wait(NULL);
        }
        
        /* Matando os processos */
        for(i = 0; i < BARBERS; i++){
            wait(NULL);

        }

    }else{
        exit(0);
    }


    /* Remove fila de mensagens */
    if( msgctl(queue_id, IPC_RMID, NULL) == -1 ) {
		fprintf(stderr,"Impossivel remover a fila de mensagens!\n");
		exit(1);
	}

    /* Remove memoria compartilhada */
    if( shmctl(g_shm_id, IPC_RMID, NULL) != 0 ) {
        fprintf(stderr,"Impossivel remover o segmento de memoria compartilhada!\n");
        exit(1);
    }
    
    return 0;
}

/* ========================= FUNÇÕES REFERENTES AO BARBEIRO E CLIENTES ========================= */

void barber(int queue_id, int barber){
    
    char stringReceived[MAXSTRINGSIZE]; /* Recebe string do cliente */

    msgbuf_t message_send;    /* Mensagem que envia */
    msgbuf_t message_receive;  /* Mensagem que recebe */
    data_t_barber *data_ptr_send = (data_t_barber *)(message_send.mtext);
    data_t_customer *data_ptr_receive = (data_t_customer *)(message_receive.mtext);

    /* Recebe mensagens do cliente */
    if( msgrcv(queue_id, (struct msgbuf_t *)&message_receive, sizeof(data_t_barber), MESSAGE_MTYPE_B, 0) == -1 ) {
		fprintf(stderr, "Impossivel receber mensagem!\n");
		exit(1);
	}

    /* Pega informações da mensagem */
    strcpy(stringReceived, data_ptr_receive->msgCustomer);
    int array[data_ptr_receive->arraySize];
    cut_hair(array, stringReceived, data_ptr_receive->arraySize);
    bbsort(array, data_ptr_receive->arraySize);
    arrayToString(array, stringReceived, data_ptr_receive->arraySize);

    /* Apronta dados para enviar mensagem ao cliente */
    message_send.mtype = MESSAGE_MTYPE_C;
	data_ptr_send->barber_no = barber;
	strcpy(data_ptr_send->msgBarber, stringReceived);
	data_ptr_send->arraySize = data_ptr_receive->arraySize;

    if( msgsnd(queue_id, (struct msgbuf_t *)&message_send, sizeof(data_t_barber), 0) == -1 ) {
		fprintf(stderr, "Impossivel enviar mensagem!\n");
	    exit(1);
	}

    /* RC */
    *g_shm_addr = *g_shm_addr - 1;

    exit(0);
    
}

void customer(int queue_id, int customer){

    srand (time(NULL));
    int sizeString = (rand() % 1021) + 2; /* Tamanho da string que será passada ao barbeiro */
    int array[sizeString]; /* Armazena valores gerados */
    char stringtoBarber[sizeString*5]; /* String que será passada ao barbeiro */
    char stringOrdered[sizeString*5]; /* String que conterá a string organizada */
    int arrayOrdered[sizeString]; /* Vetor de inteiros ordenado*/

    msgbuf_t message_send; /* Mensagem que envia */
    msgbuf_t message_receive; /* Mensagem que recebe */
    data_t_barber *data_ptr_receive = (data_t_barber *)(message_send.mtext); /* Ponteiro para os dados */
    data_t_customer *data_ptr_send = (data_t_customer *)(message_receive.mtext); /* Ponteiro para os dados */

    struct timeval start_time; /* Instante em que entra e senta na sala */
    struct timeval stop_time; /* Instante em que inicia o corte */

    /* Gera vetor aleatorio e converte para string */
    srand (time(NULL));
    for(int i = 0; i < sizeString; i++){
        array[i] = (rand() % 1021) + 2;  
    } 
    arrayToString(array, stringtoBarber, sizeString);

    /* Pega tempo atual */
    gettimeofday(&start_time, NULL);

    /* Início da RC */
    if(*g_shm_addr >= 7){
        
        /* Cliente não atendido */
        printf("Cliente #%d nao foi atendido", customer);
        exit(0);

    }else{
        
        /*Cliente atendido*/
        *g_shm_addr = *g_shm_addr + 1;

        /* Apronta os dados para enviar mensagem */
        message_send.mtype = MESSAGE_MTYPE_B;
	    data_ptr_send->customer_no = customer;
	    strcpy(data_ptr_send->msgCustomer, stringtoBarber);
	    data_ptr_send->arraySize = sizeString;

        if( msgsnd(queue_id, (struct msgbuf_t *)&message_send, sizeof(data_t_customer), 0) == -1 ) {
		    fprintf(stderr, "Impossivel enviar mensagem!\n");
		    exit(1);
	    }

    }

    /* Cliente recebe mensagem do barbeiro -> acabou de cortar o cabelo */
    if( msgrcv(queue_id, (struct msgbuf_t *)&message_receive, sizeof(data_t_barber), MESSAGE_MTYPE_C, 0) == -1 ) {
		fprintf(stderr, "Impossivel receber mensagem!\n");
		exit(1);
	}
    gettimeofday(&stop_time, NULL);

    strcpy(stringOrdered, data_ptr_receive->msgBarber);
    cut_hair(arrayOrdered, stringOrdered, data_ptr_receive->arraySize);

    float time;
    time = (float)(stop_time.tv_sec  - start_time.tv_sec);
	time += (stop_time.tv_usec - start_time.tv_usec)/(float)MICRO_PER_SECOND; 

    apreciate_hair(data_ptr_receive->barber_no, customer, time, array, arrayOrdered, sizeString);

    exit(0);

}

/*Converte string para vetor */
void cut_hair(int array[], char string[], int size){

    int i, k;
    unsigned int j = 0;

    char temp[5];

    for(i = 0; i < size; i++){
		k = 0;
		clearString(temp, 5);
		for(j = j; j < strlen(string); j++){
			if(string[j] != ' '){
				temp[k] = string[j];
				k++;
			}else{
				array[i] = atoi(temp);
				j++;
				break;
			}

		}

	}

}

/* Imprime informações */
void apreciate_hair(int barber, int customer, float time, int array[], int arrayOrdered[], int size){
    printf("\n");
    printf("Cliente #%d foi atendido pelo barbeiro #%d", customer, barber);
    printf("\nTempo aproximado para o cliente ser atendido: %.2f", time);
    printf("\nString a ser ordenada:\n");
    for(int i = 0; i < size; i++){
        printf("%d ",array[i]);
    }
    printf("\nString Ordenada:\n");
    for(int i = 0; i < size; i++){
        printf("%d ",arrayOrdered[i]);
    }
    printf("\n");
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