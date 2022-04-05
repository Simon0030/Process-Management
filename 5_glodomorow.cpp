#include <iostream>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdio.h>
#include <cstdlib>

#define hungry_mans 5
#define dinners 3
#define eTime 10
#define wTime 10 

using namespace std;

static struct sembuf buf;

void Take(int semid, int semnum)
{
    buf.sem_num = semnum;
    buf.sem_op = -1;
    buf.sem_flg = 0;
    if (semop(semid, &buf, 1) == -1)
    {
        perror("Blad podniesienia");
        exit(1);
    }
}

void Drop(int semid, int semnum)
{
    buf.sem_num = semnum;
    buf.sem_op = 1;
    buf.sem_flg = 0;
    if (semop(semid, &buf, 1) == -1)
    {
        perror("Blad opuszczenia");
        exit(1);
    }
}

int main()
{
    int startID, koniecID, knivesID, processID, statusID, WagiID;
    int *start, *koniec, *status, *Wagi;

    startID = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT|0640);
    start = (int*)shmat(startID, NULL, 0);
    start[0] = (int)hungry_mans;   
               
    knivesID = semget(IPC_PRIVATE, (int)hungry_mans, IPC_CREAT|0640);    

    for (int i=0; i<(int)hungry_mans; i++)
        semctl(knivesID, i, SETVAL, int(1));   

    statusID = shmget(IPC_PRIVATE, (int)hungry_mans*sizeof(bool), IPC_CREAT|0640);
    status = (int*)shmat(statusID, NULL, 0);
    WagiID = shmget(IPC_PRIVATE, int(hungry_mans)*sizeof(int), IPC_CREAT|0640);
    Wagi = (int*)shmat(WagiID, NULL, 0); 
    koniecID = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT|0600);
    koniec = (int*)shmat(koniecID, NULL, 0);
    koniec[0] = (int)hungry_mans;

    for (int i=0; i<(int)hungry_mans-1; i++)
    {
        processID = fork();
        if (processID == 0)
        {
            processID = i+1;
            break;
        }
        else if(i == (int)hungry_mans-2)
            processID = 0;
    }
    start[0]--;

    while (start[0] != 0);
    	cout << "Hungry man " << processID << " dosiada się" << endl;

    int lewyID = (processID+1)%int(hungry_mans);
    int prawyID = processID;
    int uczta = dinners;

    while(uczta)
    {
        cout<<"Hungry man "<<processID<<" pozostalo do zjedzenia: "<<uczta<<endl;
        int waga = uczta;
        if (processID%2==0)
        {
            cout<<"Hungry Man "<<processID<<" rząda "<<lewyID<<" (lewy)"<<endl;
            while (!(status[(processID+1)%(int)hungry_mans]==1||(status[(processID+1)%(int)hungry_mans]==0&&Wagi[processID]<=Wagi[(processID+1)%(int)hungry_mans])));
            	Take(knivesID, lewyID);
            	cout<<"Hungry Man "<<processID<<" trzyma "<<lewyID<<" (lewy)"<<endl;
            	cout<<"Hungry Man "<<processID<<" rząda "<<prawyID<<" (prawy)"<<endl;

            while (!(status[(processID+(int)hungry_mans-1)%(int)hungry_mans]==1||(status[(processID+(int)hungry_mans-1)%(int)hungry_mans]==0&&Wagi[processID]<=Wagi[(processID+(int)hungry_mans-1)%(int)hungry_mans])));
            	Take(knivesID, prawyID);
            	cout<<"Hungry Man "<<processID<<" trzyma "<<prawyID<<" (prawy)"<<endl;
        }

        else 
        {
            cout<<"Hungry Man "<<processID<<" rząda "<<prawyID<<" (prawy)"<<endl;

            while (!(status[(processID+(int)hungry_mans-1)%(int)hungry_mans]==1||(status[(processID+(int)hungry_mans-1)%(int)hungry_mans]==0&&Wagi[processID]<=Wagi[(processID+(int)hungry_mans-1)%(int)hungry_mans])));
            	Take(knivesID, prawyID);
            	cout<<"Hungry Man "<<processID<<" trzyma "<<prawyID<<" (prawy)"<<endl;
            	cout<<"Hungry Man "<<processID<<" rząda "<<lewyID<<" (lewy)"<<endl;

            while (!(status[(processID+1)%(int)hungry_mans]==1||(status[(processID+1)%(int)hungry_mans]==0&&Wagi[processID]<=Wagi[(processID+1)%(int)hungry_mans])));
            	Take(knivesID, lewyID);
            	cout<<"Hungry Man "<<processID<<" trzyma "<<lewyID<<" (lewy)"<<endl;
        }

        status[processID] = 1;
        cout<<"Hungry Man "<<processID<<" je posilek o wadze "<<waga<<endl;
        Wagi[processID] += waga;
        usleep(eTime*1000);
        uczta--;
        cout<<"Hungry Man "<<processID<<" zakończony(opuszcza sztućce) - zjedzone posilki w sumie(wagi): "<<Wagi[processID]<<endl;
        Drop(knivesID, lewyID);
        Drop(knivesID, prawyID);

        cout<<"Hungry Man "<<processID<<" oczekuje"<<endl;
        usleep(wTime*1000);
        cout<<"Hungry Man "<<processID<<" zakonczyl oczekiwanie"<<endl;
        status[processID] = 0;
    }
    cout << "Hungry Man " << processID << " odszedł od stołu" << endl;
    koniec[0]--;
    while(koniec[0]);

    if (processID==0)
    {
        cout << "Wszyscy skonczyli uczte" << endl;
    }
}
