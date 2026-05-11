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
    struct Node *esq, *dir, *pai;
} Node;

// Estrutura global para estatísticas
typedef struct {
    int pid;
    int criacao;
    int conclusao;
    int exec_total;
} Estatistica;

Node *raiz = NULL;
Node *T_NIL = NULL;

// --- Funções da Árvore (Simplificadas para o trabalho) ---

Node* criar_no(int pid, int tempo, int criacao, int prioridade) {
    Node *novo = (Node*)malloc(sizeof(Node));
    novo->pid = pid;
    novo->vruntime = 0.0;
    novo->tempo_restante = tempo;
    novo->tempo_total_original = tempo;
    novo->momento_criacao = criacao;
    novo->prioridade = prioridade;
    novo->pai = novo->esq = novo->dir = T_NIL;
    novo->cor = 1; // Vermelho
    return novo;
}

// Função para encontrar o processo com menor vruntime (mais à esquerda)
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
    // Limpa os ponteiros do nó para ele poder ser reinserido sem bugs
    z->pai = T_NIL;
    z->esq = T_NIL;
    z->dir = T_NIL;
}

void inserir_arvore(Node **raiz_ptr, Node *z) {
    Node *y = T_NIL;
    Node *x = *raiz_ptr;

    while (x != T_NIL) {
        y = x;
        if (z->vruntime < x->vruntime) x = x->esq;
        else x = x->dir;
    }
    z->pai = y;
    if (y == T_NIL) *raiz_ptr = z;
    else if (z->vruntime < y->vruntime) y->esq = z;
    else y->dir = z;
}

// FUNÇÃO NOVA: Imprime a "Fila de Prontos" caminhando pela árvore em ordem
void imprimir_fila_prontos(Node *n) {
    if (n != T_NIL) {
        imprimir_fila_prontos(n->esq); // Vai o máximo pra esquerda
        printf("[PID %d | vr=%.1f] ", n->pid, n->vruntime); // Imprime
        imprimir_fila_prontos(n->dir); // Vai pra direita
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
    
    // Inicialização segura do T_NIL (Nó Nulo da Árvore Rubro-Negra)
    if (T_NIL == NULL) {
        T_NIL = (Node*)malloc(sizeof(Node));
        T_NIL->cor = 0;
        T_NIL->esq = T_NIL->dir = T_NIL->pai = T_NIL;
    }
    raiz = T_NIL;

    FILE *out = fopen(arquivo_saida, "w");
    Estatistica *stats = malloc(num_processos * sizeof(Estatistica));
    
    // ARRAY: Mantém o registro de quem já entrou na fila de prontos
    int *inserido = calloc(num_processos, sizeof(int));
    
    int tempo_atual = 0;
    int processos_concluidos = 0;

    printf("Iniciando Escalonamento CFS...\n");

    while (processos_concluidos < num_processos) {
        // Puxa todos os processos criados <= tempo_atual que ainda não entraram
        for (int i = 0; i < num_processos; i++) {
            if (!inserido[i] && processos[i].momento_criacao <= tempo_atual) {
                inserir_arvore(&raiz, criar_no(processos[i].pid, processos[i].tempo_execucao, 
                                processos[i].momento_criacao, processos[i].prioridade_bilhetes));
                inserido[i] = 1; // Marca como inserido
            }
        }

        if (raiz != T_NIL) {
            // Seleciona o nó mais à esquerda
            Node *atual = minimo(raiz);
            
            // Tira ele da árvore temporariamente
            remover_no_minimo(&raiz, atual);
            
            // IMPRIME A FILA DE PRONTOS AQUI
            printf("\nFila de Prontos aguardando: ");
            if (raiz == T_NIL) {
                printf("Vazia");
            } else {
                imprimir_fila_prontos(raiz);
            }
            printf("\n");
            
            int tempo_rodar = (atual->tempo_restante < quantum) ? atual->tempo_restante : quantum;
            
            printf("Tempo %d: PID %d na CPU (Restava %dms | Executando %dms)\n", 
                   tempo_atual, atual->pid, atual->tempo_restante, tempo_rodar);
            
            atual->tempo_restante -= tempo_rodar;
            tempo_atual += tempo_rodar;
            
            // A prioridade age como peso. Processos com prioridade maior ganham vruntime mais devagar.
            atual->vruntime += (float)tempo_rodar * (100.0 / atual->prioridade);

            if (atual->tempo_restante <= 0) {
                // Fim de vida do processo
                stats[processos_concluidos].pid = atual->pid;
                stats[processos_concluidos].criacao = atual->momento_criacao;
                stats[processos_concluidos].conclusao = tempo_atual;
                stats[processos_concluidos].exec_total = atual->tempo_total_original;
                processos_concluidos++;
                free(atual);
            } else {
                // Volta para a fila de prontos com o vruntime atualizado
                inserir_arvore(&raiz, atual);
            }
        } else {
            // Se a árvore está vazia, o tempo passa até que alguém chegue
            tempo_atual++; 
        }
    }

    // Calcula os relatórios conforme os slides
    fprintf(out, "PID | Tempo de Turnaround | Tempo em Pronto (Tpronto)\n");
    for (int i = 0; i < num_processos; i++) {
        int turnaround = stats[i].conclusao - stats[i].criacao;
        int tpronto = turnaround - stats[i].exec_total;
        fprintf(out, "%d | %d | %d\n", stats[i].pid, turnaround, tpronto);
    }

    fclose(out);
    free(stats);
    free(inserido);
    printf("\nEscalonamento CFS concluido com sucesso! Leia o arquivo %s\n", arquivo_saida);
}