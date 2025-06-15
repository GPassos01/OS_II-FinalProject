#include "fat.h"

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "ds.h"

#define SUPER 0
#define TABLE 2
#define DIR 1

#define SIZE 1024

// the superblock
#define MAGIC_N 0xAC0010DE
typedef struct {
    int magic;
    int number_blocks;
    int n_fat_blocks;
    char empty[BLOCK_SIZE - 3 * sizeof(int)];
} super;

super sb;

// item
#define MAX_LETTERS 6
#define OK 1
#define NON_OK 0
typedef struct {
    unsigned char used;
    char name[MAX_LETTERS + 1];
    unsigned int length;
    unsigned int first;
} dir_item;

#define N_ITEMS (BLOCK_SIZE / sizeof(dir_item))
dir_item dir[N_ITEMS];

// table
#define FREE 0
#define EOFF 1
#define BUSY 2
unsigned int *fat;

int mountState = 0;

int fat_format() {
    // Se estiver montado, não formatar
    if (mountState) {
        return -1;
    }

    // Inicializa o superbloco
    sb.magic = MAGIC_N;
    sb.number_blocks = ds_size();
    // Calcula quantos blocos são necessários para armazenar a FAT
    sb.n_fat_blocks = ceil((float)(sb.number_blocks * sizeof(unsigned int)) / BLOCK_SIZE);
    memset(sb.empty, 0, sizeof(sb.empty));

    // Escreve o superbloco no disco (bloco 0)
    ds_write(SUPER, &sb);

    // Inicializa o diretório vazio
    memset(dir, 0, sizeof(dir));

    // Escreve o diretório no disco (bloco 1)
    ds_write(DIR, dir);

    // Aloca memória para a FAT
    fat = (unsigned int *)malloc(sb.number_blocks * sizeof(unsigned int));
    if (!fat) {
        return -1;
    }

    // Inicializa a FAT com blocos livres
    for (int i = 0; i < sb.number_blocks; i++) {
        fat[i] = FREE;
    }

    // Marca os blocos do superbloco, diretório e FAT como ocupados
    fat[SUPER] = BUSY;  // Bloco 0: Superbloco
    fat[DIR] = BUSY;    // Bloco 1: Diretório
    for (int i = TABLE; i < TABLE + sb.n_fat_blocks; i++) {
        fat[i] = BUSY;  // Blocos da FAT
    }

    // Escreve a FAT no disco
    for (int i = 0; i < sb.n_fat_blocks; i++) {
        ds_write(TABLE + i, ((char *)fat) + (i * BLOCK_SIZE));
    }

    free(fat);
    fat = NULL;

    return 0;
}

void fat_debug() {
    printf("superblock:\n");

    // Verifica magic number
    if (sb.magic == MAGIC_N) {
        printf("\tmagic is ok\n");
    } else {
        printf("\tmagic is not ok\n");
    }

    printf("\t%d blocks\n", sb.number_blocks);
    printf("\t%d block fat\n", sb.n_fat_blocks);

    // Lista os arquivos
    for (int i = 0; i < N_ITEMS; i++) {
        if (dir[i].used) {
            printf("File \"%s\":\n", dir[i].name);
            printf("\tsize: %d bytes\n", dir[i].length);

            // Lista os blocos do arquivo
            printf("\tBlocks: ");
            int bloco = dir[i].first;
            while (bloco != EOFF && bloco != FREE) {
                printf("%d ", bloco);
                bloco = fat[bloco];
            }
            printf("\n");
        }
    }
}

