=============================================================================
TRABALHO 01: SISTEMAS OPERACIONAIS - ESCALONADOR DE PROCESSOS (TURMA A)
=============================================================================

Integrantes do Grupo:
1. Alessandro Goldas da Cruz (Matrícula: [Sua Matrícula])
2. [Nome do Colega 2] (Matrícula: [Matrícula])
3. [Nome do Colega 3] (Matrícula: [Matrícula])
4. [Nome do Colega 4] (Matrícula: [Matrícula])

-----------------------------------------------------------------------------
1. DESCRIÇÃO DO PROJETO
-----------------------------------------------------------------------------
Este projeto implementa um simulador de escalonamento preemptivo de processos, 
desenvolvido em linguagem C. O orquestrador principal carrega a fila de 
processos a partir de um arquivo de texto e simula a execução na CPU utilizando 
um dos quatro algoritmos implementados pelo grupo:
- Alternância Circular (Round-Robin)
- Prioridade
- Loteria
- CFS (Completely Fair Scheduler)

Ao final da execução, o sistema calcula e exporta as métricas de desempenho 
(Latência, Tempo em Pronto/Tpronto e Tempo de Turnaround) para cada processo.

-----------------------------------------------------------------------------
2. COMO COMPILAR O CÓDIGO
-----------------------------------------------------------------------------
Para compilar o projeto, certifique-se de que todos os arquivos fonte (.c) 
estão no mesmo diretório. Abra o terminal e execute o comando do GCC 
linkando todos os algoritmos ao arquivo principal:

gcc processos.c alternancia_circular.c prioridade.c loteria.c cfs.c -o processos

Isso gerará um arquivo executável chamado "processos".

-----------------------------------------------------------------------------
3. COMO EXECUTAR
-----------------------------------------------------------------------------
O programa recebe o arquivo de entrada via argumento de linha de comando. 
Para rodar o simulador, execute:

./processos <nome_do_arquivo_de_entrada.txt>

Exemplo prático:
./processos entradaEscalonador.txt

-----------------------------------------------------------------------------
4. FORMATO DO ARQUIVO DE ENTRADA
-----------------------------------------------------------------------------
O arquivo de texto fornecido na execução deve seguir rigorosamente a 
seguinte estrutura, separada por pipe (|):

[Linha 1]: algoritmoDeEscalonamento|fraçãoDeCPU (quantum)
[Linha 2 em diante]: momentoDeCriação|PID|tempoDeExecução|prioridade_ou_bilhetes

* Algoritmos suportados na linha 1: alternanciaCircular, prioridade, loteria, CFS.

-----------------------------------------------------------------------------
5. RESULTADOS E SAÍDAS
-----------------------------------------------------------------------------
Durante a execução, o terminal exibirá a troca de contexto da CPU em 
tempo real e alertará sobre a latência inicial de cada processo.

Ao finalizar, o programa gerará um arquivo de texto automático correspondente 
ao algoritmo executado (ex: "resultados_cfs.txt", "resultados_prioridade.txt").
Neste arquivo de saída, você encontrará a tabela final contendo:
- PID do processo
- Latência (Tempo até o primeiro atendimento)
- Tpronto (Tempo total de espera na fila de prontos)
- Tempo de Turnaround (Tempo total de vida no sistema)
