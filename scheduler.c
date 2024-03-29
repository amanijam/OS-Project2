#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "shell.h"
#include "shellmemory.h"
#include "interpreter.h"

typedef struct PCB {
    int pid;
    int startMem;
    int len; // length = number of lines of code
    int lenScore;
    int pc; // current line to execute
    struct PCB *next;
} PCB;

PCB *head = NULL; // global head of ready queue
char *policy;
int latestPid; // holds last used pid, in order to ensure all pid's are unique

void setPolicy(char *p);
int schedulerStart(char *scripts[], int progNum);
void runQueue(int progNum);
bool age();
int enqueue(int start, int len);
int prepend(int start, int len);
int insertInQueue(int start, int len);
int dequeue();
void removeFromQueue(int pid);

void setPolicy(char *p){
    policy = p;
}

int schedulerStart(char *scripts[], int progNum){
    latestPid = -1; // initialized latestPid

    char line[1000];
    int lineCount, startPosition;
    char buff[10];

    for(int i = 0; i < progNum; i++){        
        FILE *p = fopen(scripts[i],"rt"); 
        if(p == NULL) return badcommandFileDoesNotExist();
        
        lineCount = 0;
        startPosition; // contains position in memory of 1st line of code
        
        while(!feof(p)){
            fgets(line, 999, p);
            lineCount++;
            sprintf(buff, "%d", lineCount);
            if(lineCount == 1) startPosition = insert(buff, line);
            else insert(buff, line);

            memset(line, 0, sizeof(line));
        }
        fclose(p);

        if(strcmp(policy, "SJF") == 0 || strcmp(policy, "AGING") == 0) insertInQueue(startPosition, lineCount); // add PCB to an ordered queue in inc order by program length (lines)
        else enqueue(startPosition, lineCount); // add PCB to the end of queue (no ordering)
    }
    runQueue(progNum);
}

void runQueue(int progNum){
    if(strcmp(policy, "FCFS") == 0 || strcmp(policy, "SJF") == 0){
        char *currCommand;
        for(int i = 0; i < progNum; i++){
            // execute the entire program at the head
            for(int j = 0; j < head -> len; j++){
                currCommand = mem_get_value_from_position(head -> startMem + head -> pc - 1);
                head -> pc = (head -> pc) + 1; // increment pc
                parseInput(currCommand); // from shell, which calls interpreter()
            }

            //remove script course code from shellmemory and dequeue (clean up)
            for(int k = head -> startMem; k < head -> startMem + head -> len; k++){
                mem_remove_by_position(k);
            }
            dequeue();
        }
    } else if(strcmp(policy, "AGING") == 0){
        int timeSlice = 1;
        int startT = 0;
        int endT = 0;
        char *currCommand;
        bool stopAging = false;

        while(head != NULL){
            // execute one command
            currCommand = mem_get_value_from_position(head -> startMem + head -> pc - 1);
            head -> pc = (head -> pc) + 1; // increment pc
            parseInput(currCommand); // from shell, which calls interpreter()
            endT++; // increment time

            if(endT - startT >= timeSlice){ // reassess after a time slice has passed
                endT = 0; // restart "timer"
                if(!stopAging){
                    stopAging = age();
                    if(head -> pc > head -> len){
                        for(int k = head -> startMem; k < head -> startMem + head -> len; k++){
                            mem_remove_by_position(k);
                        }
                        dequeue();
                    }
                    // check if there is a prog with lenScore lower than that of the head
                    PCB *curr = head;
                    PCB *lowest = head;
                    PCB *lowestPrev = NULL;
                    PCB *lowestNext = lowest -> next;
                    // find lowest score in queue
                    while(curr -> next != NULL){
                        if(curr -> next -> lenScore < lowest -> lenScore){
                            if(curr != head) lowestPrev = curr;
                            lowest = curr -> next;
                            lowestNext = lowest -> next;
                        }
                        curr = curr -> next;
                    }
                    // if prog with lowest score is something other than head, we promote it
                    // promote: put current head at the end of the queue and prog with new lowest score at the head
                    if(lowest != head){
                        // find tail
                        PCB *tail;
                        curr = head;
                        while (curr -> next != NULL) curr = curr -> next;
                        tail = curr;
                        PCB *headNext = head -> next;

                        // place head at the end, after tail
                        head -> next = NULL;
                        tail -> next = head;
                        
                        // remove lowest from within the queue and place it at the head
                        if(lowestPrev != NULL){ // lowest was not right after head
                            lowestPrev -> next = lowestNext;
                            lowest -> next = headNext;
                        } // else lowest is already at head position 
                        
                        head = lowest;                
                    }
                } else{ // aging stopped
                    if(head -> pc > head -> len){
                        for(int k = head -> startMem; k < head -> startMem + head -> len; k++){
                            mem_remove_by_position(k);
                        }
                        dequeue();
                    }
                }  
            }
        }
        
    } else if(strcmp(policy, "RR") == 0){
        int quantum = 2;      
        char *currCommand;
        PCB *currPCB;
        currPCB = head;
        while(head != NULL){
            for(int j = 0; j < quantum; j++){
                currCommand = mem_get_value_from_position(currPCB -> startMem + currPCB -> pc - 1);
                parseInput(currCommand); // from shell, which calls interpreter()
                currPCB -> pc = (currPCB -> pc) + 1; // increment pc
                if(currPCB -> pc > currPCB -> len) break; // break if we've reached the end of the prog
            }

            
            if(currPCB -> pc > currPCB -> len){ // if we executed everything, remove code from shellmemory and remove from queue (clean up) and go to next prog
                for(int k = currPCB -> startMem; k < currPCB -> startMem + currPCB -> len; k++){
                    mem_remove_by_position(k);
                }
                int pidToRemove = currPCB -> pid;
                if(currPCB -> next == NULL){
                    currPCB = head;
                } else {
                    currPCB = currPCB -> next;
                }
                removeFromQueue(pidToRemove);
            } // else, go to next prog
            else if(currPCB -> next == NULL) currPCB = head;
            else currPCB = currPCB -> next;
        }
    }

}

