#ifndef UNTITLED_DEFS_H
#define UNTITLED_DEFS_H

#define INSTRUCTION_SIZE_MAX 300
#define SERVER_NAME "fifo_sv"
#define CLIENT_NAME "fifo_cl"
#define SIZE 1024
#define MAX 300
#define NUM_PRIORITY_LEVELS 3 


typedef enum commandType {
    EXECUTE,
    STATUS,
    FINISHED
}commandType;

typedef enum Estado{
	A_EXECUTAR,
	EXECUTADO,
	POR_EXECUTAR
}Estado;

typedef enum executionType{
	PIPELINE,
	UNIQUE
}executionType;

typedef enum PriorityLevel{
    HIGH,
    MEDIUM,
    LOW
} PriorityLevel;

//Esqueleto de uma instrução
typedef struct Instruction{
	commandType type; //EXECUTE OR STATUS
	pid_t pid;        //Identificador do cliente
	char args[INSTRUCTION_SIZE_MAX];
	executionType isPipe;          //-u -> 0|-p ->1
	Estado estado;    //A_EXECUTAR, EXECUTADO, POR_EXECUTAR
	int id;           //Identificador da tarefa
	int time;         //Tempo esperado de execução da tarefa
	long executionTime;//Tempo de execução da tarefa
	PriorityLevel priority;
}Instruction;


//Array ou Queue de instruções
typedef struct InstructionQueue{
	Instruction* Instruction;
	struct InstructionQueue *next;
}InstructionQueue;




#endif //UNTITLED_DEFS_H