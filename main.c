#include <unistd.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include <signal.h>
#include <fcntl.h>
#include <semaphore.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nvc.h" //Custom header for this program. Nurse, Vaccinator, Citizen

//-------------------------------------------------
// 		İLKAN MERT OKUL, 
//		ilkanmertokul@gmail.com 
//		github/ilkanmertokul
//-------------------------------------------------

#define SEM_FLAGS O_CREAT | O_EXCL 	//to create uninitialized semaphore.
#define SEM_PERMS S_IRWXU | S_IRWXG | S_IRWXO	//to give permissions.

all_roles *info;

sem_t *sem1; //File Read
char sem1name[] = "semaphore_1";

sem_t *v1s; //vaccine 1 space
char v1sn[] = "semaphore_2";
sem_t *v2s; //vaccine 2 space
char v2sn[] = "semaphore_3";

sem_t *v1f; //vaccine 1 full
char v1fn[] = "semaphore_4";
sem_t *v2f; //vaccine 2 full
char v2fn[] = "semaphore_5";

sem_t *vaccinesemaphore;
char vaccinesemaphore_name[] = "semaphore_6";

sem_t *citizensem;
char citsemname[] = "semaphore_7";

sem_t *citcritical;
char citcriticalname[] = "semaphore_8";

//-------------------------------------------------
int vaccinator(all_roles *info, int vaccinatornum);
int nurse_process(all_roles *info, int nursenum);
int citizen(all_roles *info, int citizennum);
void citizen_signalhandler(int signum);

/*WARNING -- Should be called in critical zone,
because there is some shared memory reading.

This function gets best index of citizen shared 
memory, Thats it. Returns -1 if there are no 
citizen left in shared memory.
*/
int get_best_oldest_citizen(all_roles *info);

/*WARNING -- Should be called in critical zone,
because there is some shared memory reading.

Returns the count of unvaccinated citizen
*/
int get_left_citizens(all_roles *info);
void endingsignal(int signum);
void exit_fun();
//-------------------------------------------------

