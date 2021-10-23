
//-------------------------------------------------
// 		İLKAN MERT OKUL, 
//		ilkanmertokul@gmail.com 
//		github/ilkanmertokul
//-------------------------------------------------

#define MIN_NURSES 2
#define MIN_VACCIN 2
#define MIN_CITIZE 3
#define MIN_SHOTSC 1

typedef struct citizeninfo{

	int pid;
	int vaccinate_left;

}cit_info;

typedef struct roles{

	//It holds all information about roles,
	//as count of them.
	int nurses;
	int vaccinators;
	int citizens;	

	//Other information for roles.
	int sizeofbuffer;
	int vaccinatetimes;

	char* inputfilename;

	int shared_fd;
	int citizen_fd;
	int inputfilefd;

	//Shared Memories.
	int *sharedmem;
	cit_info *citizenmem;

}all_roles;

/* Gets arguments for this midterm and parses it.
Returns -1 for bad arguments given
for not enough arguments given
for more than intended arguments given.
Else, returns 0 on succes.*/
int parsearg(int argc, char **argv, all_roles *info);

/*Gets string to convert into int saves it on target and takes
a min value, if int is smaller than min value it returns -2, on 
error it returns -1, 0 on success.*/
int convert_to_int(char *string, int min_value, int *target);

/* Readnext reads next int from file, if it has reached its end
c gets 'e' char, otherwise, gets 1 or 2 as shots, if unknown char read, 
returns -1, returns 0 on success.*/
int readnext(all_roles *info, char *c);

/* This function creates new cit_info and assigns new information
inside it. Returns cit_info created.*/
cit_info givecitizenvalues(int pid,int vactimes);

/*Useless print info function that used on developing the program*/
void printinfo(all_roles info);

/*SIG_INT handler.*/
void sigint_handler(int signum);
