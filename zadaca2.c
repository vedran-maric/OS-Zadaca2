#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>


int *PRAVO;
int *ZASTAVICA;
int *suma;
int SHMID;

void udji_u_odsjecak(int i, int j){
	/*DEKKER - ulazni algoritam*/
	int broj_zastavice = i;
	int pravo_ulaska = j;
	ZASTAVICA[broj_zastavice] = 1;

	while(ZASTAVICA[pravo_ulaska]!=0){
		if (*PRAVO == pravo_ulaska)	{
			ZASTAVICA[broj_zastavice] = 0;
			while(*PRAVO == pravo_ulaska){}
			ZASTAVICA[broj_zastavice] = 1;
		}
	}
}

void izadji_iz_odsjecka(int i, int j){
	int broj_zastavice = i;
	int pravo_ulaska = j;
	*PRAVO = pravo_ulaska;
	ZASTAVICA[broj_zastavice] = 0;
}

/* Funkcija za ispis podataka*/
void proc(int i)
{
	int broj_zastavice = i;
	printf("Proces %d trazi pristup kriticnom odsjecku\n", broj_zastavice);

	for (int k_ponavljanje_petlje = 1; k_ponavljanje_petlje <= 5; k_ponavljanje_petlje++){
		udji_u_odsjecak(broj_zastavice, 1 - broj_zastavice);
		printf("Proces %d uspjesno usao u kriticni odsjecak i pocinje obradu\n", broj_zastavice);
		for (int m_ponavljanje_zbrajanja = 1; m_ponavljanje_zbrajanja <= 5; m_ponavljanje_zbrajanja++){
			*suma += 1;
			printf("suma = %d (i=%d, k = %d, m = %d)\n", *suma, broj_zastavice, k_ponavljanje_petlje, m_ponavljanje_zbrajanja);
			usleep(500000);
		}
		printf("Proces %d izlazi iz kriticnog odsjecka\n", broj_zastavice);
		izadji_iz_odsjecka(broj_zastavice, 1 - broj_zastavice);
	}
}

int main(){

	/*Zauzimanje zajednicke memorije za sumu, pravo i dvije zastavice (4 int-a)*/
	SHMID = shmget(IPC_PRIVATE, sizeof(int) * 4, 0600);

	/*U slucaju nedostatka memorije*/
	if (SHMID == -1){
		perror("shmget");
		exit(1);
	}

	/*Podjela zajednicke memorije*/
	int *zajednicka_memorija = (int *)shmat(SHMID, NULL, 0); 

	suma = zajednicka_memorija;			
	PRAVO = zajednicka_memorija + 1;		 
	ZASTAVICA = zajednicka_memorija + 2; 

	/*Postavljanje vrijednosti*/
	*suma = 0;
	*PRAVO = 0; 
	ZASTAVICA[0] = 0;
	ZASTAVICA[1] = 0;
	
	/*Pokretanje podprograma*/
	if (fork() == 0){
		proc(0);
		exit(0);
	}

	if (fork() == 0){
		proc(1);
		exit(0);
	}

	wait(NULL);
	wait(NULL);

	/*Oslobadjanje pa brisanje memorije*/
	shmdt(PRAVO);
	shmdt(ZASTAVICA);
	shmctl(SHMID, IPC_RMID, NULL);

	exit(0);

	/*Napisao Vedran Maric 2215/RR

	gcc zadaca2.c -o Zadaca2 -lm
	sudo ./Zadaca2
	*/

	return 0;
}