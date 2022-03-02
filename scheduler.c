#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int schedulerExecFCFS(char *scripts[], int progNum);
void runQueue(int progNum);

void enqueue(int start, int len){
    if(head == NULL) {
        head = malloc(sizeof(PCB)); 
        head -> pid = 1;
        head -> startMem = start;
        head -> len = len;
        head -> pc = 0;
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
        new -> pc = 0;
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

int schedulerExecFCFS(char *scripts[], int progNum){
    int errCode;
    char line[1000];
    int lineCount, startPosition;
    char buff[10];

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

    runQueue(progNum);
}

void runQueue(int progNum){
    // run head prog, remove from mem, dequeue
    char *currCommand;
    for(int i = 0; i < progNum; i++){        
        for(int j = 0; j < head -> len; j++){
            currCommand = mem_get_value_from_position(head -> startMem + head -> pc);
            head -> pc = (head -> pc) + 1; // increment pc
            parseInput(currCommand); // from shell, which calls interpreter()
        }

        //remove script course code from shellmemory and dequeue (clean up)
        for(int k = head -> startMem; k < head -> startMem + head -> len; k++){
            mem_remove_by_position(k);
        }

        dequeue();
    }
}