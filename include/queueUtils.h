#ifndef UNTITLED_QUEUE_UTILS_H
#define UNTITLED_QUEUE_UTILS_H

#include "includes.h"
Instruction* currInstruction(InstructionQueue* instruction);
void schedulerFCFS(InstructionQueue** queue, Instruction* tarefa);
void add_Task_sjf(InstructionQueue** queue, Instruction* tarefa);
void removeHeadQueue(InstructionQueue** queue);
void selectorPolicy(InstructionQueue** queue, Instruction* tarefa, char* policy);


#endif