int fat_mount() {
    // Se já estiver montado, desmonta primeiro
    if (mountState) {
        free(fat);
        fat = NULL;
        mountState = 0;
    }

    // Lê o superbloco
    ds_read(SUPER, &sb);

    // Verifica se é um sistema de arquivos válido
    if (sb.magic != MAGIC_N) {
        return -1;
    }

    // Verifica se o número de blocos da FAT é válido
    int expected_fat_blocks = ceil((float)(sb.number_blocks * sizeof(unsigned int)) / BLOCK_SIZE);
    if (sb.n_fat_blocks != expected_fat_blocks) {
        return -1;
    }

    // Lê o diretório
    ds_read(DIR, dir);

    // Aloca memória para a FAT
    fat = (unsigned int *)malloc(sb.number_blocks * sizeof(unsigned int));
    if (!fat) {
        return -1;
    }

    // Lê a FAT do disco
    for (int i = 0; i < sb.n_fat_blocks; i++) {
        ds_read(TABLE + i, ((char *)fat) + (i * BLOCK_SIZE));
    }

    // Sistema montado com sucesso
    mountState = 1;
    return 0;
}

int fat_create(char *name) {
    // Checar se o sistema de arquivos está montado
    if (!mountState) {
        fprintf(stderr, "ERRO: Sistema de arquivos nao esta montado.\n");
        return -1;
    }

    // Validar o nome do arquivo
    // Checar se o nome é muito grande (MAX_LETTERS + 1 para o '\0')
    if (strlen(name) > MAX_LETTERS) {
        fprintf(stderr, "ERRO: Nome do arquivo '%s' eh muito grande. Maximo de %d caracteres.\n", name, MAX_LETTERS);
        return -1;
    }

    // Checar se já existe um arquivo com o mesmo nome
    for (int i = 0; i < N_ITEMS; i++) {
        if (dir[i].used && strcmp(dir[i].name, name) == 0) {
            fprintf(stderr, "ERRO: Arquivo '%s' ja existe.\n", name);
            return -1;
        }
    }

    // Encontrar um espaço livre no diretório
    int indiceLivre = -1;
    for (int i = 0; i < N_ITEMS; i++) {
        if (!dir[i].used) {  // Se dir[i].used == 0, o item não está em uso
            indiceLivre = i;
            break;
        }
    }

    if (indiceLivre == -1) {
        fprintf(stderr, "ERRO: Nenhuma entrada livre.\n");
        return -1;
    }

    // Popular a nova entrada do diretório na RAM
    dir[indiceLivre].used = 1;                          // Marcar como usado
    strncpy(dir[indiceLivre].name, name, MAX_LETTERS);  // Copiar o nome do arquivo
    dir[indiceLivre].name[MAX_LETTERS] = '\0';          // Garantir que o nome esteja terminado com '\0'
    dir[indiceLivre].length = 0;                        // Arquivo vazio, então o tamanho é 0
    dir[indiceLivre].first = FREE;                      // Arquivo vazio, nenhum bloco alocado ainda

    // Escrever o diretório atualizado no disco
    ds_write(DIR, (char *)dir);  // Cast para char* porque ds_write espera um ponteiro para char

    return 0;  // Sucesso
}

int fat_delete(char *name) {
    // Checar se o sistema de arquivos está montado
    if (!mountState) {
        fprintf(stderr, "ERRO: Sistema de arquivos nao esta montado.\n");
        return -1;
    }

    // Encontrar o arquivo no diretório
    int indiceArquivo = -1;
    for (int i = 0; i < N_ITEMS; i++) {
        if (dir[i].used && strcmp(dir[i].name, name) == 0) {
            indiceArquivo = i;
            break;
        }
    }

    if (indiceArquivo == -1) {
        fprintf(stderr, "ERRO: Arquivo '%s' nao encontrado.\n", name);
        return -1;
    }

    // Liberar blocos de dados associados na FAT
    unsigned int blocoAtual = dir[indiceArquivo].first;
    unsigned int proximoBloco;

    while (blocoAtual != FREE && blocoAtual != EOFF) {  // Continua até achar um bloco livre (0) ou EOFF (fim da lista encadeada de blocos: 1)
        proximoBloco = fat[blocoAtual];                 // Obter o próximo bloco na cadeia
        fat[blocoAtual] = FREE;                         // Marcar o bloco atual como FREE na FAT
        blocoAtual = proximoBloco;                      // Mover para o próximo bloco
    }

    // Limpar a entrada do diretório
    dir[indiceArquivo].used = 0;                                          // Marcar como livre
    memset(dir[indiceArquivo].name, 0, sizeof(dir[indiceArquivo].name));  // Limpar nome
    dir[indiceArquivo].length = 0;                                        // Redefinir o tamanho
    dir[indiceArquivo].first = 0;                                         // Redefinir o ponteiro do primeiro bloco

    // Escrever a FAT atualizada de volta no disco
    for (int i = 0; i < sb.n_fat_blocks; i++) {
        ds_write(TABLE + i, ((char *)fat) + (i * BLOCK_SIZE));  // Escreve cada bloco do array fat para seu bloco TABLE + 1 correspondente
    }

    // Escrever o diretório atualizado de volta no disco
    ds_write(DIR, (char *)dir);

    return 0;  // Sucesso
}