int main(int argc, char **argv){

	signal(SIGUSR2,endingsignal);

	//All_roles, initialize...
		int i,j; //generic integers to do various things.
		if((info = malloc(sizeof(all_roles))) == NULL){
			//If error,
			perror("malloc");
			return 1;
		}

	//Get arguments parsed.
		if(parsearg(argc,argv,info) != 0){
			fprintf(stderr, "Error while parsing. Exiting.\n");
			return 1;
		}

	printf("Welcome to the GTU344 clinic. Number of citizens to vaccinate c=%d with t=%d doses.\n",info->citizens,info->vaccinatetimes);

	//Atexit initialization.
		if(atexit(exit_fun) == -1){
			perror("atexit");
			exit(errno);
		}

	//Signal Handler for SIGINT.
		struct sigaction sigint_sa;
		memset(&sigint_sa, 0, sizeof(sigint_sa));
		sigint_sa.sa_handler = &sigint_handler;
		sigaction(SIGINT,&sigint_sa,NULL);

	//Set SHARED MEMORIES

		//SHARED MEMORY.
			if(shm_unlink("sharedmem") == -1){
				if(errno != ENOENT) perror("sem_unlink");
			}

			if(((info->shared_fd = shm_open("sharedmem", O_RDWR | SEM_FLAGS, SEM_PERMS)) == -1)){
				perror("shm_open");
				return 1;
			}

			if(ftruncate(info->shared_fd,info->sizeofbuffer) == -1){
				perror("ftruncate");
				return 1;
			}

			info->sharedmem = (int* )mmap(NULL, info->sizeofbuffer * (sizeof(int)), PROT_READ | PROT_WRITE, MAP_SHARED,info->shared_fd,0);
			if(info->sharedmem == MAP_FAILED){
				perror("mmap");
				exit(1);
			}

		//CITIZEN INFO SHARED MEMORY.
			if(shm_unlink("citizenmem") == -1){
				if(errno != ENOENT) perror("sem_unlink");
			}

			if(((info->citizen_fd = shm_open("citizenmem", O_RDWR | SEM_FLAGS, SEM_PERMS)) == -1)){
				perror("shm_open");
				return 1;
			}

			if(ftruncate(info->citizen_fd,info->citizens) == -1){
				perror("ftruncate");
				return 1;
			}

			info->citizenmem = (cit_info* )mmap(NULL, info->citizens * (sizeof(cit_info)), PROT_READ | PROT_WRITE, MAP_SHARED,info->shared_fd,0);
			if(info->citizenmem == MAP_FAILED){
				perror("mmap");
				exit(1);
			}
		
	//Set semaphore(s).
		//----------------------------------------------------------------------
		if(sem_unlink(sem1name)){
			if(errno != ENOENT) perror("sem_unlink");
		}
		if((sem1 = sem_open(sem1name, SEM_FLAGS, SEM_PERMS, 1)) == SEM_FAILED){
			perror("sem_open");
			return 1;
		}
		//----------------------------------------------------------------------
		//////////
		if(sem_unlink(v1sn)){
			if(errno != ENOENT) perror("sem_unlink");
		}
		if((v1s = sem_open(v1sn, SEM_FLAGS, SEM_PERMS, info->sizeofbuffer)) == SEM_FAILED){
			perror("sem_open");
			return 1;
		}//////////
		if(sem_unlink(v2sn)){
			if(errno != ENOENT) perror("sem_unlink");
		}
		if((v2s = sem_open(v2sn, SEM_FLAGS, SEM_PERMS, info->sizeofbuffer)) == SEM_FAILED){
			perror("sem_open");
			return 1;
		}//////////
		if(sem_unlink(v1fn)){
			if(errno != ENOENT) perror("sem_unlink");
		}
		if((v1f = sem_open(v1fn, SEM_FLAGS, SEM_PERMS, 0)) == SEM_FAILED){
			perror("sem_open");
			return 1;
		}//////////
		if(sem_unlink(v2fn)){
			if(errno != ENOENT) perror("sem_unlink");
		}
		if((v2f = sem_open(v2fn, SEM_FLAGS, SEM_PERMS, 0)) == SEM_FAILED){
			perror("sem_open");
			return 1;
		}//////////
		if(sem_unlink(vaccinesemaphore_name)){
			if(errno != ENOENT) perror("sem_unlink");
		}
		if((vaccinesemaphore = sem_open(vaccinesemaphore_name, SEM_FLAGS, SEM_PERMS, 1)) == SEM_FAILED){
			perror("sem_open");
			return 1;
		}
		//////////
		//----------------------------------------------------------------------
		if(sem_unlink(citsemname)){
			if(errno != ENOENT) perror("sem_unlink");
		}
		if((citizensem = sem_open(citsemname, SEM_FLAGS, SEM_PERMS, 1)) == SEM_FAILED){
			perror("sem_open");
			return 1;
		}
		if(sem_unlink(citcriticalname)){
			if(errno != ENOENT) perror("sem_unlink");
		}
		if((citcritical = sem_open(citcriticalname, SEM_FLAGS, SEM_PERMS, 1)) == SEM_FAILED){
			perror("sem_open");
			return 1;
		}
		//----------------------------------------------------------------------

	//Citizen Starter:
		for(i=0 ; i<info->citizens ;i++){
			switch(j=fork()){
				case 0: //Child Process!
					//Place the citizen to the vaccination system.
					return citizen(info,i+1);
				case -1: //Error.
					perror("fork");
					return -1;
			}

			//Place created citizen info to shared memory of citizens, it is like a
			//book for vaccinators to keep track of citizens vaccination.
			sem_wait(citizensem);
			info->citizenmem[i] = givecitizenvalues(j,info->vaccinatetimes);
			sem_post(citizensem);
		}

	//Nurse Starter:
		if((info->inputfilefd = open(info->inputfilename,O_RDONLY | O_NONBLOCK)) == -1){
			perror("open");
			return 0;
		}

		for(i=0 ; i<info->nurses ;i++){
			switch(j=fork()){
				case 0: //Child Process!
					return nurse_process(info,i+1);
				case -1: //Error.
					perror("fork");
					return -1;
			}
		}
		
	//Vaccinator Starter:
		for(i=0 ; i<info->vaccinators ;i++){
			switch(j=fork()){
				case 0: //Child Process!
					return vaccinator(info,i+1);
				case -1: //Error.
					perror("fork");
					return -1;
			}
		}

	//Wait for children to finish.
		pid_t finished;
		int status = 0;
		while ((finished = wait(&status)) > 0)
		{/*Empty intented loop*/}

		munmap(info->sharedmem,info->sizeofbuffer*(sizeof(char)));
		if(close(info->inputfilefd) == -1){
			perror("close");
			return 1;
		}
		if(shm_unlink("sharedmem") == -1){
			perror("sem_unlink");
			return 1;
		}
		if(shm_unlink("citizenmem") == -1){
			perror("sem_unlink");
			return 1;
		}
		if(sem_unlink(sem1name)){
			perror("sem_unlink");
			return 1;
		}
		if(sem_unlink(v1sn)){
			perror("sem_unlink");
			return 1;
		}
		if(sem_unlink(v2sn)){
			perror("sem_unlink");
			return 1;
		}
		if(sem_unlink(v1fn)){
			perror("sem_unlink");
			return 1;
		}
		if(sem_unlink(v2fn)){
			perror("sem_unlink");
			return 1;
		}
		if(sem_unlink(vaccinesemaphore_name)){
			perror("sem_unlink");
			return 1;
		}
		if(sem_unlink(citsemname)){
			perror("sem_unlink");
			return 1;
		}
		if(sem_unlink(citcriticalname)){
			perror("sem_unlink");
			return 1;
		}
		free(info);

	return 0;
}

