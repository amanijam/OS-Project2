#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shell.h"
#include "shellmemory.h"

typedef struct PCB {
    int pid;
    int startMem;
    int scriptlen; // length = number of lines of code
    int pc; // current line to execute, offset from startMem
    struct PCB *next;
} PCB;

PCB *head = NULL; // global head of ready queue

int schedulerRunScript(int start, int len);

void append(int start, int len){
    if(head == NULL) {
        head = malloc(sizeof(PCB)); 
        head -> pid = 1;
        head -> startMem = start;
        head -> scriptlen = len;
        head -> pc = 0;
        head -> next = NULL;
    } else {
        PCB *current = head;
        int prevID = current -> pid;
        while (current -> next != NULL) {
            current = current -> next;
            prevID = current -> pid;
        } 
        PCB *new = current -> next;
        new -> pid = prevID++; // unique pid
        new -> startMem = start;
        new -> scriptlen = len;
        new -> pc = 0;
        new -> next = NULL;
    }
}

int schedulerRunScript(int start, int len){
    // add PCB for script to the tail of the readyqueue
    append(start, len);

    // run process at head of queue
    // send each line to the interpreter
    char *currCommand;
    int errCode = 0;
    for(int i = 0; i < head -> scriptlen; i++){
        currCommand = mem_get_value_from_position(head -> startMem + head -> pc);
        head -> pc = (head -> pc) + 1; // increment pc
        errCode = parseInput(currCommand); // from shell, which calls interpreter()
    }

    return errCode;
    
}