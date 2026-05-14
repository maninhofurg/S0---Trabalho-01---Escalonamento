#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Estrutura interna para o nó da Árvore Rubro-Negra
typedef struct Node {
    int pid;
    float vruntime;
    int tempo_restante;
    int momento_criacao;
    int tempo_total_original;
    int prioridade;
    int cor; // 0 para Preto, 1 para Vermelho
    int ja_executou; // Flag para latência
    int latencia;    
    struct Node *esq, *dir, *pai;
} Node;

// Estrutura global para estatísticas
typedef struct {
    int pid;
    int criacao;
    int conclusao;
    int exec_total;
    int latencia;    
} Estatistica;

Node *raiz = NULL;
Node *T_NIL = NULL;

// --- Funções da Árvore ---

Node* criar_no(int pid, int tempo, int criacao, int prioridade) {
    Node *novo = (Node*)malloc(sizeof(Node));
    novo->pid = pid;
    novo->vruntime = 0.0;
    novo->tempo_restante = tempo;
    novo->tempo_total_original = tempo;
    novo->momento_criacao = criacao;
    novo->prioridade = prioridade;
    novo->ja_executou = 0; 
    novo->latencia = 0;
    novo->pai = novo->esq = novo->dir = T_NIL;
    novo->cor = 1; 
    return novo;
}

// Encontra o processo com menor vruntime (mais à esquerda)
Node* minimo(Node *no) {
    while (no->esq != T_NIL) no = no->esq;
    return no;
}

// Remove o nó mínimo da árvore antes de ir pra CPU
void remover_no_minimo(Node **raiz_ptr, Node *z) {
    Node *x = z->dir;
    if (z->pai == T_NIL) {
        *raiz_ptr = x;
    } else if (z == z->pai->esq) {
        z->pai->esq = x;
    } else {
        z->pai->dir = x;
    }
    if (x != T_NIL) {
        x->pai = z->pai;
    }
    z->pai = T_NIL;
    z->esq = T_NIL;
    z->dir = T_NIL;
}

// ---> ALTERAÇÃO AQUI: Árvore agora usa o Desempate <---
void inserir_arvore(Node **raiz_ptr, Node *z) {
    Node *y = T_NIL;
    Node *x = *raiz_ptr;

    while (x != T_NIL) {
        y = x;
        // Se o vruntime for menor, vai pra esquerda.
        if (z->vruntime < x->vruntime) {
            x = x->esq;
        } 
        // REGRA DE DESEMPATE: vruntime igual, mas prioridade MENOR (ex: 01 ganha de 02), vai pra esquerda.
        else if (z->vruntime == x->vruntime && z->prioridade < x->prioridade) {
            x = x->esq;
        } 
        // Caso contrário, vai pra direita.
        else {
            x = x->dir;
        }
    }
    
    z->pai = y;
    if (y == T_NIL) {
        *raiz_ptr = z;
    } else if (z->vruntime < y->vruntime) {
        y->esq = z;
    } else if (z->vruntime == y->vruntime && z->prioridade < y->prioridade) {
        y->esq = z; // Aplica o desempate ao encaixar o nó
    } else {
        y->dir = z;
    }
}

// Imprime a "Fila de Prontos"
void imprimir_fila_prontos(Node *n) {
    if (n != T_NIL) {
        imprimir_fila_prontos(n->esq);
        printf("[PID %d | vr=%.1f] ", n->pid, n->vruntime);
        imprimir_fila_prontos(n->dir);
    }
}

// --- Lógica do Escalonador CFS ---

void cfs(void *lista_ptr, int num_processos, int quantum, const char *arquivo_saida) {
    typedef struct {
        int momento_criacao;
        int pid;
        int tempo_execucao;
        int prioridade_bilhetes;
    } ProcBase;
    
    ProcBase *processos = (ProcBase*)lista_ptr;
    
    if (T_NIL == NULL) {
        T_NIL = (Node*)malloc(sizeof(Node));
        T_NIL->cor = 0;
        T_NIL->esq = T_NIL->dir = T_NIL->pai = T_NIL;
    }
    raiz = T_NIL;

    FILE *out = fopen(arquivo_saida, "w");
    Estatistica *stats = malloc(num_processos * sizeof(Estatistica));
    int *inserido = calloc(num_processos, sizeof(int));
    
    int tempo_atual = 0;
    int processos_concluidos = 0;

    printf("Iniciando Escalonamento CFS...\n");

    while (processos_concluidos < num_processos) {
        for (int i = 0; i < num_processos; i++) {
            if (!inserido[i] && processos[i].momento_criacao <= tempo_atual) {
                inserir_arvore(&raiz, criar_no(processos[i].pid, processos[i].tempo_execucao, 
                                processos[i].momento_criacao, processos[i].prioridade_bilhetes));
                inserido[i] = 1;
            }
        }

        if (raiz != T_NIL) {
            Node *atual = minimo(raiz);
            remover_no_minimo(&raiz, atual);
            
            printf("\nFila de Prontos aguardando: ");
            if (raiz == T_NIL) printf("Vazia"); else imprimir_fila_prontos(raiz);
            printf("\n");
            
            // ---> CÁLCULO E IMPRESSÃO DA LATÊNCIA <---
            if (atual->ja_executou == 0) {
                atual->latencia = tempo_atual - atual->momento_criacao;
                atual->ja_executou = 1; 
                printf(">>> ALERTA: PID %d entrou na CPU pela PRIMEIRA VEZ (Latencia: %dms) <<<\n", atual->pid, atual->latencia);
            }

            int tempo_rodar = (atual->tempo_restante < quantum) ? atual->tempo_restante : quantum;
            
            printf("Tempo %d: PID %d na CPU (Restava %dms | Executando %dms)\n", 
                   tempo_atual, atual->pid, atual->tempo_restante, tempo_rodar);
            
            atual->tempo_restante -= tempo_rodar;
            tempo_atual += tempo_rodar;
            
            // Fórmula do vruntime (tempo * prioridade)
            atual->vruntime += (float)tempo_rodar * atual->prioridade;

            if (atual->tempo_restante <= 0) {
                stats[processos_concluidos].pid = atual->pid;
                stats[processos_concluidos].criacao = atual->momento_criacao;
                stats[processos_concluidos].conclusao = tempo_atual;
                stats[processos_concluidos].exec_total = atual->tempo_total_original;
                stats[processos_concluidos].latencia = atual->latencia; 
                processos_concluidos++;
                free(atual);
            } else {
                inserir_arvore(&raiz, atual);
            }
        } else {
            tempo_atual++; 
        }
    }

    fprintf(out, "PID | Latencia | Tpronto | Tempo de Turnaround\n");
    for (int i = 0; i < num_processos; i++) {
        int turnaround = stats[i].conclusao - stats[i].criacao;
        int tpronto = turnaround - stats[i].exec_total;
        fprintf(out, "%d | %d | %d | %d\n", stats[i].pid, stats[i].latencia, tpronto, turnaround);
    }

    fclose(out);
    free(stats);
    free(inserido);
    printf("\nEscalonamento CFS concluido com sucesso! Leia o arquivo %s\n", arquivo_saida);
}