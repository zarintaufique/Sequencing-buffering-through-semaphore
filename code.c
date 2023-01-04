// Libraries
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <semaphore.h>
#include <sys/shm.h>


// Main starts here
int main(int argc, char*argv[]){
	
	//creating all the shared memories
	int skey=shmget(71,200, 0666 | IPC_CREAT | IPC_EXCL);//A
	int skey1=shmget(72,200, 0666 | IPC_CREAT | IPC_EXCL);//B
	int skey2=shmget(73,200, 0666 | IPC_CREAT | IPC_EXCL);//C
	int skey3=shmget(74,200, 0666 | IPC_CREAT | IPC_EXCL);//D
	int id_cs1= shmget(75, 200, 0666| IPC_CREAT | IPC_EXCL);//buffer1
	int id_cs2= shmget(76, 200, 0666| IPC_CREAT | IPC_EXCL);//buffer2
	int id_cs3= shmget(77, 200, 0666| IPC_CREAT | IPC_EXCL);//buffer3

	//error handling
	if (skey==-1 || skey1==-1 || id_cs1==-1 || skey2==-1||id_cs2==-1||skey3==-1||id_cs3==-1) {
		printf("error occured in creating shared memory\n");
		return 1;
	}
	
	//semaphore initialization, attachment
	sem_t *s1;
	sem_t *s2;
	sem_t *s3;
	sem_t *s4;
	s1= (sem_t*) shmat(skey, NULL, 0);
	s2= (sem_t*) shmat(skey1, NULL, 0);
	s3= (sem_t*) shmat(skey2, NULL, 0);
	s4= (sem_t*) shmat(skey3, NULL, 0);
	sem_init(s1, 1, 1);    //unlocked
	sem_init(s2, 1, 0);   //locked
	sem_init(s3, 1, 0);    //locked
	sem_init(s4, 1, 0);   //locked
	shmdt(s1);
	shmdt(s2);
	shmdt(s3);
	shmdt(s4);

	int pid = fork();

	//first process
	if (pid == 0) {      

		//pointers to the shared memory
		s1= (sem_t*) shmat(skey, NULL, 0);
		s2= (sem_t*) shmat(skey1, NULL, 0);

		sem_wait(s1);
		char* buffer1= (char*) shmat(id_cs1, NULL, 0);
		
		//reading from first file
		char ch = '\0';
		int index = 0;
		FILE* fd = fopen("file1.txt","r");
		if (fd == NULL){
			printf("Error Number %d", errno); 
			perror("Program");  			
		}
		while ((ch = fgetc(fd)) != EOF) {
			buffer1[index] = ch;   //storing the whole sentence in an array
			ch = fgetc(fd);
			index++;
			buffer1[index] = ch;
			index++;
		}
		buffer1[index++] = '\0';
		buffer1[index++] = ' ';
		fclose(fd);

		sem_post(s2);  //file2
	}
	
	else {
		//reading from second file
		pid=fork();
		if(pid == 0) {
			sem_t *s2;
			sem_t *s3;
			s2 = (sem_t*) shmat(skey1, NULL, 0);
			s3 = (sem_t*) shmat(skey2, NULL, 0);

			char* buffer2 = (char*) shmat(id_cs2, NULL, 0);

			char ch = '\0';
			int index = 0;
			FILE* fd = fopen("file2.txt","r");
			if (fd == NULL){
				printf("Error Number %d", errno); 
				perror("Program");  			
			}
			sem_wait(s2);
			while ((ch= fgetc(fd)) != EOF) {
				buffer2[index]=ch;

				ch = fgetc(fd);
				index++;
				buffer2[index] = ch;
				index++;
			}
			buffer2[index++] = '\0';
			buffer2[index++] = ' ';
			fclose(fd);

			sem_post(s3);
			//detachment of the pointers
			shmdt(s1);
			shmdt(s2);
			shmdt(buffer2);
		}
		else {  
			//combining of buffer1 and buffer2
			pid = fork();  

			if(pid == 0) {     
			sem_t *s3;
			sem_t *s4;

			s3= (sem_t*) shmat(skey2, NULL, 0);
			s4= (sem_t*) shmat(skey3, NULL, 0);

			char* buffer1= (char*) shmat(id_cs1, NULL, 0);
			char* buffer2= (char*) shmat(id_cs2, NULL, 0);
			char* buffer3= (char*) shmat(id_cs3, NULL, 0);

			sem_wait(s3);
			 
			int i = 0, j = 0;
			int size1 = strlen(buffer1);
			int size2 = strlen(buffer2);

			for(i = 0;i < size1;i++) {
				buffer3[i] = buffer1[i];
			}

			for(j = 0;j < size2;i++, j++) {
				buffer3[i] = buffer2[j];
			}

			buffer3[i] = '\0';

			sem_post(s4);
			
			//detachment of pointers
			shmdt(s3);
			shmdt(s4);
			shmdt(buffer2);
			shmdt(buffer1);
			shmdt(buffer3);
			}
			else {
				//printing of buffer3
				s4= (sem_t*) shmat(skey3, NULL, 0);
				sem_wait(s4);
				char* buffer3= (char*) shmat(id_cs3, NULL, 0);

				printf("%s",buffer3);

				shmdt(buffer3);
				shmdt(s4);
				
				//deletion of all shared memories
				shmctl(skey, IPC_RMID, NULL);
				shmctl(skey1, IPC_RMID, NULL);
				shmctl(skey2, IPC_RMID, NULL);
				shmctl(skey3, IPC_RMID, NULL);
				shmctl(id_cs1, IPC_RMID, NULL);
				shmctl(id_cs2, IPC_RMID, NULL);
				shmctl(id_cs3, IPC_RMID, NULL);
			}
		}
	}		
	return 0;
}
// end of main