//-------------------------------------------------

void exit_fun(){
	//Free and unlink Everything.
	if(sem_close(sem1)){
		perror("sem_close");
	}
	if(sem_close(v1s)){
		perror("sem_close");
	}
	if(sem_close(v2s)){
		perror("sem_close");
	}
	if(sem_close(v1f)){
		perror("sem_close");
	}
	if(sem_close(v2f)){
		perror("sem_close");
	}
	if(sem_close(vaccinesemaphore)){
		perror("sem_close");
	}
	if(sem_close(citizensem)){
		perror("sem_close");
	}
	if(sem_close(citcritical)){
		perror("sem_close");
	}
	free(info);
}

//NURSE PROCESS
int nurse_process(all_roles *info, int nursenum){

	char c;
	int i,j;
	int vac1, vac2;

	while(1){

		//FILE-READ-----------------------
		sem_wait(sem1);//get semaphore to read

		//read only one int from file.
		if(readnext(info,&c) == -1){
			perror("read");
			return 1;
		}

		if(c == 'e'){
			printf("Nurse have carried all vaccines to the buffer, terminating.\n");
			sem_post(sem1);
			return 0;
		}

		sem_post(sem1); //lose semaphore to move on.
		//------------------------------------

		i = (int)c - 48; //Convert into integer.

		//SHAREDMEM WRITE---------------------
		if(i == 1) sem_wait(v1s);
		else sem_wait(v2s);
		sem_wait(vaccinesemaphore);

		//Put last vaccine to sharedmemory's empty space.
		for(j=0 ; j< info->sizeofbuffer ; j++){
			if(info->sharedmem[j] == 0){
				info->sharedmem[j] = i;
				break;
			}
		}

		sem_getvalue(v1f,&vac1);
		sem_getvalue(v2f,&vac2);

		if(i == 1) vac1++;
		else vac2++;

		printf("Nurse %d (pid=%d) has brought vaccine %d: the clinic has %d vaccine1 and %d vaccine2.\n"
			,nursenum,getpid(),i,vac1,vac2);

		sem_post(vaccinesemaphore);
		if(i == 1) sem_post(v1f);
		else sem_post(v2f);
		//------------------------------------
	}

	return 0;
}

