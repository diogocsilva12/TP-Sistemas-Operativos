#include "includes.h"

int main(int argc, char *argv[]){

    // Variável usada para guardar o número do task atual
    int task_id = 0;

    // Verificação da sintaxe dos argumentos
	if(argc != 4 || (strcmp(argv[3],"fcfs")!=0 && strcmp(argv[3],"sjf")!=0)){
        char buffer[256];
        sprintf(buffer, "SERVER: Invalid Schedule policy\n");
        write(2, buffer, strlen(buffer));
        return -1;
    }

    // Criação do pipe para comunicação entre os clientes e o servidor
    if(mkfifo(SERVER_NAME, 0666) == -1){
            perror("SERVER: mkfifo client-server\n");
            return -1;
    }

	int fd_cl_rd = open(SERVER_NAME, O_RDONLY);
    if (fd_cl_rd == -1) {
        perror("SERVER: Dind't open client-server fifo to read\n");
        return -1;
    }

    int fd_cl_rw = open(SERVER_NAME, O_WRONLY);
    if (fd_cl_rw == -1) {
        perror("SERVER: Dind't open client-server fifo to write\n");
        return -1;
    }

    // Ficheiro de outputs. Aqui serão guardados os outputs do stdin após a execução
    char outputsfile[100];
    sprintf(outputsfile, "%s/outputs_file.txt",argv[1]);

    // Alocação de uma instrução e queue
	Instruction* instruction = malloc(sizeof(struct Instruction));
    InstructionQueue* queue = NULL;
    struct timeval before; // Começamos a contar o tempo de execução(antes)

    
    // Loop que roda enquanto o orchestrator ler instruções dos clientes
    while(read(fd_cl_rd,instruction, sizeof(struct Instruction))>0){
        // Se a opção execute for escolhida
        if(instruction->type == EXECUTE){
            gettimeofday(&before,NULL);
            instruction->id=task_id;
            task_id++; // incrementação do id da task
            char aux[20];
            sprintf(aux, "Task id: %d\n", instruction->id);
            instruction->estado=A_EXECUTAR;
            // selecionamos o escalonador(será inserido na queue também)
            selectorPolicy(&queue, instruction, argv[3]);
            char msg[30];
            sprintf(msg, "Added to queue!\n");  
            write(1, msg, strlen(msg));

            //Envio do id da tarefa para o cliente
            char fifoc_name[30];
    		sprintf(fifoc_name,CLIENT_NAME "%d",instruction->pid);

            // Enviamos a mensagem a dizer qual Task id está a ser executado
            int fd_cl = open(fifoc_name,O_WRONLY);
            if(fd_cl==-1){
                perror("SERVER: Didn't open server-client fifo to write\n");
                return -1;
            }
            write(fd_cl,&aux,strlen(aux));
            close(fd_cl);

            // Se a queue não estiver vazia podemos executar
            if(queue!=NULL){
                // Instrução atual
                instruction = currInstruction(queue);
                if(instruction->isPipe==UNIQUE){ // -u (execução única)
                    execCommandUnique(instruction, before, argv[1],outputsfile);
                }
                else if(instruction -> isPipe == PIPELINE){
                    executeTaskPIPELINE(instruction,before,argv[1],outputsfile);
                }
                // Após execução, removemos da queue a cabeca
                removeHeadQueue(&queue);
            }

        }

        else if(instruction->type == STATUS){

            char fifoc_name[30];
    		sprintf(fifoc_name,CLIENT_NAME "%d",instruction->pid);

            execStatus(instruction,queue,outputsfile, fifoc_name);

        }

        else if(instruction->type == FINISHED){
            if(queue!=NULL){
                instruction = currInstruction(queue);
                if(instruction->isPipe==UNIQUE){ // -u
                    execCommandUnique(instruction, before, argv[1],outputsfile);
                }
                removeHeadQueue(&queue);
            }
        }
    }
    // Libertamos memória da instrução, fechamos o pipe de leitura, e damos unlink ao pipe do servidor
    free(instruction);
    close(fd_cl_rd);
    unlink(SERVER_NAME);

    return 0;
}