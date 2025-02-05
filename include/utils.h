#ifndef UTILS_H
#define UTILS_H


void parserArgs(char *args, char *list[]);

int execCommandUnique(Instruction *instruction, struct timeval before, char *folderDest, char *OutFile);

void myExec(char* args);

int execStatus(Instruction *instruction, InstructionQueue* queue, char *outputFile, char *fifoc_name);

int executeTaskPIPELINE(Instruction *instruction, struct timeval before, char *folder, char *outputsFile);


#endif