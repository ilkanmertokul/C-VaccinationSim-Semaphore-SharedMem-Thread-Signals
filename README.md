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
Welcome to the GTU344 clinic. Number of citizens to vaccinate c=8 with t=2 doses.<br />
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
...
Nurses have carried all vaccines to the buffer, terminating.<br />
...<br />
All citizens have been vaccinated .<br />
Vaccinator 1 (pid=3454) vaccinated 6 doses. Vaccinator 2 (pid=3455) vaccinated 10 doses. The clinic
is now closed. Stay healthy.
