# Sistema de Arquivos FAT

Implementação de um sistema de arquivos FAT (File Allocation Table) como parte do trabalho da disciplina de Sistemas Operacionais 2.

## 📝 Descrição

Este projeto implementa um sistema de arquivos FAT simplificado que simula o gerenciamento de arquivos em um disco virtual. O sistema é baseado na estrutura clássica do FAT (File Allocation Table), com adaptações para fins didáticos.

### 💾 Características do Sistema

- Blocos de 4KB (4096 bytes)
- Sistema de arquivos simulado em um único arquivo
- Diretório único (sem subdiretórios)
- Estrutura do disco:
  - Bloco 0: Superbloco (metadados do sistema)
  - Bloco 1: Diretório (entradas de arquivos)
  - Bloco 2+: FAT (tabela de alocação)
  - Blocos restantes: Área de dados

### ⚙️ Funcionalidades

- `fat_format`: Formata o sistema de arquivos
- `fat_mount`: Monta o sistema e carrega estruturas para RAM
- `fat_debug`: Exibe estado atual do sistema
- `fat_create`: Cria um novo arquivo
- `fat_delete`: Remove um arquivo
- `fat_read`: Lê conteúdo de um arquivo
- `fat_write`: Escreve em um arquivo
- `fat_getsize`: Obtém tamanho de um arquivo

## 🚀 Como Usar

### Pré-requisitos

- Sistema operacional Linux ou Windows com WSL
- GCC (GNU Compiler Collection)
- Make

### Compilação e Execução

O projeto inclui diferentes alvos no Makefile:

```bash
# Compilar e executar em modo desenvolvimento
make dev

# Compilar projeto
make all

# Limpar arquivos gerados
make clean

```

Para mais detalhes sobre os comandos disponíveis, consulte o arquivo `Makefile`.

### 📋 Comandos do Sistema

Após executar o programa, você terá acesso aos seguintes comandos:

- `formatar`: Formata o sistema de arquivos
- `montar`: Monta o sistema
- `debugar`: Debuga o sistema
- `criar <arquivo>`: Cria um arquivo
- `deletar <arquivo>`: Remove um arquivo
- `ver <arquivo>`: Lê arquivo e mostra seu conteúdo
- `medir <arquivo>`: Mostra o tamanho do arquivo
- `importar <nome no linux> <nnome fat-sys>`: Importa um arquivo do linux para o fat-sys
- `exportar <nnome fat-sys> <nome no linux>`: Exporta um arquivo do fat-sys para o linux
- `help`: Mostra os comandos disponíveis
- `sair`: Sai do programa

## 🛠️ Estrutura do Projeto

```
.
├── fat.c/fat.h     # Implementação principal do sistema FAT
├── ds.c/ds.h       # Simulador de disco
├── cmd.c           # Interface de linha de comando
├── Makefile        # Scripts de compilação
└── docs/           # Documentação
    ├── Trabalho2-FAT.pdf    # Enunciado
    └── CRONOGRAMA.md        # Planejamento
```

## 👥 Autores

Este projeto foi desenvolvido como trabalho acadêmico para a disciplina de Sistemas Operacionais 2 do curso de Ciência da Computação (IGCE - Rio Claro) da Universidade Estadual Paulista (UNESP).

Desenvolvido por:
- Gabriel Passos (@gpassos01)
- Lucas Goes (@)
- Odair Neto (@)

## 📄 Licença

Este projeto é um trabalho acadêmico e está disponível para fins educacionais.

## 📚 Referências

- Enunciado do trabalho: `Trabalho2-FAT.pdf`
- Cronograma de desenvolvimento: `CRONOGRAMA.md`