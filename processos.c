#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Estrutura para armazenar os dados de cada processo conforme o arquivo de entrada
typedef struct {
    int momento_criacao;
    int pid;
    int tempo_execucao;
    int prioridade_bilhetes;
} Processo;

// Protótipos das funções que serão implementadas nos arquivos específicos
void alternancia_circular(Processo *processos, int num_processos, int quantum, const char *arquivo_saida);
void escalonamento_prioridade(Processo *processos, int num_processos, int quantum, const char *arquivo_saida);
void loteria(Processo *processos, int num_processos, int quantum, const char *arquivo_saida);
void cfs(Processo *processos, int num_processos, int quantum, const char *arquivo_saida);

// Função para definir o nome do arquivo de saída baseado no algoritmo detectado
const char* definir_saida(const char *algoritmo_nome) {
    if (strcmp(algoritmo_nome, "alternanciaCircular") == 0) return "resultados_alternancia.txt";
    if (strcmp(algoritmo_nome, "prioridade") == 0) return "resultados_prioridade.txt";
    if (strcmp(algoritmo_nome, "loteria") == 0) return "resultados_loteria.txt";
    if (strcmp(algoritmo_nome, "CFS") == 0) return "resultados_cfs.txt";
    return "resultados_saida.txt";
}

int main(int argc, char* argv[]) {
    // Verifica se o usuário passou o nome do arquivo como argumento
    if (argc < 2) {
        printf("Uso: %s <nome_do_arquivo_de_entrada>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo: %s\n", argv[1]);
        return 1;
    }

    char alg_nome[50];
    int quantum;

    // Lê a primeira linha: algoritmoDeEscalonamento|fraçãoDeCPU
    // Exemplo no arquivo: CFS|10 
    if (fscanf(file, "%[^|]|%d\n", alg_nome, &quantum) != 2) {
        printf("Erro na leitura do cabeçalho do arquivo.\n");
        fclose(file);
        return 1;
    }

    // Contagem de processos para alocação dinâmica
    int num_processos = 0;
    char buffer[256];
    long posicao_inicial = ftell(file);
    while (fgets(buffer, sizeof(buffer), file)) {
        if (strlen(buffer) > 1) num_processos++;
    }
    fseek(file, posicao_inicial, SEEK_SET);

    Processo *lista = (Processo *)malloc(num_processos * sizeof(Processo));
    if (lista == NULL) {
        printf("Erro de memória.\n");
        fclose(file);
        return 1;
    }

    // Lê os dados dos processos: momentoDeCriação|PID|tempoDeExecução|prioridade
    for (int i = 0; i < num_processos; i++) {
        fscanf(file, "%d|%d|%d|%d\n", 
               &lista[i].momento_criacao,
               &lista[i].pid,
               &lista[i].tempo_execucao,
               &lista[i].prioridade_bilhetes);
    }
    fclose(file);

    const char *saida = definir_saida(alg_nome);
    printf("Algoritmo detectado: %s | Quantum: %d\n", alg_nome, quantum);
    printf("Processando %d processos...\n", num_processos);

    // Seleção automática do algoritmo baseada na string do arquivo
    if (strcmp(alg_nome, "alternanciaCircular") == 0) {
        alternancia_circular(lista, num_processos, quantum, saida);
    } else if (strcmp(alg_nome, "prioridade") == 0) {
        escalonamento_prioridade(lista, num_processos, quantum, saida);
    } else if (strcmp(alg_nome, "loteria") == 0) {
        loteria(lista, num_processos, quantum, saida);
    } else if (strcmp(alg_nome, "CFS") == 0) {
        cfs(lista, num_processos, quantum, saida);
    } else {
        printf("Algoritmo '%s' não suportado.\n", alg_nome);
    }

    free(lista);
    return 0;
}

// =========================================================================
// STUBS TEMPORÁRIOS - APAGUE QUANDO FOR implementar AS FUNÇÕES DE CADA ALGORITMO
// =========================================================================

void alternancia_circular(Processo *processos, int num_processos, int quantum, const char *arquivo_saida) {
    printf("[Aviso] Executando funcao padrao: Alternancia Circular\n");
    // Código do seu colega virá aqui
}

void escalonamento_prioridade(Processo *processos, int num_processos, int quantum, const char *arquivo_saida) {
    printf("[Aviso] Executando funcao padrao: Prioridade\n");
    // Código do seu colega virá aqui
}

void loteria(Processo *processos, int num_processos, int quantum, const char *arquivo_saida) {
    printf("[Aviso] Executando funcao padrao: Loteria\n");
    // Código do seu colega virá aqui
}

