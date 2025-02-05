#include "includes.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_PRIORITY_LEVELS 3 // Number of priority levels


Instruction* currInstruction(InstructionQueue* instruction){
    return instruction->Instruction;
}



void assignRandomPriority(Instruction* instruction) {
    // Generate a random priority level between 0 and NUM_PRIORITY_LEVELS - 1
    instruction->priority = rand() % NUM_PRIORITY_LEVELS;
}



void schedulerFCFS(InstructionQueue** queue, Instruction* tarefa){
    InstructionQueue* queueFCFS = malloc(sizeof (struct InstructionQueue));
    if(queueFCFS == NULL){
        perror("Erro ao alocar memória para a lista fcfs");
        exit(EXIT_FAILURE);
    }
    queueFCFS->Instruction = tarefa;
    queueFCFS->next = NULL;

    if(*queue == NULL){
        *queue = queueFCFS;
    }
    else{
        InstructionQueue* curr = *queue;
        while(curr->next != NULL){
            curr = curr->next;
        }
        curr->next = queueFCFS;
    }
}


void schedulerSJF(InstructionQueue** queue, Instruction* instruction){ 

    InstructionQueue* queueSJF = malloc(sizeof (struct InstructionQueue));
    if(queueSJF == NULL){
        perror("Erro ao alocar memória para a lista sjf");
        exit(EXIT_FAILURE);
    }
    queueSJF->Instruction = instruction;
    queueSJF->next=NULL;

    if(*queue == NULL || (*queue)->Instruction->time >= instruction->time){
        queueSJF->next = *queue;
        *queue = queueSJF;
    }
    else{
        InstructionQueue *curr = *queue;
        while(curr->next != NULL && curr->next->Instruction->time < instruction->time){
            curr = curr->next;
        }
        queueSJF->next = curr->next;   
        curr->next = queueSJF;
    }
}

void schedulerPriority(InstructionQueue** queue, Instruction* instruction) {
    InstructionQueue* newNode = malloc(sizeof(struct InstructionQueue));
    if (newNode == NULL) {
        perror("Error allocating memory for priority queue node");
        exit(EXIT_FAILURE);
    }
    newNode->Instruction = instruction;
    newNode->next = NULL;

    if (*queue == NULL) {
        // If the queue is empty, insert the new node as the first element
        *queue = newNode;
    } else {
        // Traverse the queue to find the correct position based on priority
        InstructionQueue* prev = NULL;
        InstructionQueue* curr = *queue;

        while (curr != NULL && curr->Instruction->priority >= instruction->priority) {
            prev = curr;
            curr = curr->next;
        }

        // Insert the new node at the appropriate position
        if (prev == NULL) {
            // If the new node has the highest priority, insert it at the beginning
            newNode->next = *queue;
            *queue = newNode;
        } else {
            // Otherwise, insert it between prev and curr
            prev->next = newNode;
            newNode->next = curr;
        }
    }
}


void schedulerRandomizedPriority(InstructionQueue** queue, Instruction* instruction) {
    // Assign random priority to the instruction
    assignRandomPriority(instruction);

    // Insert the instruction into the queue based on its priority level
    // For simplicity, we'll insert at the end of the queue regardless of priority
    InstructionQueue* newNode = malloc(sizeof(struct InstructionQueue));
    if (newNode == NULL) {
        perror("Error allocating memory for scheduler node");
        exit(EXIT_FAILURE);
    }
    newNode->Instruction = instruction;
    newNode->next = NULL;

    if (*queue == NULL) {
        // If the queue is empty, insert the new node as the first element
        *queue = newNode;
    } else {
        // Traverse to the end of the queue
        InstructionQueue* curr = *queue;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        // Insert the new node at the end of the queue
        curr->next = newNode;
    }
}



void removeHeadQueue(InstructionQueue** queue){ 
    if(queue!=NULL){
        InstructionQueue* tmp = *queue;
        *queue = (*queue)->next;
        free(tmp);
    }
}


void selectorPolicy(InstructionQueue** queue, Instruction* tarefa, char* policy){
    if(strcmp(policy, "fcfs") == 0){
        schedulerFCFS(queue, tarefa);
    } else if (strcmp(policy, "sjf") == 0){
        schedulerSJF(queue, tarefa);
    }
    else if (strcmp(policy,"priority")==0){
        schedulerPriority(queue,tarefa);
    }
    else if (strcmp(policy,"priorityRandom")==0){
        schedulerRandomizedPriority(queue,tarefa);
    }
}


