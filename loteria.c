#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Estrutura interna para facilitar o controle na Loteria
typedef struct {
    int pid;
    int momento_criacao;
    int tempo_execucao;
    int tempo_restante;
    int bilhetes;
    int na_fila;      // 0 ou 1 (se já chegou no sistema)
    int concluido;    // 0 ou 1 (se já terminou)
    int ja_executou;  // Para calcular a latência
    int latencia;
    int tempo_conclusao;
} ProcLoteria;

// --- Lógica do Escalonador Loteria ---

void loteria(void *lista_ptr, int num_processos, int quantum, const char *arquivo_saida) {
    // Cast para o formato original lido do arquivo
    typedef struct {
        int momento_criacao;
        int pid;
        int tempo_execucao;
        int prioridade_bilhetes;
    } ProcBase;
    
    ProcBase *processos_base = (ProcBase*)lista_ptr;
    
    // Aloca nossa estrutura de controle interna
    ProcLoteria *procs = (ProcLoteria*)malloc(num_processos * sizeof(ProcLoteria));
    for (int i = 0; i < num_processos; i++) {
        procs[i].pid = processos_base[i].pid;
        procs[i].momento_criacao = processos_base[i].momento_criacao;
        procs[i].tempo_execucao = processos_base[i].tempo_execucao;
        procs[i].tempo_restante = processos_base[i].tempo_execucao;
        procs[i].bilhetes = 100 - processos_base[i].prioridade_bilhetes; // Invertendo a prioridade para bilhetes
        procs[i].na_fila = 0;
        procs[i].concluido = 0;
        procs[i].ja_executou = 0;
        procs[i].latencia = 0;
    }

    FILE *out = fopen(arquivo_saida, "w");
    srand(time(NULL)); // Inicializa o gerador de aleatórios
    
    int tempo_atual = 0;
    int concluidos = 0;
    int total_bilhetes_ativos = 0;

    printf("Iniciando Escalonamento por Loteria...\n");

    while (concluidos < num_processos) {
        // 1. Verifica quem chegou no sistema e adiciona os bilhetes na "piscina"
        for (int i = 0; i < num_processos; i++) {
            if (!procs[i].na_fila && procs[i].momento_criacao <= tempo_atual) {
                procs[i].na_fila = 1;
                total_bilhetes_ativos += procs[i].bilhetes;
            }
        }

        if (total_bilhetes_ativos > 0) {
            // 2. Realiza o sorteio
            int bilhete_sorteado = rand() % total_bilhetes_ativos;
            int soma_bilhetes = 0;
            int idx_vencedor = -1;

            // 3. Encontra quem é o dono do bilhete sorteado
            for (int i = 0; i < num_processos; i++) {
                if (procs[i].na_fila && !procs[i].concluido) {
                    soma_bilhetes += procs[i].bilhetes;
                    if (soma_bilhetes > bilhete_sorteado) {
                        idx_vencedor = i;
                        break;
                    }
                }
            }

            ProcLoteria *vencedor = &procs[idx_vencedor];

            // ---> CÁLCULO DA LATÊNCIA <---
            if (vencedor->ja_executou == 0) {
                vencedor->latencia = tempo_atual - vencedor->momento_criacao;
                vencedor->ja_executou = 1;
                printf(">>> ALERTA: PID %d ganhou o sorteio pela PRIMEIRA VEZ (Latencia: %dms) <<<\n", vencedor->pid, vencedor->latencia);
            }

            int tempo_rodar = (vencedor->tempo_restante < quantum) ? vencedor->tempo_restante : quantum;
            
            printf("Tempo %d: Sorteio %d (de %d). PID %d vai pra CPU (Restava %dms | Executando %dms)\n", 
                   tempo_atual, bilhete_sorteado, total_bilhetes_ativos, vencedor->pid, vencedor->tempo_restante, tempo_rodar);
            
            vencedor->tempo_restante -= tempo_rodar;
            tempo_atual += tempo_rodar;

            // 4. Se o processo terminou, removemos os bilhetes dele do total!
            if (vencedor->tempo_restante <= 0) {
                vencedor->concluido = 1;
                vencedor->tempo_conclusao = tempo_atual;
                total_bilhetes_ativos -= vencedor->bilhetes; // É aqui que tiramos os bilhetes permanentemente
                concluidos++;
            }
        } else {
            // Nenhum processo chegou ainda
            tempo_atual++; 
        }
    }

    // --- RELATÓRIO FINAL ---
    fprintf(out, "PID | Latencia | Tpronto | Tempo de Turnaround\n");
    for (int i = 0; i < num_processos; i++) {
        int turnaround = procs[i].tempo_conclusao - procs[i].momento_criacao;
        int tpronto = turnaround - procs[i].tempo_execucao;
        fprintf(out, "%d | %d | %d | %d\n", procs[i].pid, procs[i].latencia, tpronto, turnaround);
    }

    fclose(out);
    free(procs);
    printf("\nEscalonamento Loteria concluido com sucesso! Leia o arquivo %s\n", arquivo_saida);
}