int vaccinator(all_roles *info, int vaccinatornum){

	int i,indexnum; 
	int vac1,vac2;
	int vacctimes = 0;

	while(1){ //Run until all vaccinated.

		sem_wait(v1f); //See if vaccines are avaiable.
		sem_wait(v2f);
		sem_wait(vaccinesemaphore);

		//Since vaccines are avaiable, its time to remove them from buffer.
		for(i=0 ; i< info->sizeofbuffer ;i++){
			if(info->sharedmem[i] == 1){
				info->sharedmem[i] = 0;
				break;
			}
		}
		for(i=0 ; i< info->sizeofbuffer ;i++){
			if(info->sharedmem[i] == 2){
				info->sharedmem[i] = 0;
				break;
			}
		}
		//------------------------------------

		//Get a citizen semaphore.
		sem_wait(citizensem);
			sem_wait(citcritical);

			indexnum = get_best_oldest_citizen(info);
			if(indexnum == -1) return 0;
			sem_getvalue(v1f,&vac1);
			sem_getvalue(v2f,&vac2);

			printf("Vaccinator %d (pid=%d) is inviting citizen pid:%d to clinic.\n",vaccinatornum,getpid(),info->citizenmem[indexnum].pid);
			kill(info->citizenmem[indexnum].pid,SIGUSR1);
			vacctimes++;

			printf("The clinic has %d vaccine1 and %d vaccine2. Vaccinator %d vaccinated people %d times..\n",vac1,vac2,vaccinatornum,vacctimes);

			sem_post(citcritical);

		sem_post(vaccinesemaphore);
		sem_post(v1s);
		sem_post(v2s);
		
	}

	return 0;
}

int citizen(all_roles *info, int citizennum){

	sigset_t set;
	int i;
	int j=1;
	int c_index = citizennum -1;

	signal(SIGUSR1, citizen_signalhandler);

	sigemptyset(&set);
	if(sigaddset(&set, SIGUSR1) == -1) {
		perror("sigaddset");
		return 1;
	}

	while(1){
		sigwait(&set,&i);
		sem_wait(citcritical);

			printf("Citizen %d (pid=%d) vaccinated for the %dth time.\n",citizennum,getpid(),j);

			if(info->citizenmem[c_index].vaccinate_left == j){
				info->citizenmem[c_index].vaccinate_left = 0;
				printf("The citizen is LEAVING. Remaining citizens to vaccinate:%d\n",
				get_left_citizens(info));

				if(get_left_citizens(info) == 0){
					printf("All citizens have been vaccinated, ending all processes.\n");
					printf("Stay safe.\n");
					kill(0,SIGUSR2);
				}

				sem_post(citcritical);
				sem_post(citizensem);
				return 0;
			} 
		sem_post(citcritical);
		sem_post(citizensem);
		j++;
	}

	return 0;
}

void citizen_signalhandler(int signum){
	//Does nothing..
}

/*WARNING -- Should be called in critical zone,
because there is some shared memory reading.

This function gets best index of citizen shared 
memory, Thats it. Returns -1 if there are no 
citizen left in shared memory.
*/
int get_best_oldest_citizen(all_roles *info){

	int i;
	int oldest = 0;

	int most_vaccinate_left = 0;
	int index = 0;

	for(i=0 ; i< info->citizens ; i++){

		if(info->citizenmem[i].vaccinate_left > most_vaccinate_left){
			most_vaccinate_left = info->citizenmem[i].vaccinate_left;
			index = i;
		}
	}

	if(most_vaccinate_left == 0){
		//ALL CITIZENS VACCINATED.
		return -1;
	}

	for(i=0 ; i< info->citizens ; i++){
		if(info->citizenmem[i].vaccinate_left == most_vaccinate_left){
			if(info->citizenmem[i].pid > oldest){
				oldest = info->citizenmem[i].pid;
				index = i;
			}
		}
	}

	return index;
}

/*WARNING -- Should be called in critical zone,
because there is some shared memory reading.

Returns the count of unvaccinated citizen
*/
int get_left_citizens(all_roles *info){

	int total =0,i ;

	for(i=0 ; i<info->citizens ;i++){
		if(info->citizenmem[i].vaccinate_left != 0) total++;
	}

	return total;
}

void endingsignal(int signum){
	exit(0);
}
