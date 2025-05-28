#include "fat.h"
#include "ds.h"
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#define SUPER 0
#define TABLE 2
#define DIR 1

#define SIZE 1024

// the superblock
#define MAGIC_N           0xAC0010DE
typedef struct{
	int magic;
	int number_blocks;
	int n_fat_blocks;
	char empty[BLOCK_SIZE-3*sizeof(int)];
} super;

super sb;

//item
#define MAX_LETTERS 6
#define OK 1
#define NON_OK 0
typedef struct{
	unsigned char used;
	char name[MAX_LETTERS+1];
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

int fat_format(){ 
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
	fat = (unsigned int*)malloc(sb.number_blocks * sizeof(unsigned int));
	if (!fat) {
		return -1;
	}

	// Inicializa a FAT com blocos livres
	for (int i = 0; i < sb.number_blocks; i++) {
		fat[i] = FREE;
	}

	// Marca os blocos do superbloco, diretório e FAT como ocupados
	fat[SUPER] = BUSY; // Bloco 0: Superbloco
	fat[DIR] = BUSY;   // Bloco 1: Diretório
	for (int i = TABLE; i < TABLE + sb.n_fat_blocks; i++) {
		fat[i] = BUSY; // Blocos da FAT
	}

	// Escreve a FAT no disco
	for (int i = 0; i < sb.n_fat_blocks; i++) {
		ds_write(TABLE + i, ((char*)fat) + (i * BLOCK_SIZE));
	}

	free(fat);
	fat = NULL;

	return 0;
}

void fat_debug(){
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

int fat_mount(){
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
	fat = (unsigned int*)malloc(sb.number_blocks * sizeof(unsigned int));
	if (!fat) {
		return -1;
	}

	// Lê a FAT do disco
	for (int i = 0; i < sb.n_fat_blocks; i++) {
		ds_read(TABLE + i, ((char*)fat) + (i * BLOCK_SIZE));
	}

	// Sistema montado com sucesso
	mountState = 1;
	return 0;
}

int fat_create(char *name){
  	return 0;
}

int fat_delete( char *name){
  	return 0;
}

int fat_getsize( char *name){ 
	return 0;
}

//Retorna a quantidade de caracteres lidos
int fat_read( char *name, char *buff, int length, int offset){
	return 0;
}

//Retorna a quantidade de caracteres escritos
int fat_write( char *name, const char *buff, int length, int offset){
	return 0;
}
