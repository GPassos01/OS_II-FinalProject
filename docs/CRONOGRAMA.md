# 📅 Cronograma — Trabalho Sistema de Arquivos FAT

## 1. Estudo e planejamento (28/05 a 01/06)

- [x] **28/05:**
  - Ler toda a especificação com atenção.
  - Explorar o código já fornecido: `cmd.c`, `ds.c`, `fat.c` e arquivos `.h`.
  - Executar `make dev` e testar comandos na `imagem-pronta`.

- [x] **29/05 a 31/05:**
  - Estudar o funcionamento do `fat_debug()`, `fat_format()` e `fat_mount()`.
  - Mapear variáveis globais (superbloco, diretório, FAT) e entender como persistem no disco.
  - Revisar como a simulação de disco (`ds.c`) manipula blocos.

- [x] **01/06:**
  - Planejar detalhadamente a implementação das três primeiras funções (`debug`, `format`, `mount`).

---

## 2. Primeira implementação funcional (02/06 a 08/06)

- [x] **02/06 a 04/06:**
  - Implementar `fat_debug()` → imprime o estado do sistema.
  - Testar com `make img` e `make dev`.

- [x] **05/06 a 06/06:**
  - Implementar `fat_format()` → cria superbloco, diretório e FAT.
  - Testar: criar nova imagem e verificar com `fat_debug()`.

- [x] **07/06 a 08/06:**
  - Implementar `fat_mount()` → carrega FAT e diretório para memória.
  - Testar montando imagem pronta e nova imagem.

---

## 3. Manipulação de arquivos (09/06 a 13/06)

- [ ] **09/06 a 10/06:**
  - Implementar `fat_create()` → adicionar arquivo vazio no diretório.
  - Testar criando vários arquivos, usar `fat_debug()` para verificar.

- [ ] **11/06 a 12/06:**
  - Implementar `fat_delete()` → remover arquivos, liberar blocos.
  - Testar deletando arquivos da imagem e verificando FAT.

- [ ] **13/06:**
  - Refatorar e revisar as funções implementadas até agora.
  - Garantir que o diretório e a FAT sejam atualizados no disco e na RAM.

---

## 4. Manipulação de dados (14/06 a 15/06)

- [ ] **14/06:**
  - Implementar `fat_read()` → ler dados de arquivos com offset.
  - Testar com `ver` e `exportar`.

- [ ] **15/06:**
  - Implementar `fat_write()` → escrever dados com alocação de blocos.
  - Testar com `importar`.

---

## 5. Apresentação: **16/06**

- [ ] Testar o sistema completo com comandos: `formatar`, `montar`, `criar`, `deletar`, `importar`, `exportar`, `ver`, `medir`.
- [ ] Preparar slides (se necessário) e exemplos demonstrativos.
- [ ] Deixar pronto para demonstrar execução e explicação das funções.

---

## Resumo das etapas:

| Data          | Atividade                                                        |
|---------------|-------------------------------------------------------------------|
| 28/05         | Leitura da especificação, análise do código, execução inicial.    |
| 29/05-01/06   | Estudo das estruturas e planejamento da implementação.            |
| 02/06-04/06   | Implementação e testes do `fat_debug()`.                          |
| 05/06-06/06   | Implementação e testes do `fat_format()`.                         |
| 07/06-08/06   | Implementação e testes do `fat_mount()`.                          |
| 09/06-10/06   | Implementação e testes do `fat_create()`.                         |
| 11/06-12/06   | Implementação e testes do `fat_delete()`.                         |
| 13/06         | Revisão das funções até então implementadas.                      |
| 14/06         | Implementação e testes do `fat_read()`.                           |
| 15/06         | Implementação e testes do `fat_write()`.                          |
| 16/06         | Testes finais e apresentação do trabalho.                         |
