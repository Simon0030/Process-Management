#include <iostream>
#include <cstdio>       //printf
#include <unistd.h>     //sleep
#include <pthread.h>
#include <ctime>

#define giftingTime 5       // w sekundach
#define consultationTime 3  // w sekundach

#define sleepTimeRen1 1     // w sekundach
#define sleepTimeRen2 20    // w sekundach
#define sleepTimeSkrz1 1    // w sekundach
#define sleepTimeSkrz2 30   // w sekundach

using namespace std;

int licznikRen = 0;
int licznikSkrz = 0;

pthread_mutex_t mutexStart = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condStart = PTHREAD_COND_INITIALIZER;

pthread_mutex_t mutexRen = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexSkrz = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condRen = PTHREAD_COND_INITIALIZER;
pthread_cond_t condSkrz = PTHREAD_COND_INITIALIZER;

pthread_mutex_t mutexRenEnd = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexSkrzEnd = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condRenEnd = PTHREAD_COND_INITIALIZER;
pthread_cond_t condSkrzEnd = PTHREAD_COND_INITIALIZER;

void *ReniferRoutine(void *it)
{
    pthread_mutex_lock(&mutexStart);
    pthread_cond_wait(&condStart, &mutexStart);
    pthread_mutex_unlock(&mutexStart);

    srand(time(NULL));
    int t1 = sleepTimeRen1;
    int t2 = sleepTimeRen2;
    int timeDif = t2-t1;

    long reniferID = (long)it;
    printf("    Jestem renifer %ld\n", reniferID);

    while(1)
    {
        sleep(rand()%timeDif+t1);   // dla t1=1s i t2=20s   =>  sleep na: 1-20s

        pthread_mutex_lock(&mutexRen);
        licznikRen++;
        printf("\t\tRenifer %ld czeka, licznik reniferow w kolejce: %i\n", reniferID, licznikRen);
        pthread_cond_wait(&condRen, &mutexRen);
        printf(" Renifer %ld zostaje zaprzezony\n", reniferID);
        licznikRen--;
        pthread_mutex_unlock(&mutexRen);

        pthread_mutex_lock(&mutexRenEnd);
        pthread_cond_wait(&condRenEnd, &mutexRenEnd);
        printf(" Renifer %ld konczy prace\n", reniferID);
        pthread_mutex_unlock(&mutexRenEnd);
    } 
}

void *SkrzatRoutine(void *it)
{
    pthread_mutex_lock(&mutexStart);
    pthread_cond_wait(&condStart, &mutexStart);
    pthread_mutex_unlock(&mutexStart);

    srand(time(NULL));
    int t1 = sleepTimeSkrz1;
    int t2 = sleepTimeSkrz2;
    int timeDif = t2-t1;

    long skrzatID = (long)it;
    printf("  Jestem skrzat %ld\n", skrzatID);

    while(1)
    {
        sleep(rand()%timeDif+t1);   // dla t1=1s i t2=30s   =>  sleep na: 1-30s

        pthread_mutex_lock(&mutexSkrz);
        licznikSkrz++;
        printf("\tSkrzat %ld czeka, licznik skrzatow w kolejce: %i\n", skrzatID, licznikSkrz);
        pthread_cond_wait(&condSkrz, &mutexSkrz);
        printf(" Skrzat %ld zaproszony do biura\n", skrzatID);
        licznikSkrz--;
        pthread_mutex_unlock(&mutexSkrz);

        pthread_mutex_lock(&mutexSkrzEnd);
        pthread_cond_wait(&condSkrzEnd, &mutexSkrzEnd);
        printf(" Skrzat %ld wychodzi od Mikolaja\n", skrzatID);
        pthread_mutex_unlock(&mutexSkrzEnd);
    }
}

int main()
{
    pthread_t renifery[9];
    pthread_t skrzaty[10];
    long it;
    printf("TWORZENIE WATKOW:\n");
    for (int i=0; i<9; i++)
    {
        it = long(i);
        if(pthread_create(&renifery[i], NULL, ReniferRoutine, (void *)it))
        {perror("Nie udalo sie stworzyc watku 'renifer'");exit(1);}
    }
    for (int i=0; i<10; i++)
    {
        it = long(i);
        if(pthread_create(&skrzaty[i], NULL, SkrzatRoutine, (void *)it))
        {perror("Nie udalo sie stworzyc watku 'skrzat'");exit(1);}
    }
    pthread_cond_broadcast(&condStart); // obudzenie wszystkich watkow
    usleep(20000);
    printf("START:\n");

    while(1)
    {
        if (licznikRen==9)
        {
            usleep(20000);
            //obsługa reniferów
            printf("Mikolaj budzi się i zaprzega renifery\n");
            pthread_cond_broadcast(&condRen);

            usleep(20000);
            printf("Renifery zaprzezone, Mikolaj dostarcza prezenty..\n");
            sleep((int)giftingTime);

            printf("Koniec dostarczania prezentow, Mikolaj idzie spac\n");
            pthread_cond_broadcast(&condRenEnd);
        }
        if (licznikSkrz>=3 && licznikRen!=9)
        {
            usleep(20000);
            //obsługa skrzatów
            printf("Mikolaj budzi się i zaprasza skrzaty do biura\n");
            pthread_cond_broadcast(&condSkrz);

            usleep(20000);
            printf("Udzielanie konsultacji skrzatom..\n");
            sleep((int)consultationTime);

            printf("Koniec konsultacji, Mikolaj idzie spac\n");
            pthread_cond_broadcast(&condSkrzEnd);
        }
    }

    return 0;
}