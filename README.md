# C-VaccinationSim-Semaphore-SharedMem-Thread-Signals
This project build for training Semaphores, Threads, Shared Memories in C. It is a vaccination simulator that has producer &amp; consumer problems too.

# How
./program –n 3 –v 2 –c 3 –b 11 –t 3 –i inputfilepath<br />
n >= 2: the number of nurses (integer)<br />
v >= 2: the number of vaccinators (integer)<br />
c >= 3: the number of citizens (integer)<br />
b >= tc+1: size of the buffer (integer),<br />
t >= 1: how many times each citizen must receive the 2 shots (integer),<br />
i: pathname of the input file<br /><br />
The input ASCII file (provided externally) will contain the numbers 1 and 2 in an arbitrary order
and t × c of each (her birinden t × c adet). The ‘1’ and ‘2’ characters in the file, represent
respectively the first and second vaccine shots that every citizen must receive. In other words, there
is exactly enough vaccines for all c citizens.
# Nurse
The nurse processes have a simple job. They read the input file, one character at a time (yes, I know
it’s inefficient) (in the usual order, from left to right, top to bottom, nothing fancy), and place the
vaccine they read (either the character ‘1’ or ‘2’) into the clinic/buffer (of which there will be one!).
Imagine this as bringing one vaccine from cold storage (the file) to the clinic (the buffer). The nurse
processes terminate after having carried all the vaccines into the clinic/buffer. If there is not enough
room in the buffer for vaccines, the nurses are supposed to wait.
# Vaccinator
The vaccinators are medical personel, and if there is at least one ‘1’, and one ‘2’ at the buffer/clinic,
then a vaccinator invites a citizen into the clinic, applies both vaccines/shots, by removing the two
characters from the buffer, and sends the citizen away. If there are not sufficient shots in the buffer
(or only ‘1’s or only ‘2’s), they wait until there is. The vaccinators terminate after having vaccinated
all citizens t times. The vaccinators must not prevent the maximal use of the buffer by the nurses.
# Citizen
Each citizen process waits for a vaccinator to call her inside the clinic. If that happens she gets
vaccinated, and waits to be called again for a total of t times. Once the citizen receives both shots t
times, she leaves and lives happily ever after.

## Expected Output
Welcome to the clinic. Number of citizens to vaccinate c=8 with t=2 doses.<br />
Nurse 1 (pid=3451) has brought vaccine 1: the clinic has 1 vaccine1 and 0 vaccine2.<br />
Nurse 3 (pid=3452) has brought vaccine 1: the clinic has 2 vaccine1 and 0 vaccine2.<br />
...<br />
Nurse 2 has brought vaccine 2: the clinic has 5 vaccine1 and 1 vaccine2.<br />
Vaccinator 1 (pid=3454) is inviting citizen pid=3456 to the clinic<br />
...<br />
Nurse 2 has brought vaccine 2: the clinic has 5 vaccine1 and 2 vaccine2.<br />
...<br />
Citizen 1 (pid=3456) is vaccinated for the 1 st time: the clinic has 6 vaccine1 and 1 vaccine2<br />
...<br />
Citizen 1 (pid=3456) is vaccinated for the 5 th time: the clinic has 5 vaccine1 and 3 vaccine2. The
citizen is leaving. Remaining citizens to vaccinate: 7<br />
...<br />
Nurses have carried all vaccines to the buffer, terminating.<br />
...<br />
All citizens have been vaccinated .<br />
Vaccinator 1 (pid=3454) vaccinated 6 doses. Vaccinator 2 (pid=3455) vaccinated 10 doses. The clinic
is now closed. Stay healthy.

# REPORT

i used three code files: nvc.h, nvc.c, main.c (and makefile
of course).<br />
nvc. files are basically helper function to our actions, does not use any shared
memory or semaphore itself. (nvc stands for nurse – vaccinator - citizen).
## nvc. files:
nvc.h file has really important structs to use: all_roles struct and cit_info struct.<br />
Cit_info: This struct used as shared memory type, to keep information about
citizens to vaccinate.<br />
All_roles: This struct appears only one instance on program, and it holds all
information we need, command line arguments that is parsed in parsearg(..)
stores its information in here to use later on.<br />
nvc.c file has the function codes that is declared in .h file, let’s see them one by
one:<br />

### 1. void sigint_handler(int signum):
This function is a handler function for
SIGINT signal, that happens on pressing ctrl+c, it gets ready to exit the
program.