// Decrease lenScore of all PCBs by 1, except for the head
// Length score cannot be lower than 0
// Return whether all scores are 0
bool age(){
    PCB *curr = head;
    bool allZeros = head -> lenScore == 0;
    while(curr -> next != NULL){
        curr = curr -> next;
        if(curr -> lenScore > 0){
            allZeros = false;
            curr -> lenScore = curr -> lenScore - 1;
        }
    }
    return allZeros;
}

// Add PCB at the end of the queue
// Return its pid
int enqueue(int start, int len){
    if(head == NULL) {
        head = malloc(sizeof(PCB)); 
        head -> pid = ++latestPid;
        head -> startMem = start;
        head -> len = len;
        head -> lenScore = len;
        head -> pc = 1;
        head -> next = NULL;
        return head -> pid;
    } else {
        PCB *current = head;
        while (current -> next != NULL) {
            current = current -> next;
        } 
        PCB *new = malloc(sizeof(PCB));
        current -> next = new; 
        new -> pid = ++latestPid; // unique pid
        new -> startMem = start;
        new -> len = len;
        new -> lenScore = len;
        new -> pc = 1;
        new -> next = NULL;
        return new -> pid;
    }
}

// Add PCB at the head of the queue
// Return its pid
int prepend(int start, int len){
    if(head == NULL){
        head = malloc(sizeof(PCB)); 
        head -> pid = ++latestPid;
        head -> startMem = start;
        head -> len = len;
        head -> lenScore = len;
        head -> pc = 1;
        head -> next = NULL;
    } else{
        PCB *new = malloc(sizeof(PCB));
        new -> pid = ++latestPid; // unique pid
        new -> startMem = start;
        new -> len = len;
        new -> lenScore = len;
        new -> pc = 1;
        new -> next = head;
        head = new;
    }
    return head -> pid;
}

// Add a PCB to an ordered queue (increasing by length)
// Return pid
int insertInQueue(int start, int len){
    if(head == NULL) return enqueue(start, len);
    else if(len < head -> len) return prepend(start, len);
    else{
        PCB *curr = head;
        while(curr -> next != NULL){  
            if(len < curr -> next -> len){
                PCB *new = malloc(sizeof(PCB));
                new -> pid = ++latestPid; 
                new -> startMem = start;
                new -> len = len;
                new -> lenScore = len;
                new -> pc = 1;
                PCB *next = curr -> next;
                curr -> next = new;
                new -> next = next;
                return new -> pid;
            }
            curr = curr -> next;
        }
        return enqueue(start, len); // if program wasn't placed in the queue, add it to the end
    }
}


// Remove PCB at the head of the queue
// Return its pid
int dequeue(){
    PCB **head_ptr = &head;

    //  Checks if queue is empty
    if (*head_ptr == NULL){
        return -1;
    }

    int retpid = (*head_ptr) -> pid; 
    PCB *next_pcb = (*head_ptr) -> next; 
    free(*head_ptr); 
    head = next_pcb;

    return retpid;
}

// Remove PCB with given pid from the queue (and free memory space)
void removeFromQueue(int pid){
    bool cont;
    PCB *currPCB;
    if(pid == head -> pid){
        dequeue();
        cont = false;
    } else{
        currPCB = head;
        cont = true;
    }

    while(cont){
        if(currPCB -> next == NULL){
            cont = false;
        } else if(currPCB -> next -> pid == pid){
            PCB *toRemove = currPCB -> next;
            if(currPCB -> next -> next == NULL){
                currPCB -> next = NULL;
            }else{
                currPCB -> next = currPCB -> next -> next;
            }
            free(toRemove);
            cont = false;
        } else{
            currPCB = currPCB -> next;
        }
    }
}