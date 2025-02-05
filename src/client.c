#include "includes.h"

//TODO SCRIPTS
//TODO COMPARAÇÃO SCHEDULER
//STATUS NAO DIZ PROGRAMAS QUE ESTAO A CORRER





//void parserPriorityLevel();

void parseArgsInstruction(int argc, char *argv[],Instruction *instruction){
	//./client(argv(0)) commandType[EXECUTE OR STATUS](argv(1)) Time(argv(2)) isPipe[-u OR -p](argv(3)) [ARGS](argv[4]) 
	
		instruction->type = EXECUTE;
		instruction->pid = getpid();
		instruction->estado = A_EXECUTAR;
		instruction->time = atoi(argv[2]);
		instruction->executionTime = 0;
		instruction->id = -1;

} 


int main(int argc, char * argv[]){
	Instruction* instruction = malloc(sizeof(struct Instruction));
    pid_t pid = getpid();
	if (instruction == NULL) {
        perror("Erro ao alocar memória para a instrução...\n");
        return -1;
    }
    if(argc == 5 && (strcmp(argv[1],"execute")==0 || (strcmp(argv[1],"Execute")==0)|| (strcmp(argv[1],"EXECUTE")==0))){
	    parseArgsInstruction(argc,argv,instruction); //preencher a estrutura da instrucção

	    char *buf = strdup(argv[4]);
	    if (buf == NULL) {
	        perror("Erro ao alocar espaço para o parse");
	        exit(-1);
    	}

        //Remove as aspas e colocar um "\0" no fim da string...
        if (buf[0] == '"' && buf[strlen(buf) - 1] == '"') {
            buf[strlen(buf) - 1] = '\0';
            buf++;
        }

        //Copia o conteudo do buffer, ou seja, os argumentos a executar, para a nossa estrutura Instruction
        strcpy(instruction->args,buf);
        
        if(strcmp(argv[3],"-u")==0){
            instruction->isPipe = UNIQUE;
        } 
        else if(strcmp(argv[3],"-p")==0){
            instruction->isPipe= PIPELINE;
        }
         else{
        printf("CLIENT: invalid arguments\n");
        return -1;
    }
}

    else if(argc == 2 && strcmp(argv[1],"status")==0){
        instruction->type = STATUS;
        instruction->pid = pid;
    }
    else{
        printf("CLIENT: invalid arguments\n");
        return -1;
    }

    char fifoc_name[30];
    sprintf(fifoc_name,CLIENT_NAME "%d",pid);

    if(mkfifo(fifoc_name, 0666) == -1){
            perror("CLIENT: mkfifo server-client\n");
            return -1;
    }


    //ENVIA A ESTRUTURA PARA O SERVER
    int fd_fidocl = open(SERVER_NAME, O_WRONLY);
    if(fd_fidocl == -1){
        perror("CLIENT: Dind't open client-server fifo to write\n");
        return -1;
    }

    write(fd_fidocl,instruction,sizeof(struct Instruction));
    close(fd_fidocl);


    int fd_fifosv = open(fifoc_name,O_RDONLY);
        if(fd_fifosv == -1){
            perror("CLIENT: Dind't open server-client fifo to read\n");
            return -1;
        }

        char serverResponse[350];
        ssize_t bytes_read;
        while((bytes_read =read(fd_fifosv,&serverResponse,sizeof(&serverResponse)))>0){
            serverResponse[bytes_read] = '\0';
            printf("%s",serverResponse);
        }

        if (bytes_read < 0) {
            perror("CLIENT: Error reading from fifo server_client");
            return -1;
        }

    close(fd_fifosv);

    unlink(fifoc_name);

  	return 0;



}

