/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Aluno: Daniel Nocito -- 122076971 */
/* Codigo: padrão produtor/consumidor usando semáforos */

#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <math.h>
#define NUM_PROD 1      // Número de produtores. Deve ser 1.
#define NUM_CONS 3      // Número de consumidores arbitrário.

#define NTHREADS NUM_PROD + NUM_CONS

// Variaveis globais
sem_t produtor, consumidor; //semaforos para sincronizar a ordem de execucao das threads
long long int *canal; // Canal de inteiros de tamanho M
int pos; // Posição mod M do canal
pthread_mutex_t mutex_pos; //variavel de lock para exclusao mutua
pthread_mutex_t mutex_qtPrimos; //variavel de lock para exclusao mutua

// Variáveis recebidas do usuário
uint num_cons; // Número de threads consumidoras
uint N; // Verificaremos os primos de 2 até N
uint M; // Tamanho do buffer de inteiros (pequeno)

typedef struct _ret {
  long int id;
  long int qtPrimos;
  long int qtAvaliados;
} ret_t;

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
	
	for (int i = pos; i <= pos + quantidade; i++) {
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

/**
 * Função que pega o próximo inteiro disponível.
*/
int get_proximo_n(void) {
  long long int valor;

  pthread_mutex_lock(&mutex_pos);
  valor = canal[pos];

  /* Atualiza o valor de pos */
  if (pos != M) pos++; 
  else pos = 0;
  pthread_mutex_unlock(&mutex_pos);
  
  return valor;
}

void *Produtor (void *threadid) {
}

void *Consumidor (void *threadid) {
}



//funcao principal
int main(int argc, char *argv[]) {
	pthread_t tid[NTHREADS];  // TIDs
	int t, id[NTHREADS];
  ret_t **rets;             // Vetor de retornos
  int id_vencedora = 0;
  int qt_vencedora = 0;

	//inicia os semaforos
	sem_init(&produtor, 0, 1);
	sem_init(&consumidor, 0, 1);

	// Cria a thread produtora
  id[0] = 1;
  if (pthread_create(&tid[0], NULL, Produtor, (void *)&id[0])) { printf("--ERRO: pthread_create()\n"); exit(-1); }

  // Cria as Threads consumidoras
	for (t=1; t<NTHREADS; t++) {
		id[t] = t;
		if (pthread_create(&tid[t], NULL, Consumidor, (void *)&id[t])) { printf("--ERRO: pthread_create()\n"); exit(-1); }
	}  

	// Espera todas as threads terminarem
	for (t=0; t<NTHREADS; t++) {
    if (pthread_join(tid[t], (void**) &rets[t])) {
			printf("--ERRO: pthread_join() \n"); exit(-1); 
		} 
	} 

  printf("RESUMO\n");
  printf("------\n");
  for (t = 1; t < NTHREADS; t++) { // Apenas as consumidoras
    // Verifica a Vencedora
    if (rets[t]->qtPrimos > qt_vencedora) {
      qt_vencedora = rets[t]->qtPrimos;
      id_vencedora = rets[t]->id;
    }
    printf("Thread #%ld\n", rets[t]->id);
    printf("  Avaliados:          %ld\n", rets[t]->qtAvaliados);
    printf("  Primos descobertos: %ld\n", rets[t]->qtPrimos);
    free(rets[t]);
  }

  // Imprime a vencedora
  printf("\nThread Vencedora (+primos): %d\n", id_vencedora);

  // Arruma a Casa
  pthread_mutex_destroy(&mutex_pos);
  pthread_mutex_destroy(&mutex_qtPrimos);
}

