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
    int pc; // current line to execute, offset from startMem
    struct PCB *next;
} PCB;

PCB *head = NULL; // global head of ready queue

char *policy;

// int schedulerExecFCFS(char *scripts[], int progNum, bool RR, bool SJF);
// void runQueue(int progNum, bool RR, bool SJF);
void setPolicy(char *p);
int schedulerStart(char *scripts[], int progNum);
void runQueue(int progNum);
void enqueue(int start, int len);
int dequeue();
void removeFromQueue(int pid);
void mergeSort(struct PCB** headRef);
void splitMiddle(struct PCB* head, struct PCB** aRef, struct PCB** bRef);
struct PCB* sortedMerge(struct PCB* a, struct PCB* b);

void setPolicy(char *p){
    policy = p;
}

int schedulerStart(char *scripts[], int progNum){
    int errCode;
    char line[1000];
    int lineCount, startPosition;
    char buff[10];

    if(strcmp(policy, "SJF") != 0) { // don't sort
        for(int i = 0; i < progNum; i++){
            errCode = 0;
            
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

            enqueue(startPosition, lineCount);
        }
    } else{ // SJF

    }
    
    runQueue(progNum);
}

void runQueue(int progNum){
    // run head prog, remove from mem, dequeue
    // if(strcmp(policy, "SJF") == 0){
    //     mergeSort(&head);
    // }

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
    } else if(strcmp(policy, "RR") == 0){
        int quantum = 2;      
        char *currCommand;
        PCB *currPCB;
        currPCB = head;
        while(head != NULL){
            for(int j = 0; j < quantum; j++){
                currCommand = mem_get_value_from_position(currPCB -> startMem + currPCB -> pc - 1);
                //printf("Running command: %s\n", currCommand);
                parseInput(currCommand); // from shell, which calls interpreter()
                currPCB -> pc = (currPCB -> pc) + 1; // increment pc
                if(currPCB -> pc > currPCB -> len) break;
            }

            // if we executed everything, remove code from shellmemory and remove from queue (clean up)
            if(currPCB -> pc > currPCB -> len){
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

void enqueue(int start, int len){
    if(head == NULL) {
        head = malloc(sizeof(PCB)); 
        head -> pid = 1;
        head -> startMem = start;
        head -> len = len;
        head -> pc = 1;
        head -> next = NULL;
    } else {
        PCB *current = head;
        int prevID = current -> pid;
        while (current -> next != NULL) {
            current = current -> next;
            prevID = current -> pid;
        } 
        PCB *new = malloc(sizeof(PCB));
        current -> next = new; 
        new -> pid = ++prevID; // unique pid
        new -> startMem = start;
        new -> len = len;
        new -> pc = 1;
        new -> next = NULL;
    }
}

// Remove PCB in head of queue and return it's pid
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

void mergeSort(struct PCB** headRef){
    PCB* head = *headRef;
    PCB* a;
    PCB* b;

    if(head == NULL || head -> next == NULL){
        return;
    }

    splitMiddle(head, &a, &b);

    mergeSort(&a);
    mergeSort(&b);

    *headRef = sortedMerge(a, b);
}

void splitMiddle(PCB* head, PCB** aRef, PCB** bRef){
    PCB* fast;
    PCB* slow;
    slow = head;
    fast = head -> next;

    while(fast != NULL && fast -> next != NULL){
        slow = slow -> next;
        fast = fast -> next -> next;
    }

    *aRef = head;
    *bRef = slow -> next;
}

struct PCB* sortedMerge(PCB* a, PCB* b){

    PCB* sort = NULL;
    if(a == NULL){
        return b;
    }
    else if(b == NULL){
        return a;
    }

    if(a -> len <= b -> len){
        sort = a;
        a -> next = sortedMerge(a -> next, b);
    }
    else{
        sort = b;
        b -> next = sortedMerge(b -> next, a);
    }

    return sort;
}