### 2. int parsearg(int argc, char ** argv, all_roles * info)
Gets the argument
line without order, and parses its options, and saves it in target * info
struct. This way we can return int value to indicate error, it returns -1 on
error. Also checks for b > t * c+1 condition, and returns -1 if not enough
arguments given. Returns 0 on succes.

### 3. int convert_to_int(char * string, int min_value, int * target): 
This function gets a string and converts it into number, and saves on target.Returns -1
on standart error, returns -2 if the strings value is smaller than min_value,
but still saves on target, otherwise it returns 0;

### 4. int readnext(all_roles * info, char * c): 
Readnext reads next int from file, and stores in char c. If file reached its end, ‘e’ gets stored in c. Returns -1
on error.

### 5. cit_info givecitizenvalues(int pid,int vactimes): 
Creates new struct of
cit_info with given arguments and returns the struct.

## main.c
This file does most of the work, has all semaphores and shared memories. This
report explains every aspect of program as detailed as possible.<br />
### Shared Memories:
This program has 2 shared memories:<br />
a. sharedmem is for vaccines and it uses integer values only, and only for
1 and 2. It gets initialized in main, and gets unlinked and closed in main.<br />
b. citizenmem is for citizen information to keep track of vaccinations, uses
cit_info values only. It gets initialized in main, and gets unlinked and
closed in main.<br />
### Semaphores:
This program uses ,unfortunately, 8 named semaphores.<br />
a. “Semaphore_1” = is critical zone for nurses to read input file, so they
can not enter the zone same time.<br />
b. Semaphore 2, 3, 4, 5 ,6 used in producer/consumer problem, will
explained later.<br />
c. Semaphore 7,8 are used in citizen related critical works. To keep
connection between vaccinator and citizens.<br />
## Explanations of all functions of main.c:

### int main(int argc, char ** argv):
This function does most of the preperation work, forks all
citizens,nurses,vaccinators and waits them to finish, then closes everything and
terminates.<br />
Steps for main function:<br />
1. Parse argument line and store it in all_info struct.<br />
2. Handle SIGINT.<br />
3. Create shared memories and allocate them.<br />
4. Create semaphores and their initial values.<br />
5. Fork citizens, and while doing that, ready up the citizenmem values.<br />
6. Fork Nurses.<br />
7. Fork Vaccinators.<br />
8. Wait children to finish their work.<br />
9. Close / Unlink allocated memories.<br />
10. Terminate.<br />
### void exit_fun():
Used for closing allocated memories, to work with SIG_INT too.<br />
### int nurse_process(all_roles * info, int nursenum):
This function runs in a while loop to read input file’s data one by one, and write
the data one by one to shared memory sharedmem.<br />
Steps for nurse process (in a loop):<br />
1.Get sem1, read file input by one element, post sem1.<br />
2.Convert file element to integer.<br />
3.---Zone of Producer/Consumer---<br />
4.Get semaphore of producer<br />
5.Put element to sharedmem.<br />
6.Print out information about vaccine count.<br />
7.Post semaphore of producer.<br />
8.---Zone of Producer/Consumer---<br />
### int vaccinator(all_roles * info, int vaccinatornum):
This function consumes what nurses produced, then gives it to citizens one by
one.<br />
Steps for vaccinator (in a loop):<br />
1.---Zone of Producer/Consumer---<br />
2.Consume both vaccine1 and vaccine2.<br />
3.---Zone of Citizen/Vaccinator-----<br />
4.Get count of vaccines<br />
5.Get oldest process to vaccinate<br />
6.Vaccinate oldest with a SIGUSR1<br />
7.---Zone of Citizen/Vaccinator-----<br />
8.---Zone of Producer/Consumer---<br />

### int citizen(all_roles * info, int citizennum):
This function is citizen who waits a SIGUSR1 from vaccinator to get vaccinated,
in a loop that will iterate t times, which is given by argument count.<br />
Steps for citizen (in a loop):<br />

1.(before loop) set signal handlers.
2.Wait for signal
3.---Zone of Citizen/Vaccinator-----
4.Print out information about getting vaccinated
5.Update your left vaccinations.
6.---Zone of Citizen/Vaccinator-----

### int get_best_oldest_citizen(all_roles * info):
This function returns of citizenmem’s oldest not vaccinated citizen. Returns -1 if
there are no non-vaccinated citizens.
### int get_left_citizens(all_roles * info):
This functions returns the count of non-vaccinated citizens.