int fat_getsize(char *name) {
    // Checar se o sistema de arquivos está montado
    if (!mountState) {
        fprintf(stderr, "ERRO: Sistema de arquivos nao esta montado.\n");
        return -1;
    }

    // Encontrar o arquivo no diretório
    int indiceArquivo = -1;
    for (int i = 0; i < N_ITEMS; i++) {
        if (dir[i].used && strcmp(dir[i].name, name) == 0) {
            indiceArquivo = i;
            break;
        }
    }

    if (indiceArquivo == -1) {
        fprintf(stderr, "ERRO: Arquivo '%s' nao encontrado.\n", name);
        return -1;
    }

    // Retornar o tamanho armazenado na entrada do diretório
    return dir[indiceArquivo].length;  // O campo length armazena diretamente o tamanho do arquivo em bytes
}

// Retorna a quantidade de caracteres lidos
int fat_read(char *name, char *buff, int length, int offset) {
    // Checar se o sistema de arquivos está montado
    if (!mountState) {
        fprintf(stderr, "ERRO: Sistema de arquivos nao esta montado.\n");
        return -1;
    }

    // Encontrar o arquivo no diretório
    int indiceArquivo = -1;
    for (int i = 0; i < N_ITEMS; i++) {
        if (dir[i].used && strcmp(dir[i].name, name) == 0) {
            indiceArquivo = i;
            break;
        }
    }

    if (indiceArquivo == -1) {
        fprintf(stderr, "ERRO: Arquivo '%s' nao encontrado.\n", name);
        return -1;
    }

    // Obter informações do arquivo
    unsigned int tamanhoArquivo = dir[indiceArquivo].length;
    unsigned int primeiroBloco = dir[indiceArquivo].first;

    // Validar offset e length
    // Se offset estiver além do tamanho do arquivo, nada para ler
    if (offset >= tamanhoArquivo) {
        return 0;  // Ler 0 bytes
    }

    // Ajustar length se estiver lendo além do final do arquivo
    if (offset + length > tamanhoArquivo) {  // Exemplo: offset = 19 (começar a partir de 19), length = 5 (ler 5 bytes), tamanhoArquivo = 20 (tamanho do arquivo de 20 bytes)
        length = tamanhoArquivo - offset;    // Exemplo: length = 20 - 19 = 1 (apenas ler o último byte)
    }

    if (length <= 0) {  // Caso o usuário solicite ler 0 bytes ou menos
        return 0;       // Nada para ler ou comprimento inválido
    }

    char bufferBloco[BLOCK_SIZE];  // Buffer temporário para ler blocos do disco
    int totalBytesLido = 0;

    // Encontrar o bloco inicial para o offset
    unsigned int blocoAtual = primeiroBloco;
    int blocosParaPular = offset / BLOCK_SIZE;

    // Andar pela FAT para encontrar o bloco correspondente ao offset
    for (int i = 0; i < blocosParaPular; i++) {
        blocoAtual = fat[blocoAtual];
    }

    // Calcular o byte inicial dentro do primeiro bloco relevante
    int byteInicialNoBloco = offset % BLOCK_SIZE;

    // Loop para ler dados
    while (totalBytesLido < length && blocoAtual != FREE && blocoAtual != EOFF) {
        if (blocoAtual >= sb.number_blocks) {
            fprintf(stderr, "ERRO: Numero de bloco invalido (%u) na FAT durante a leitura do arquivo '%s'.\n", blocoAtual, name);
            return -1;  // Indicar erro devido a entrada inválida na FAT
        }

        ds_read(blocoAtual, bufferBloco);  // Ler o bloco de dados atual

        int bytesParaCopiarDoBloco = BLOCK_SIZE - byteInicialNoBloco;
        if (bytesParaCopiarDoBloco > (length - totalBytesLido)) {
            bytesParaCopiarDoBloco = (length - totalBytesLido);
        }

        // Copiar dados do buffer do bloco para o buffer do usuário
        memcpy(buff + totalBytesLido, bufferBloco + byteInicialNoBloco, bytesParaCopiarDoBloco);

        totalBytesLido += bytesParaCopiarDoBloco;
        byteInicialNoBloco = 0;  // Após o primeiro bloco, sempre começar do início dos blocos subsequentes

        // Mover para o próximo bloco na cadeia do arquivo
        if (totalBytesLido < length) {  // Somente mover se mais dados precisarem ser lidos
            blocoAtual = fat[blocoAtual];
        }
    }

    return totalBytesLido;  // Retornar a quantidade total de bytes lidos com sucesso
}

