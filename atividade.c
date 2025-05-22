/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Aluno: Daniel Nocito -- 122076971 */
/* Codigo: padrão produtor/consumidor usando semáforos */

#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#define NUM_PROD 1      // Número de produtores. Deve ser 1.
#define NUM_CONS 3      // Número de consumidores arbitrário.

#define NTHREADS NUM_PROD + NUM_CONS

// Variaveis globais
sem_t produtor, consumidor; //semaforos para sincronizar a ordem de execucao das threads
long long int *canal; // Canal de inteiros de tamanho M
int pos; // Posição mod M do canal

// Variáveis recebidas do usuário
uint num_cons; // Número de threads consumidoras
uint N; // Verificaremos os primos de 2 até N
uint M; // Tamanho do buffer de inteiros (pequeno)

// Função executada pelas threads
int ehPrimo(long long int n) {
	int i;
	if (n<=1) return 0;
	if (n==2) return 1;
	if (n%2==0) return 0;
	for (i=3; i<sqrt(n)+1; i+=2)
		if(n%i==0) return 0;
	return 1;
}

/**
 * Insere no canal os próximos inteiros a serem verificados em ordem
 * 
 * @param: quantidade de inteiros a serem adicionados no buffer. Deve ser menor ou igual a M
 *
 * @return 0, se ok na inserção
 *         1, se estouro do buffer do canal
*/
int insere(uint quantidade) {
	static long long int ja_produzidos = 0;
	
	for (int i = pos; i++; i <= pos + quantidade) {
		if (pos < M) {
			canal[i] = ++ja_produzidos;
		} else {
			printf("ERRO: estouro do buffer do canal durante a inserção\n");
			printf("      na execução de  insere(%u), com M = %u\n", quantidade, M);
			return 1;
		}
	}

	return 0;
}


void *Produtor (void *threadid) {
}

void *Consumidor (void *threadid) {
}



//funcao principal
int main(int argc, char *argv[]) {
	pthread_t tid[L+E];
	int t, id[L+E];

	//inicia os semaforos
	sem_init(&leitura, 0, 1);
	sem_init(&escrita, 0, 1);

	//cria as threads
	for (t=0; t<E; t++) {
		id[t]=t;
		if (pthread_create(&tid[t], NULL, Escritor, (void *)&id[t])) { printf("--ERRO: pthread_create()\n"); exit(-1); }
	}
	for (t=L; t<NTHREADS; t++) {
		id[t]=t-L;
		if (pthread_create(&tid[t], NULL, Leitor, (void *)&id[t])) { printf("--ERRO: pthread_create()\n"); exit(-1); }
	}  
	//--espera todas as threads terminarem
	for (t=0; t<NTHREADS; t++) {
		if (pthread_join(tid[t], NULL)) {
			printf("--ERRO: pthread_join() \n"); exit(-1); 
		} 
	} 
	pthread_exit(NULL);
}

