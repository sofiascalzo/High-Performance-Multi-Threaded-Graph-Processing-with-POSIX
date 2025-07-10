#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "attori.h"

attore *attori_crea(int codice, char *nome, int anno){
    attore *a = malloc(sizeof(*a));
    if (a == NULL){
        fprintf(stderr, "Errore allocazione memoria\n");
        exit(1);
    }
    a->codice = codice;
    a->nome = strdup(nome);
    a->anno = anno;
    a->numcop = 0;
    a->cop = NULL;
    return a;
}

void attore_distruggi(attore *a){
    if (a != NULL) {
        free(a->nome);
        free(a->cop);
        free(a);
    }
}

attore **attori_leggi(char *f, int *n) {
    FILE *fp = fopen(f, "r");
    if (fp == NULL) {
        fprintf(stderr, "Errore apertura del file\n");
        return NULL;
    }

    int capacita = 20000;
    attore **lista = malloc(capacita * sizeof(*lista));
    if (lista == NULL) {
        fprintf(stderr, "Errore malloc iniziale\n");
        fclose(fp);
        return NULL;
    }

    int conteggio = 0;

    char *buffer = NULL;
    size_t len = 0;

    while (getline(&buffer, &len, fp) != -1) {
        char *token;

        token = strtok(buffer, "\t\n");
        if (token == NULL) continue;
        int codice = strtol(token, NULL, 10);

        token = strtok(NULL, "\t\n");
        if (token == NULL) continue;
        char *nome = token;

        token = strtok(NULL, "\t\n");
        if (token == NULL) continue;
        int anno = strtol(token, NULL, 10);

        if (conteggio == capacita) {
            capacita *= 2;
            attore **temp = realloc(lista, capacita * sizeof(*lista));
            if (temp == NULL) {
                fprintf(stderr, "Errore realloc memoria\n");
                free(buffer);
                fclose(fp);
                for (int i = 0; i < conteggio; i++) {
                    attore_distruggi(lista[i]);
                }
                free(lista);
                return NULL;
            }
            lista = temp;
        }

        lista[conteggio] = attori_crea(codice, nome, anno);
        lista[conteggio]->index = conteggio;  
        conteggio++;
    }

    free(buffer);
    fclose(fp);
    *n = conteggio;
    return lista;
}


void attore_stampa(attore **lista, int i,FILE *file){
    fprintf(file, "Codice: %d\tNome: %s\tAnno: %d\n", lista[i]->codice, lista[i]->nome, lista[i]->anno);
}

void attori_stampa(attore **lista, int n){
    for(int i = 0; i < n; i++){
        attore_stampa(lista, i, stderr);
    }
}

int bcomparator(const void *chiave,const void *elem) {
    int codice = *((int *) chiave);
    attore *a = *((attore **) elem);

    return codice - a->codice;
}
