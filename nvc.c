#include <unistd.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <signal.h>
#include <fcntl.h>
#include <semaphore.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nvc.h" //Custom header for this program.

/*SIG_INT handler.*/
void sigint_handler(int signum){
	printf("SIGINT handled...\n");
	exit(SIGINT);
}

/*Gets string to convert into int saves it on target and takes
a min value, if int is smaller than min value it returns -2, on 
error it returns -1, 0 on success.*/
int convert_to_int(char *string, int min_value, int *target){

	int i;
	//Check if it is actually integer.
	for(i=0; i<strlen(string) ;i++ ){
		if( !(string[i] >= '0' && string[i] <= '9')){
			fprintf(stderr, "This argument should be all integer, you entered %s.\n",string);
			return -1;
		}
	}

	//Check if it is greater than min_value
	if((*target = atoi(string)) < min_value){
		fprintf(stderr, "This argument  cannot be less than %d, you entered %s\n",min_value,string);
		return -2;
	}

	return 0;
}

/* Gets arguments for this midterm and parses it.
Returns -1 for bad arguments given
for not enough arguments given
for more than intended arguments given.
Else, returns 0 on succes.*/
int parsearg(int argc, char **argv, all_roles *info){

	//This is a flag array to check if we get all of those arguments correctly.
	int argcount[6] = {0,0,0,0,0,0};
	int chosen,i;

	//Loop to excract argument values.
	while((chosen = getopt(argc,argv,"n:v:c:b:t:i:")) != -1){

		switch(chosen){

			case 'n': 
				if(convert_to_int(optarg, MIN_NURSES, &i) != 0)
					return -1;
				info->nurses = i;
				argcount[0]++; 
				break;

			case 'v':
				if(convert_to_int(optarg, MIN_VACCIN, &i) != 0)
					return -1;
				info->vaccinators = i;
				argcount[1]++; 
				break;

			case 'c':
				if(convert_to_int(optarg, MIN_CITIZE, &i) != 0)
					return -1;
				info->citizens = i;
				argcount[2]++; 
				break;

			case 'b':
				if(convert_to_int(optarg, 0, &i) != 0)
					return -1;
				info->sizeofbuffer = i;
				argcount[3]++; 
				break;

			case 't':
				if(convert_to_int(optarg, MIN_SHOTSC, &i) != 0)
					return -1;
				info->vaccinatetimes = i;
				argcount[4]++; 
				break;

			case 'i':
				if(strlen(optarg) <= 0)
					return -1;
				info->inputfilename = optarg;
				argcount[5]++;
				break;

			default: //UNKNOWN ARG
				fprintf(stderr,"Incorrect argument input %c\n",(char)chosen);
				return -1;
		}
	}	

	//Check argcount if it is intented as we wanted.
	for(i=0 ; i< 6; i++){
		if(argcount[i] != 1){
			fprintf(stderr,"Arguments are not correct format, %d.\n",i);
			return -1;
		}
	}

	//We should check if b >=tc+1 condition is satisfied.
	if(info->sizeofbuffer < (info->vaccinatetimes*info->citizens)){
		fprintf(stderr,"Size of buffer is less than intended.\n");
		return -1;
	}

	return 0;
}

/*Useless print info function that used on developing the program*/
void printinfo(all_roles info){
	printf("Nurse count: %d\n",info.nurses);
	printf("Vaccinator count: %d\n",info.vaccinators);
	printf("Citizen count: %d\n", info.citizens);
	printf("Size of buffer: %d\n",info.sizeofbuffer);
	printf("Vaccinate times: %d\n",info.vaccinatetimes);
	printf("Input filename: %s\n",info.inputfilename);
}

/* Readnext reads next int from file, if it has reached its end
c gets 'e' char, otherwise, gets 1 or 2 as shots, if unknown char read, 
returns -1, returns 0 on success.*/
int readnext(all_roles *info, char *c){

	switch(read(info->inputfilefd,c,sizeof(char))){
		case 0: //END OF FILE
			*c = 'e';
			return 0;
		case -1: //ERROR
			*c = 'e';
			return -1;
	}
	//Else, return 0 because succesful.
	return 0;
}

/* This function creates new cit_info and assigns new information
inside it. Returns cit_info created.*/
cit_info givecitizenvalues(int pid,int vactimes){

	cit_info returner;

	returner.pid = pid;
	returner.vaccinate_left = vactimes;
	return returner;
}