// Retorna a quantidade de caracteres escritos
int fat_write(char *name, const char *buff, int length, int offset) {
    // Checar se o sistema de arquivos tá montado
    if (!mountState) {
        fprintf(stderr, "ERRO: Sistema de arquivos nao esta montado.\n");
        return -1;
    }

    // Encontrar o arquivo no diretório
    int indiceArquivo = -1;
    for (int i = 0; i < N_ITEMS; i++) {
        if (dir[i].used && strcmp(dir[i].name, name) == 0) {
            indiceArquivo = i;
            break;
        }
    }

    if (indiceArquivo == -1) {
        fprintf(stderr, "ERRO: Arquivo '%s' nao encontrado.\n", name);
        return -1;
    }

    // Informações do arquivo
    unsigned int tamanhoArquivo = dir[indiceArquivo].length;
    unsigned int primeiroBloco = dir[indiceArquivo].first;
    
    // Se o offset é maior que o tamanho do arquivo vira um append
    if (offset > tamanhoArquivo) {
        offset = tamanhoArquivo;
        fprintf(stderr, "AVISO: Offset maior que o tamanho do arquivo. Ajustando para anexar no final do arquivo.\n");
    }

    char bufferBloco[BLOCK_SIZE]; // Buffer temporário pra ler/escrever blocos
    int bytesEscritosTotal = 0;
    int posicaoEscritaAtual = 0;

    unsigned int blocoAtualNaCadeia;
    unsigned int ultimoBlocoCadeia = FREE; // Pra vincular novos na cadeia

    // Encontrar o bloco inicial para o offset
    if (primeiroBloco == FREE) { // O arquivo tá vazio
        blocoAtualNaCadeia = FREE; // Nenhum bloco atribuído
    } else {
        blocoAtualNaCadeia = primeiroBloco;
        int blocosParaPular = offset / BLOCK_SIZE;

        // Passar pela FAT pra encontrar o bloco onde a gravação tem que começar
        for (int i = 0; i < blocosParaPular; i++) {
            ultimoBlocoCadeia = blocoAtualNaCadeia;
            blocoAtualNaCadeia = fat[blocoAtualNaCadeia];
            if (blocoAtualNaCadeia == FREE || blocoAtualNaCadeia == EOFF) {
                break;
            }
        }
    }

    int byteInicialBloco = offset % BLOCK_SIZE;

    // Loop para gravar dados
    while (bytesEscritosTotal < length) {
        int bytesPrecisos = length - bytesEscritosTotal;  // Bytes restantes para gravar

        // Se precisarmos de um novo bloco (ou o arquivo estava vazio/chegou a EOFF ou FREE)
        if (blocoAtualNaCadeia == FREE || blocoAtualNaCadeia == EOFF) {
            int novoBloco = -1;
            for (int i = 0; i < sb.number_blocks; i++) {
                if (fat[i] == FREE) {
                    novoBloco = i;
                    break;
                }
            }
            if (novoBloco == -1) {
                fprintf(stderr, "AVISO: Disco cheio.\n");
                break; // Para de gravar
            }

            fat[novoBloco] = EOFF; // Marcar novo bloco como fim da cadeia

            if (primeiroBloco == FREE) { // Se este for o primeiro bloco do arquivo
                dir[indiceArquivo].first = novoBloco;
                primeiroBloco = novoBloco; // Atualizar variável local
            } else { // Vincular o novo bloco ao anterior
                fat[ultimoBlocoCadeia] = novoBloco;
            }
            blocoAtualNaCadeia = novoBloco; // Definir bloco atual como o recém-alocado
        }

        // Lê o bloco do disco pra preservar o que já tinha antes no arquivo
        if (byteInicialBloco != 0 || bytesPrecisos < BLOCK_SIZE) { // Só lê se for gravação parcial ou não for um bloco novo
            ds_read(blocoAtualNaCadeia, bufferBloco);
        } else {
            memset(bufferBloco, 0, BLOCK_SIZE); // Se for gravar um bloco inteiro do começo n tem q ler primeiro
        }

        int bytesParaCopiarParaBloco = BLOCK_SIZE - byteInicialBloco; // Espaço máximo que pode ser escrito no bloco atual nessa iteração
        if (bytesParaCopiarParaBloco > bytesPrecisos) { // Compara o espaço disponível no bloco com a quantidade de dados que ainda resta pra ser escrita
            bytesParaCopiarParaBloco = bytesPrecisos;
        }

        // Copiar dados do buff (parâmetro: usuário) para o buffer do bloco
        memcpy(bufferBloco + byteInicialBloco, buff + posicaoEscritaAtual, bytesParaCopiarParaBloco);

        // Escrever o bloco modificado de volta ao disco
        ds_write(blocoAtualNaCadeia, bufferBloco);

        bytesEscritosTotal += bytesParaCopiarParaBloco;
        posicaoEscritaAtual += bytesParaCopiarParaBloco;
        byteInicialBloco = 0; // Próximas escritas vão começar do início do bloco

        // Atualizar ultimoBlocoCadeia para a próxima iteração (se uma nova alocação acontecer)
        ultimoBlocoCadeia = blocoAtualNaCadeia;
        // Mover para o próximo bloco na cadeia do arquivo para a próxima iteração (se mais dados a serem escritos)
        if (bytesEscritosTotal < length) {
            blocoAtualNaCadeia = fat[blocoAtualNaCadeia];
        }
    }

    // Atualizar o tamanho do arquivo no diretório se ele tiver crescido
    unsigned int novoTamanhoArquivo = offset + bytesEscritosTotal;
    if (novoTamanhoArquivo > tamanhoArquivo) {
        dir[indiceArquivo].length = novoTamanhoArquivo;
    }

    // Escreve a FAT atualizada de volta no disco
    for (int i = 0; i < sb.n_fat_blocks; i++) {
        ds_write(TABLE + i, ((char *)fat) + (i * BLOCK_SIZE));
    }
    // Escreve o diretório atualizado de volta no disco
    ds_write(DIR, (char *)dir);

    return bytesEscritosTotal;  // Retorna o total de bytes gravados com sucesso
}