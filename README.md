# ECSE 427/COMP 310
# Assignment 2: Multi-process Scheduling
## Authors:
### Names & McGill IDs
* Amani Jammoul 260381641

* Annabelle Dion 260928845

## In this folder:
* Makefile

* shell.c, shell.h

* interpreter.c, interpreter.h

* shellmemory.c, shellmemory.h

* scheduler.c, scheduler.h

### (given test cases)
* prog1, prog2, prog3 (used as programs for testing)

* FCFS, FCFS_result

* SJF, SJF_result

* RR, RR_result

* AGING, AGING_result

* FileNameTest, FileNameTest_result 


## Notes:
Our code builds off of our A1 submission code. Everything added onto our previous submission is entirely our work.

## Brief Description:
For assignment 1 we used the given started code to begin with, so that logic remains. For this assignment, we added a scheduler.c file which is used for scheduling and multi-process execution. This file and its methods are called when the "exec" command is inputted. It is able to handle FCFS, SJF, RR, and AGING scheduling policies. As in the assignment description, SJF executes the programs in order from least number of lines to the most. RR rotates through the progams, executing 2 lines at a time (which are originally ordered as FCFS). AGING uses the SJF ordering and an aging mechanism (with timestamp = 1 line) to promote jobs when necessary. A large part of our scheduler relies on properly maintaining a PCB queue. The queue can be assesed from the global *head* PCB and each PCB has a *next* attribute which points to the following PCB in the queue.

## Assignment Description
![A2 - 1 of 7](https://github.com/amanijam/OS-Project2/assets/19826681/5aa9a2ff-4b6b-418d-ad15-352a05a132c8)
![A2 - 2 of 7](https://github.com/amanijam/OS-Project2/assets/19826681/e4b43db6-9c2d-4789-8d7c-033e00d1723f)
![A2 - 3 of 7](https://github.com/amanijam/OS-Project2/assets/19826681/96b890b6-57e9-45ee-9c81-d2a6dcc560c3)
![A2 - 4 of 7](https://github.com/amanijam/OS-Project2/assets/19826681/34ead1fc-4087-4f60-9eb8-0763615953b5)
![A2 - 5 of 7](https://github.com/amanijam/OS-Project2/assets/19826681/1354e812-585d-4c28-b2ad-fffafa783e10)
![A2 - 6 of 7](https://github.com/amanijam/OS-Project2/assets/19826681/45fa8847-08f3-4830-91e1-7c13e9c5b2cc)
![A2 - 7 of 7](https://github.com/amanijam/OS-Project2/assets/19826681/7d0d1a56-abe6-4638-a119-c1644e58c9b1)

