#include "includes.h"
//This parses the arguments in a string to a list
void parserArgs(char *args, char *list[]){
    char *buffer = strdup(args);
    char *token;
    int i=0;
    while ((token = strsep(&buffer, " ")) != NULL && i < INSTRUCTION_SIZE_MAX) {
        if(strlen(token) > 0){
        list[i] = strdup(token);
        if (list[i] == NULL) {
            return;
        }
        i++;
        }
    }
    list[i]=NULL;
    free(buffer);
}
//Simplifies the code
long calculateExecutionTime(struct timeval before, struct timeval after) {
    long execTime = (after.tv_sec - before.tv_sec) * 1000 + (after.tv_usec - before.tv_usec) / 1000;
    return execTime;
}

//Executa as instruções com comandos únicos
int execCommandUnique(Instruction *instruction, struct timeval before, char *folderDest, char *OutFile){
    char *args[INSTRUCTION_SIZE_MAX];
    parserArgs(instruction->args, args);
    char fdOutFile[100];
    char tmpBuff[400];
    struct timeval after;
    pid_t pid = fork();
    int status;
    switch (pid) {
        case -1:
            perror("Error execCommandUnique: Failed to create child process!\n");
            return -1;
        case 0: 
            pid_t inner_pid = fork(); 
            switch (inner_pid) {
                case -1:
                    perror("Error execCommandUnique: Failed to create inner child process!\n");
                    return -1;
                case 0: //CHILD PROCESS
                    sprintf(fdOutFile, "%s/%d_output_file.txt", folderDest, instruction->id);
                    int fdout = open(fdOutFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                    switch (fdout) {
                        case -1:
                            perror("Error execCommandUnique: Failed to open file for writing.\n");
                            return -1;
                    }
                    dup2(fdout, 1);
                    dup2(fdout, 2);
                    close(fdout);
                    execvp(args[0], args);
                    _exit(0); 
                default: //PARENT PROCESS
                    wait(&status); // Espera pelos filhos do fork interior, para evitar bloqueios no server
                    gettimeofday(&after, NULL); 
                    long execTime = calculateExecutionTime(before, after);

                    char buffer[256];
                    if (WIFEXITED(status)) {
                        sprintf(buffer, "Task %d finished\n", instruction->id);
                        write(1, buffer, strlen(buffer));
                    } else {
                        sprintf(buffer, "Task %d did not finish correctly\n", instruction->id);
                        write(2, buffer, strlen(buffer));
                    }
                    instruction->executionTime = execTime;
                    sprintf(tmpBuff, "Instrução executada com o id: %d e argumentos: %s em : %ld ms\n", instruction->id, instruction->args, instruction->executionTime);

                    int fd_FileOut = open(OutFile, O_WRONLY | O_CREAT | O_APPEND, 0666);
                    if (fd_FileOut == -1) {
                        perror("Error execCommandUnique: Erro ao abrir o ficheiro persistente para escrita\n");
                        return -1;
                    }
                    ssize_t bytes_written = write(fd_FileOut, &tmpBuff, strlen(tmpBuff));
                    if (bytes_written == -1) {
                        perror("Error execCommandUnique: Erro ao escrever no ficheiro.\n");
                        close(fd_FileOut);
                        return -1;
                    }
                    close(fd_FileOut);
                    _exit(0); 
            }
        default: 
            break;
    }
    instruction->type = FINISHED;

    int fd_svread = open(SERVER_NAME, O_WRONLY);
    if (fd_svread == -1) {
        perror("Error execCommandUnique: Nao consegui abrir o fifo para ler\n");
        return -1;
    }
    write(fd_svread, instruction, sizeof(struct Instruction));

    return 0;
}




//Execution of commands to use in the pipeline execution function
void myExec(char* args){
	char *argsExec[INSTRUCTION_SIZE_MAX];
	parserArgs(args,argsExec);
	execvp(argsExec[0],argsExec);
}


//Parses a string with args to a list
int parserPipeline(char *command, char *list[]){

    char *copy = command;
    char *token;
    int i=0;
    while ((token = strsep(&copy, "|")) != NULL && i < MAX) {
        list[i] = strdup(token);
        i++;
    }
    list[i]=NULL;
    free(copy);
    return i;
}



//Executes an instruction with pipeline commands
int executeTaskPIPELINE(Instruction *instruction, struct timeval before, char *folder, char *outputsFile){
    char *instructions[MAX];
    int n = parserPipeline(instruction->args,instructions);

    char outfilename[100];
    struct timeval after;

    sprintf(outfilename, "%s/%d_output.txt",folder, instruction->id);

    int fdout = open(outfilename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fdout == -1) {
        perror("Error executeTaskPIPELINE: Didn't open output file");
        return -1;
    }

    int pipes[n][2];
    for(int i = 0; i< n; i++){
        if(i == 0){
            pipe(pipes[i]);
            switch(fork()){
                case -1:
                    perror("Error executeTaskPIPELINE: Problem creating child process");
                    break;

                case 0 : 
                    close(pipes[i][0]);
                    dup2(pipes[i][1],1);
                    dup2(fdout,2);
                    myExec(instructions[i]);
                    break;
                default:
                    close(pipes[i][1]);
                    break;
            }
        }
        else if(i == n-1){
            switch(fork()){
                case 0 :
                    dup2(pipes[i-1][0],0);
                    close(pipes[i-1][0]);
                    dup2(fdout,1);
                    dup2(fdout,2);
                    close(fdout);
                    myExec(instructions[i]);
                    break;
                default :
                    close(pipes[i-1][0]);
                    close(fdout);
                    break;
            }
        }
        else{
            pipe(pipes[i]);
            switch(fork()){
                case -1:
                    perror("Error executeTaskPIPELINE: Problem creating child process");
                    break;
                case 0 :
                    close(pipes[i][0]);
                    dup2(pipes[i-1][0], 0);
                    close(pipes[i-1][0]);
                    dup2(pipes[i][1],1);
                    close(pipes[i][1]);
                    dup2(fdout,2);
                    myExec(instructions[i]);
                    break;
                default:
                    close(pipes[i-1][0]);
                    close(pipes[i][1]);
                    break;
            }
        }
    }
    


    int status;
    int flag=0;
    for(int i = 0; i<n;i++){
        wait(&status);
        if(WIFEXITED(status))
            continue;
        else{
            flag=1;
            char buffer[50];
            sprintf(buffer, "Task %d did not finish correctly\n", instruction->id);
            write(2, buffer, strlen(buffer));
            break;
        }
    }
    if(flag==0) {
        char buffer[50];
        sprintf(buffer, "Task %d finished \n", instruction->id);
        write(1, buffer, strlen(buffer));
    }

    gettimeofday(&after,NULL);

    long execTime = calculateExecutionTime(before, after);
    instruction->executionTime = execTime;

    int fd = open(outputsFile,O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd == -1) {
        perror("Error executeTaskPIPELINE: Didn't open outputs file");
        return -1;
    }

    char buff[350];
    sprintf(buff,"%d %s %ld\n", instruction->id,instruction->args,execTime);

    ssize_t bytes_written = write(fd,buff,strlen(buff));
    if (bytes_written == -1) {
        perror("Error executeTaskPIPELINE: Error writing to outputs file");
        close(fd);
        return -1;
    }

    close(fdout);
   
    //fazer a cena de mandar para o server
    instruction->type = FINISHED;

    int fd_sv_rd = open(SERVER_NAME, O_WRONLY);
    if (fd_sv_rd == -1) {
        perror("Error executeTaskPIPELINE: Dind't open client-server fifo to read\n");
        return -1;
    }
    write(fd_sv_rd,instruction,sizeof(struct Instruction));

    return 0;
}

//FUNCAO QUE TRATA DO STATUS
int execStatus(Instruction *instruction, InstructionQueue* queue, char *outputFile, char *fifoc_name){
	int fd_client = open(fifoc_name,O_WRONLY);
	if(fd_client == -1){
		perror("Error execStatus: Nao consegui abrir o fifo do cliente\n");
		return -1;
	}

	InstructionQueue* curr = queue;
	char message[350];
	sprintf(message,"EXECUTING: \n");
	write(fd_client,&message,strlen(message));

	if(curr == NULL){
		sprintf(message,"\nEXECUTING\n\n");
		write(fd_client,&message,strlen(message));
	}
	else{
		while(curr != NULL){
			Instruction* instruction = currInstruction(curr);
			if(instruction->estado == A_EXECUTAR){
				sprintf(message,"Processo com o pid: %d a executar o comando %s \n",instruction->id,instruction->args);
				write(fd_client,&message,strlen(message));
			}
			curr = curr -> next;
		}
		sprintf(message,"\n");
		write(fd_client,&message,strlen(message));

		curr = queue;
		while(curr != NULL){
			Instruction* instruction = currInstruction(curr);
			if(instruction->estado == POR_EXECUTAR){
				sprintf(message,"Processo com o pid: %d por executar com comando %s \n",instruction->id,instruction->args);
				write(fd_client,&message,strlen(message));
			}
			curr = curr -> next;
		}
		sprintf(message,"\n");
		write(fd_client,&message,strlen(message));
	}

	sprintf(message,"FINISHED: \n");
    write(fd_client,&message,strlen(message));

    int fd_outFile = open(outputFile,O_RDONLY);
    if (fd_outFile == -1) {
        perror("Error execStatus: Erro ao abrir o ficheiro");
        return -1;
    }

    ssize_t bytes_read;

    while ((bytes_read = read(fd_outFile, message, 350)) > 0) {
        message[bytes_read] = '\0'; 
        write(fd_client,&message,strlen(message));
    }
    if (bytes_read < 0) {
        perror("Error execStatus: Error reading from outputs file");
        return 1;
    }

    close(fd_outFile);
    close(fd_client);

    instruction -> type = FINISHED;

    int fd_svrd = open(SERVER_NAME,O_WRONLY);
    if (fd_svrd == -1) {
        perror("Error execStatus: Dind't open client-server fifo to read\n");
        return -1;
    }
    write(fd_svrd,instruction,sizeof(struct Instruction));

    return 0;

}
