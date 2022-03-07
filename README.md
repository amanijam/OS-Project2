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
