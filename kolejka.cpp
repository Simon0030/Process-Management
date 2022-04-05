#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <pthread.h>
#include <ctime>
#include <cstdlib>

#define przejazd 5       // w sekundach
#define sleepTimePass1 1
#define sleepTimePass2 20


using namespace std;

int licznik = 0;
int P = 6;
int N = P + 12;

pthread_mutex_t mutexPass = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condPass = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutexPassEnd = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condPassEnd = PTHREAD_COND_INITIALIZER;

void *Passenger(void *it)
{
    srand(time(NULL));
    int t1 = sleepTimePass1;
    int t2 = sleepTimePass2;
    int timeDif = t2-t1;

    long passengerID = (long)it;
    printf("Witamy pasażera %ld\n", passengerID);

    while(1)
    {
        sleep(rand()%timeDif+t1);   // sleep między: 1-20s	
	if (++licznik<=P)
	{
		pthread_mutex_lock(&mutexPass);
        	printf("Pasażer %ld czeka, osoby oczekujące: %i\n", passengerID, licznik);	
		pthread_cond_wait(&condPass, &mutexPass);
        	printf("Pasażer %ld wsiada na miejsce\n", passengerID);
        	pthread_mutex_unlock(&mutexPass);
		licznik--;
		pthread_mutex_lock(&mutexPassEnd);
        	pthread_cond_wait(&condPassEnd, &mutexPassEnd);
        	printf("Pasażer %ld wysiada z kolejki\n", passengerID);
        	pthread_mutex_unlock(&mutexPassEnd);
	}
	else{licznik--;}
    } 
}


int main()
{
    printf("Liczba ludzi w lunaparku: %d\n", N);
    printf("Limit miejsc w DIABELSKIEJ KOLEJCE: %d\n", P);
    pthread_t passengers[N];
    long it;
    printf("\nTWORZENIE WATKOW:\n");
    for (int i=0; i<N; i++)
    {
        it = long(i);
        if(pthread_create(&passengers[i], NULL, Passenger, (void *)it))
        {perror("Nie udalo sie stworzyc watku");exit(1);}
    }

    usleep(20000);
    printf("\nSTART:\n");

    while(1)
    {
        if (licznik==P)
        {
            usleep(20000);
            printf("Kolejka oczekuje na pasażerów\n");
            pthread_cond_broadcast(&condPass);

            usleep(20000);
            printf("Miejsca zapełnione kolejka rusza.\n");
            sleep((int)przejazd);

            printf("Koniec przejażdżki. Kolejka zatrzymuje się.\n");
            pthread_cond_broadcast(&condPassEnd);
        }
    }

    return 0;
}
