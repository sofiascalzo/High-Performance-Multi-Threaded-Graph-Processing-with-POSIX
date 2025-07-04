#ifndef RICERCA_H
#define RICERCA_H

#include "attori.h"
#include <stdbool.h>

typedef struct abr_attore {
    int codice;
    struct abr_attore *left, *right;
} abr_attore;

typedef struct {
    attore **lista;
    int n;
    int codice_a;
    int codice_b;
} bfs_args;

abr_attore *abr_inserisci(abr_attore *root, int codice);
bool abr_ricerca(abr_attore *root, int codice);
void abr_distruggi(abr_attore *root);
void *bfs(void *arg);

#endif
