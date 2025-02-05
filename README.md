Como usar o nosso escalonar de tarefas: 

1º Fazer make clean no terminal, para apagar os fifos e os ficheiros output

2º Dar make no terminal, para compilar os ficheiros bin

3º Fazer no terminal: ./orchestrator out num_args (fcfs/sjf)

Agora o que o servidor está a correr podemos enviar pedidos do cliente da seguinte forma: 

./cliente execute time (-u/-p) "args"

or 

./cliente status
