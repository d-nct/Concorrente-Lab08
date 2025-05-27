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

/* debug! debug!! debug!!! */
/* compilador com a flag -DDEBUG */

#ifdef DEBUG
#define LOG(fmt, ...)             printf("[l:%d] " fmt, __LINE__, ##__VA_ARGS__)
#define LOGT(fmt, ...)             printf("[l:%d] Thread %d: " fmt, __LINE__, ##__VA_ARGS__)
#define ENDL                      printf("\n")
#else
#define LOG(fmt, ...)    
#define LOGT(fmt, ...)    
#define ENDL
#endif

// Variaveis globais
sem_t produtor, consumidor; // Semaforos para sincronizar a ordem de execucao das threads
long long int *canal;                // Canal de inteiros de tamanho M
int pos_in = 0, pos_out = 0;         // Posição mod M do canal (entrada e saída)
pthread_mutex_t mutex_in, mutex_out, mutex_cons; // Variavel de lock para exclusao mutua
int num_prod = 1, NTHREADS;
int terminei = 0;                    // Booleano se a tarefa do programa é finda
int consumidores_vivos = 0;

// Variáveis recebidas do usuário
uint num_cons; // Número de threads consumidoras
uint N;        // Verificaremos os primos de 2 até N
uint M;        // Tamanho do buffer de inteiros (pequeno)

// Struct para abrigar o retorno das threads consumidoras
typedef struct _ret {
  long int id;
  long int qtPrimos;
  long int qtAvaliados;
} ret_t;

