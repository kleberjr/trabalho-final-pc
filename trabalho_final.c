// Aluno: Kleber Rodrigues da Costa Júnior
// Matrícula: 200053680
// Matéria: Programação Concorrente

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>

#define NUM_REG_NINJAS 5			// Número de ninjas normais
#define NUM_CLONES 3				// Número de clones do Naruto
#define CHAKRA_TO_HEAL_NINJA 25		// Quantidade de chakra necessário para curar um ninja	
#define CHAKRA_TO_VITALIZE_CLONE 15	// Quantidade de chakra necessário para revitalizar um clone
#define MAX_CHAKRA_LEVEL 200		// Nível máximo de chakra que a Sakura pode alcançar
#define POWER_TO_HELP_LEE 40		// Nível de poder necessário para ajuda o Rock Lee

int sakura_chakra = MAX_CHAKRA_LEVEL;		// Nível de chakra da Sakura
int regular_ninjas_injured = 0; // Quantidade de ninjas normais querendo cura

pthread_t sakura;							// Declara a thread da Sakura
pthread_t rock_lee;							// Declara a thread do Rock Lee
pthread_t regular_ninjas[NUM_REG_NINJAS]; 	// Declara o vetor de threads dos ninjas normais
pthread_t naruto_clones[NUM_CLONES];		// Declara o vetor de threads dos clones do naruto

pthread_cond_t sakura_cond = PTHREAD_COND_INITIALIZER;			// Inicializa a condição de parada da Sakura
pthread_cond_t regular_ninja_cond = PTHREAD_COND_INITIALIZER;	// Inicializa a condição de parada dos ninjas normais
pthread_cond_t naruto_clone_cond = PTHREAD_COND_INITIALIZER;	// Inicializa a condição de parada dos clones do Naruto

pthread_mutex_t healing_zone = PTHREAD_MUTEX_INITIALIZER;	// Inicializa o lock da zona de cura da Sakura 	

sem_t power_portions;	// Declara o semáforo de porções de poder do Rock Leee

void *f_sakura(void *arg);
void *f_regular_ninja(void *arg);
void *f_naruto_clone(void *arg);
void *f_rock_lee(void *arg);

int main(int argc, char**argv) {
    int i;
    int *id;
    
	// Inicialmente o Lee não tem nenhuma porção de poder disponível
    sem_init(&power_portions, 0, 0);        
	
	id = (int *) malloc(sizeof(int));
	*id = 0;
	
	// Cria a thread da Sakura 
	pthread_create(&sakura, NULL, f_sakura, (void *)(id));
	// Cria a thread do Rock Lee
	pthread_create(&rock_lee, NULL, f_rock_lee, (void *)(id));

	// Cria as threads dos ninjas comuns
    for (i = 0; i < NUM_REG_NINJAS; i++) {
        id = (int *) malloc(sizeof(int));
        *id = i;
        pthread_create(&regular_ninjas[i], NULL, f_regular_ninja, (void *)(id));
    }

	// Cria as threads dos clones do naruto
    for (i = 0; i < NUM_CLONES; i++) {
        id = (int *) malloc(sizeof(int));
        *id = i;
        pthread_create(&naruto_clones[i], NULL, f_naruto_clone, (void *)(id));
    }

    pthread_join(sakura, NULL);
    pthread_join(rock_lee, NULL);
    sem_destroy(&power_portions);

    return 0;
}

void *f_sakura(void *arg) {
	while(1) {
        pthread_mutex_lock(&healing_zone); 
            // Enquanto houver houver chakra suficiente, a Sakura pode descansar
			while(sakura_chakra > CHAKRA_TO_HEAL_NINJA) {							
				printf("Sakura não precisa reunir mais chakra por hora... -- Nivel de chakra atual: %d\n", sakura_chakra);
                pthread_cond_wait(&sakura_cond, &healing_zone);
            }
			
			// Se o chakra for insuficiente, Sakura deve reunir mais
            printf("Sakura esta reunindo chakra... -- Nivel de chakra da Sakura: %d\n", sakura_chakra);
            sakura_chakra = MAX_CHAKRA_LEVEL;
			sleep(4);
            printf("Sakura esta com 100 por cento de seu chakra!\n");

			// Acorda os ninjas e os clones para que eles possam entrar na zona de cura
			pthread_cond_broadcast(&regular_ninja_cond);
            pthread_cond_broadcast(&naruto_clone_cond);
        pthread_mutex_unlock(&healing_zone);
    }

    pthread_exit(0);
}

