#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

union semun{
	int val;
	struct semid_ds *buf;
	unsigned short *array;
	struct seminfo *__buf;
};


int main(int argc, const char *argv[]){

	key_t key = 69;
	int shmid;
	int semid;
	union semun arg;
	struct sembuf lock_res = {0, -1, 0}; //блокировка ресурса 
	struct sembuf rel_res = {0, 1, 0}; //освобождения ресурса 
	char *s, str;
	char *shm;

	if (argc < 3){
		fprintf (stderr, "Используйте %s <ключ> <строка>\n", argv[0]);
		exit (1);
	}

	sscanf(argv[1], "%d", &key);

	/*получим ключ*/
	if ((key = ftok(".", 'S')) < 0){
		printf("Невозможно получить ключ\n");
		exit(1);
	}

	/*Подключимся к семафору семафор*/
	semid = semget(key, 1, 0666);

	/*Установим в семафоре № 1 (Контроллер ресурса) значение 1*/
	//arg.val = 1;
	//semctl(semid, 0, SETVAL, arg);
	int size = strlen(&str);
	/* Подключим область разделяемой памяти */
	if ((shmid = shmget(key, sizeof(argv[2]), 0666)) < 0){
		perror("shmget");
		exit(1);
	}
	/* Получим доступ к разделяемой памяти */
	if ((shm = (char *)shmat(shmid, NULL, 0)) == (char *) -1) {
		perror("shmat");
		exit(1);
	}

	for (int i = 2; i<argc; i++)
	{
		strcat(shm,argv[i]);
		strcat(shm," ");
	}
	
	semop(semid, &rel_res, 1);
	
	if (shmdt(shm) < 0) {
		printf("Ошибка отключения\n");
		exit(1);
	} 
	if (semctl(semid, 0, IPC_RMID) < 0){
	printf("Невозможно удалить семафор\n");
	exit(1);
	}
	exit(0);
}