// Função executada pelas threads
int ehPrimo(long long int n) {
	if (n<=1) return 0;
	if (n==2) return 1;
	if (n%2==0) return 0;
	for (int i=3; i<sqrt(n)+1; i+=2)
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
/*int insere(uint quantidade) {*/
/*	static long long int ja_produzidos = 0;*/
/**/
/*	for (int i = pos; i <= pos + quantidade; i++) {*/
/*		if (pos < M) {*/
/*			canal[i] = ++ja_produzidos;*/
/*		} else {*/
/*			printf("ERRO: estouro do buffer do canal durante a inserção\n");*/
/*			printf("      na execução de  insere(%u), com M = %u\n", quantidade, M);*/
/*			return 1;*/
/*		}*/
/*	}*/
/**/
/*	return 0;*/
/*}*/

/**
 * Função que pega o próximo inteiro disponível de forma atômica.
 * Atualiza internamente o contador de posição do canal.
 *
 * @return valor a ser testado.
*/
/*long long int get_proximo_n(void) {*/
/*  long long int valor;*/
/**/
/*  pthread_mutex_lock(&mutex_pos);*/
/*  valor = canal[pos];*/
/**/
/*  // Atualiza o valor de pos */
/*  if (pos != M) pos++; */
/*  else pos = 0;*/
/*  pthread_mutex_unlock(&mutex_pos);*/
/**/
/*  return valor;*/
/*}*/

void *Produtor (void *threadid) {
  long long int proximo = 1;

  // Rotina de produção no canal
  do {
    sem_wait(&produtor); // Espera sua vez
    if (consumidores_vivos == 0 && terminei) break;
  
    pthread_mutex_lock(&mutex_in);
    
    // Preenche o canal inteiro. Obs: não tem problema se passar de N!
    do {
    canal[pos_in] = proximo++;
    pos_in = (pos_in + 1) % M;
    } while (pos_in % M != 0);

    LOGT("Produzi um batch de %d (até %lld).\n", 0, M, proximo - 1);

    pthread_mutex_unlock(&mutex_in);

    // Sinaliza que o canal foi preenchido M vezes
    for (int i = 0; i < M; i++) {
      sem_post(&consumidor); 
    }
    
  } while (1);

  LOGT("Fim.\n", 0);
  pthread_exit(NULL);
}

void *Consumidor (void *threadid) {
  int id = *(int*) threadid;
  long long int atual;
  ret_t *ret = (ret_t*) malloc(sizeof(ret_t));
  if (ret == NULL) {printf("--ERRO: malloc \n"); exit(-2);}

  pthread_mutex_lock(&mutex_cons);
  consumidores_vivos++;
  pthread_mutex_unlock(&mutex_cons);
			
  // Inicializa o Retorno
  ret->id = id;
  ret->qtPrimos = 0;
  ret->qtAvaliados = 0;

  // Rotina de consumismo
  LOGT("Iniciando rotina de consumismo.\n", id);
  while (!terminei) {
    sem_wait(&consumidor); // Espera a sua vez
    
    // Pega o inteiro a ser trabalhado de forma atômica
    pthread_mutex_lock(&mutex_out);
    atual = canal[pos_out];
    pos_out = (pos_out + 1) % M;
    
    // Verifica se o canal precisa ser abastecido
    if (pos_out % M == 0) {
      LOGT("Enviando sinal para reabastecer o canal.\n", id);
      sem_post(&produtor);
    }

    pthread_mutex_unlock(&mutex_out);

    // Verifica se terminou
    if (atual <= N) { // Não terminou
      // Consome o cara atual
      LOGT("Verificando n = %lld.\n", id, atual);
      if (ehPrimo(atual)) ret->qtPrimos++;
      ret->qtAvaliados++;
      
      // Verifica se foi o último cara
      if (atual == N) { // Último cara
        LOGT("Encontrei n = %lld == N! Setando terminei = 1.\n", id, atual);

        terminei = 1;

        break;
      }
    } else {
        LOGT("Encontrei n = %lld > N! Ignorando.\n", id, atual);
      break;
    }
  }

  pthread_mutex_lock(&mutex_cons);
  consumidores_vivos--;
  pthread_mutex_unlock(&mutex_cons);

  // Manda o sinal derradeiro para o produtor se finalizar também
  if (consumidores_vivos == 0 && terminei == 1) {
    LOGT("Enviando sinal terminal para produtor.\n", id);
    sem_post(&produtor);
  }

  LOGT("Fim.\n", id);
  pthread_exit(ret);
}



//funcao principal
int main(int argc, char *argv[]) {
	pthread_t* tid;           // Vetor de TIDs
  int *id;                  // Vetor de IDs
	int t;                    // Índice geral
  ret_t **rets;             // Vetor de retornos
  int id_vencedora = 0;     // Thread vencedora
  int qt_vencedora = 0;
  int total_primos = 0;     // Quantidade total de primos encontrados 
  int total_avaliados = 0;  // Quantidade total de numeros testados 

  // Recebe a entrada
  if (argc != 4) {
    printf("Digite: %s <numero de consumidores> <N> <M>\n", argv[0]);
    return 1;
  }
  num_cons = atoi(argv[1]);
  N =        atoi(argv[2]);
  M =        atoi(argv[3]);

  // Inicializa as variáveis auxiliares
  canal = (long long int*) malloc(M * sizeof(long long int));
  if (canal == NULL) {printf("--ERRO: malloc \n"); exit(-2);}

	sem_init(&produtor, 0, 1);
	sem_init(&consumidor, 0, 0);

  pthread_mutex_init(&mutex_in, NULL);
  pthread_mutex_init(&mutex_out, NULL);
  pthread_mutex_init(&mutex_cons, NULL);

  NTHREADS = num_prod + num_cons;
  tid = (pthread_t*) malloc(NTHREADS * sizeof(pthread_t));
  if (tid == NULL) {printf("--ERRO: malloc \n"); exit(-2);}
  id = (int*) malloc(NTHREADS * sizeof(int));
  if (id == NULL) {printf("--ERRO: malloc \n"); exit(-2);}
  rets = (ret_t**) malloc(NTHREADS * sizeof(ret_t*));
  if (rets == NULL) {printf("--ERRO: malloc \n"); exit(-2);}

	// Cria a thread produtora
  id[0] = 1;
  if (pthread_create(&tid[0], NULL, Produtor, (void *)&id[0])) { printf("--ERRO: pthread_create()\n"); exit(-1); }

  // Cria as Threads consumidoras
	for (t = 1; t < NTHREADS; t++) {
		id[t] = t;
		if (pthread_create(&tid[t], NULL, Consumidor, (void *)&id[t])) { printf("--ERRO: pthread_create()\n"); exit(-1); }
	}  

	// Espera todas as threads terminarem
	for (t = 0; t < NTHREADS; t++) {
    if (pthread_join(tid[t], (void**) &rets[t])) {
			printf("--ERRO: pthread_join() \n"); exit(-1); 
		} 
	} 

  ENDL;
  printf("RESUMO\n");
  printf("------\n");
  for (t = 1; t < NTHREADS; t++) { // Apenas as consumidoras
    total_avaliados += rets[t]->qtAvaliados; // Contabiliza o subtotal
    total_primos += rets[t]->qtPrimos; // Contabiliza o subtotal

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

  // Imprime o total de primos descobertos
  printf("\nTotal Avaliados: %d\n", total_avaliados);
  printf("Total de Primos: %d\n", total_primos);

  // Imprime a vencedora
  printf("\nVencedora: Thread #%d\n", id_vencedora);

  // Arruma a Casa
  free(canal);
  free(tid);
  free(id);
  free(rets[0]);
  free(rets);
  pthread_mutex_destroy(&mutex_in);
  pthread_mutex_destroy(&mutex_out);
  pthread_mutex_destroy(&mutex_cons);
  sem_destroy(&produtor);
  sem_destroy(&consumidor);

  return 0;
}