void *f_regular_ninja(void *arg) {
	int id = *((int *) arg);

	printf("Ninja %d entrou no campo de batalha!\n", id);

    while(1) {
		sleep(5+rand()%5);

		// Ninja se machuca e pega o lock da zona de cura
        pthread_mutex_lock(&healing_zone);
			// Incrementa a variável para indicar que um ninja normal precisa de cura
            regular_ninjas_injured++;    
			printf("O ninja %d se machucou e precisa ser curado!\n", id);
            
			// Enquanto a sakura não tiver chakra o suficiente, o ninja aguarda
			while(sakura_chakra < CHAKRA_TO_HEAL_NINJA) {
				printf("Sakura nao tem chakra suficiente para curar o ninja %d -- Nivel de chakra da Sakura: %d\n", id, sakura_chakra);
                pthread_cond_signal(&sakura_cond); 
                pthread_cond_wait(&regular_ninja_cond, &healing_zone); 
            }
            
			regular_ninjas_injured--;
            sakura_chakra = sakura_chakra - CHAKRA_TO_HEAL_NINJA;
			printf("Ninja %d foi curado e vai retornar para o campo de batalha! -- Nivel de chakra da Sakura: %d\n", id, sakura_chakra);
        pthread_mutex_unlock(&healing_zone);
    }

    pthread_exit(0);
}

void *f_naruto_clone(void *arg) {
    int id = *((int *) arg);
	int vital_power = 0;
	int value = 0;

	printf("Naruto fez o clone de numero %d!\n", id);

    while(1) {
        sleep(rand()%3);

        pthread_mutex_lock(&healing_zone);
			printf("O clone %d precisa de mais forca vital para ajudar o Lee! -- Forca vital atual: %d\n", id, vital_power);

			// O clone só será atendido quando não houver ninjas normais esperando pelo atendimento e o chakra da sakura for o suficiente
            while(sakura_chakra < CHAKRA_TO_VITALIZE_CLONE || regular_ninjas_injured > 0) {
				if (sakura_chakra < 15) {
					printf("Sakura nao tem chakra suficiente para dar força vital ao clone %d -- Nivel de chakra da Sakura: %d\n", id, sakura_chakra);
				}
				
				if (regular_ninjas_injured > 0) {
					printf("O clone %d nao pode ser curado pois ha um ninja machucado -- Nivel de chakra da Sakura: %d\n", id, sakura_chakra);
				}
				
                pthread_cond_signal(&sakura_cond); 
                pthread_cond_wait(&naruto_clone_cond, &healing_zone);
			} 

			vital_power = vital_power + 15;
			sakura_chakra = sakura_chakra - CHAKRA_TO_VITALIZE_CLONE;
			printf("O clone %d recebeu um pouco de poder vital! -- Forca vital atual: %d -- Nivel de chakra da Sakura: %d\n", id, vital_power, sakura_chakra);
        pthread_mutex_unlock(&healing_zone);

		// Enquanto o clone tiver poder vital para ajudar o Lee, ele o fará
        while (vital_power >= POWER_TO_HELP_LEE) {
            sem_post(&power_portions);
			sem_getvalue(&power_portions, &value);
            vital_power = vital_power - POWER_TO_HELP_LEE;
			printf("O clone %d deu uma porcao de poder ao Rock Lee, agora ele tem %d! -- Forca vital atual: %d\n", id, value, vital_power);
        }
    }

    pthread_exit(0);
}

void *f_rock_lee(void *arg) {
	int value = 0;

    printf("Rock Lee entrou no campo de batalha!\n");

    while(1) {
		sem_getvalue(&power_portions, &value);
		printf("Rock Lee esta lutando contra os inimigos...\n");
		sleep(2+rand()%5);

		// Sempre que houver porções de poder para o Lee, ele pega uma e vai lutar...
        sem_wait(&power_portions);
		printf("Rock Lee utilizou uma porcao do poder do Naruto! -- Quantidade de porcoes restantes: %d\n", value);
    }
	
    pthread_exit(0);
}