#include <iostream>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>

#define liczbaFil 10    // liczba filozofow
#define liczbaPos 3    // liczba posilkow dla kazdego filozofa
#define eatingTime 3    // czas jedzenia w milisekundach
#define waitingTime 3   // czas czekania w milisekundach

using namespace std;

static struct sembuf buf;

void podniesWid(int semid, int semnum)
{
    buf.sem_num = semnum;
    buf.sem_op = -1;
    buf.sem_flg = 0;
    if (semop(semid, &buf, 1) == -1)
    {
        perror("Blad przy podnoszeniu widelca");
        exit(1);
    }
}

void opuscWid(int semid, int semnum)
{
    buf.sem_num = semnum;
    buf.sem_op = 1;
    buf.sem_flg = 0;
    if (semop(semid, &buf, 1) == -1)
    {
        perror("Blad przy opuszczaniu widelca");
        exit(1);
    }
}

int main()
{
    // Deklaracja pamieci wspoldzielonych i semaforow
    
    int startID, widelceID, statusID, tabWagID, koniecID, processID;
    int *start, *status, *koniec, *tabWag;
    // pamiec wspoldzielona 'start[0]' - zmienna int
    startID = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT|0640); // zmienna 'start' odpowiedzialna za rownoczesne wystartowanie filozofow;
    start = (int*)shmat(startID, NULL, 0);  // zatrzymuje procesy w oczekiwaniu na stworzenie wszystkich potrzebnych procesow
    start[0] = (int)liczbaFil;                  
    // tablica semaforow 'widelceID'
    widelceID = semget(IPC_PRIVATE, (int)liczbaFil, IPC_CREAT|0640);    // deklaracja widelcow (tablica semaforow)
    for (int i=0; i<(int)liczbaFil; i++)
        semctl(widelceID, i, SETVAL, int(1));   // zmienna semaforowa dla kazdego semafora jest ustawiana na 1
    // pamiec wspoldzielona 'status[n]' - tablica int
    statusID = shmget(IPC_PRIVATE, (int)liczbaFil*sizeof(bool), IPC_CREAT|0640);
    status = (int*)shmat(statusID, NULL, 0);    // deklaracja statusu jako wspoldzielona tablica bool
    // status:
    // 0 - chce jesc
    // 1 - jest w trakcie jedzenia (takze: nie chce jesc, bo je)
    
    // pamiec wspoldzielona 'tabWag[n]' - tablica int
    tabWagID = shmget(IPC_PRIVATE, int(liczbaFil)*sizeof(int), IPC_CREAT|0640);    // deklaracja tablicy sumy wag zjedzonych
    tabWag = (int*)shmat(tabWagID, NULL, 0);                                       // posilkow przez kazdego filozofa
    
    // pamiec wspoldzielona 'koniec[0]' - zmienna int
    koniecID = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT|0600);
    koniec = (int*)shmat(koniecID, NULL, 0);    // zmienna 'koniec' odpowiedzialna za rownoczesne zakonczenie uczty
    koniec[0] = (int)liczbaFil;                 // przez wszystkich filozofow; konczy program kiedy wszyscy osiagneli koniec uczty
    
    // Kontrola bledow
    
    if (startID == -1){perror("Utworzenie segmentu pamieci wspoldzielonej (start)");exit(1);}
    if (start == NULL){perror("Przylaczenie segmentu pamieci wspoldzielonej (start)");exit(1);}
    if (widelceID == -1){perror("Utworzenie tablicy semaforow (widelce)");exit(1);}
    if (statusID == -1){perror("Utworzenie segmentu pamieci wspoldzielonej (status)");exit(1);}
    if (status == NULL){perror("Przylaczenie segmentu pamieci wspoldzielonej (status)");exit(1);}
    if (koniecID == -1){perror("Utworzenie segmentu pamieci wspoldzielonej (koniec)");exit(1);}
    if (koniec == NULL){perror("Przylaczenie segmentu pamieci wspoldzielonej (koniec)");exit(1);}
    if (tabWagID == -1){perror("Utworzenie segmentu pamieci wspoldzielonej (tabWag)");exit(1);}
    if (tabWag == NULL){perror("Przylaczenie segmentu pamieci wspoldzielonej (tabWag)");exit(1);}

    // Tworzenie dodatkowych procesow

    for (int i=0; i<(int)liczbaFil-1; i++)
    {
        processID = fork();
        if (processID == 0)     // jeden z utworzonych procesow (w tym przypadku potomny) przerywa petle
        {
            processID = i+1;
            break;
        }
        else if(i == (int)liczbaFil-2)
            processID = 0;
    }
    start[0]--;
    while (start[0] != 0);

    // Kazdy proces jest oddzielnym filozofem o numerze id: processID

    cout << "Jestem filozofem " << processID << endl;

    int lewyID = (processID+1)%int(liczbaFil);  // identyfikator lewego widelca
    int prawyID = processID;                    // identyfikator prawego widelca
    int posilki = liczbaPos;

    // Poczatek uczty

    while(posilki)
    {
        cout<<"Dla filozofa "<<processID<<" pozostalo posiłków do zjedzenia: "<<posilki<<endl;
        int wagaPos = posilki;
        // proces z id=0 jest leworeczny (jako pierwszy widelec probuje podniesc ten z lewej strony)
        if (processID==0)
        { // 1 podnosi jeśli 2 nie chce jeść lub (2 chce jeść i 2 ma wyższy priorytet)
            cout<<"Filozof "<<processID<<" chce podniesc widelec "<<lewyID<<" (lewy)"<<endl;
            while (!(status[(processID+1)%(int)liczbaFil]==1||(status[(processID+1)%(int)liczbaFil]==0&&tabWag[processID]<=tabWag[(processID+1)%(int)liczbaFil])));
            podniesWid(widelceID, lewyID);
            cout<<"Filozof "<<processID<<" podniosl widelec "<<lewyID<<" (lewy)"<<endl;
            cout<<"Filozof "<<processID<<" chce podniesc widelec "<<prawyID<<" (prawy)"<<endl;
            while (!(status[(processID+(int)liczbaFil-1)%(int)liczbaFil]==1||(status[(processID+(int)liczbaFil-1)%(int)liczbaFil]==0&&tabWag[processID]<=tabWag[(processID+(int)liczbaFil-1)%(int)liczbaFil])));
            podniesWid(widelceID, prawyID);
            cout<<"Filozof "<<processID<<" podniosl widelec "<<prawyID<<" (prawy)"<<endl;
        }
        else // reszta filozofow jest praworeczna (jako pierwszy widelec wybieraja zawsze ten z prawej strony)
        {
            cout<<"Filozof "<<processID<<" chce podniesc widelec "<<prawyID<<" (prawy)"<<endl;
            while (!(status[(processID+(int)liczbaFil-1)%(int)liczbaFil]==1||(status[(processID+(int)liczbaFil-1)%(int)liczbaFil]==0&&tabWag[processID]<=tabWag[(processID+(int)liczbaFil-1)%(int)liczbaFil])));
            podniesWid(widelceID, prawyID);
            cout<<"Filozof "<<processID<<" podniosl widelec "<<prawyID<<" (prawy)"<<endl;
            cout<<"Filozof "<<processID<<" chce podniesc widelec "<<lewyID<<" (lewy)"<<endl;
            while (!(status[(processID+1)%(int)liczbaFil]==1||(status[(processID+1)%(int)liczbaFil]==0&&tabWag[processID]<=tabWag[(processID+1)%(int)liczbaFil])));
            podniesWid(widelceID, lewyID);
            cout<<"Filozof "<<processID<<" podniosl widelec "<<lewyID<<" (lewy)"<<endl;
        }
        status[processID] = 1;
        // filozof zaczyna jedzenie posilku
        cout<<"Filozof "<<processID<<" je posilek o wadze "<<wagaPos<<endl;
        tabWag[processID] += wagaPos;
        usleep(eatingTime*1000);
        posilki--;
        cout<<"Filozof "<<processID<<" skonczyl jesc; opuszcza widelce; waga zjedzonych posilkow w sumie: "<<tabWag[processID]<<endl;
        opuscWid(widelceID, lewyID);
        opuscWid(widelceID, prawyID);
        // filozof rozmysla/odpoczywa
        cout<<"Filozof "<<processID<<" rozmysla"<<endl;
        usleep(waitingTime*1000);
        cout<<"Filozof "<<processID<<" skonczyl rozmyslac"<<endl;
        status[processID] = 0;
    }
    cout << "Filozof " << processID << " zakonczyl uczte" << endl;
    koniec[0]--;
    while(koniec[0]);
    // process o id=0 wypisze komunikat o skonczeniu uczty i sie zakonczy
    // reszta procesow nic nie wypisze i sie zakoncza
    if (processID==0)
    {
        cout << "Wszyscy filozofowie skonczyli uczte" << endl;
        cout << "KONIEC" << endl;
    }
}