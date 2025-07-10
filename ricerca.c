#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/times.h>
#include <unistd.h>
#include <pthread.h>
#include "attori.h"
#include "ricerca.h"

int shuffle(int n) {
    return ((((n & 0x3F) << 26) | ((n >> 6) & 0x3FFFFFF)) ^ 0x55555555);
}

int unshuffle(int n) {
    return ((((n >> 26) & 0x3F) | ((n & 0x3FFFFFF) << 6)) ^ 0x55555555);
}

abr_attore *abr_inserisci(abr_attore *root, int codice) {
    if (!root) {
        abr_attore *nuovo = malloc(sizeof(abr_attore));
        nuovo->codice = codice;
        nuovo->left = nuovo->right = NULL;
        return nuovo;
    }

    int diff = shuffle(codice) - shuffle(root->codice);
    if (diff < 0)
        root->left = abr_inserisci(root->left, codice);
    else if (diff > 0)
        root->right = abr_inserisci(root->right, codice);

    return root;
}

bool abr_ricerca(abr_attore *root, int codice) {
    if (!root) return false;
    int diff = shuffle(codice) - shuffle(root->codice);
    if (diff == 0) return true;
    if (diff < 0) return abr_ricerca(root->left, codice);
    return abr_ricerca(root->right, codice);
}

void abr_distruggi(abr_attore *root) {
    if (!root) return;
    abr_distruggi(root->left);
    abr_distruggi(root->right);
    free(root);
}

void *bfs(void *arg) {
    bfs_args *args = (bfs_args *)arg;
    attore **lista = args->lista;
    int n = args->n;
    int codice_a = args->codice_a;
    int codice_b = args->codice_b;
    free(arg);

    struct tms start_time, end_time;
    clock_t start = times(&start_time);

    int idx_a = -1, idx_b = -1;
    attore **a=bsearch(&codice_a, lista, n, sizeof(attore *), &bcomparator);
    if (a==NULL){
        idx_a=-1;
    } else {
        idx_a= (*a)->index;
    }
   attore **b=bsearch(&codice_b, lista, n, sizeof(attore *), &bcomparator);
        if (b==NULL){
        idx_b=-1;
    }  else {
        idx_b= (*b)->index;
    }


    char nomefile[64];
    snprintf(nomefile, sizeof(nomefile), "%d.%d", codice_a, codice_b);
    FILE *out = fopen(nomefile, "w");
    if (!out) {
        perror("Errore apertura file output");
        pthread_exit(NULL);
    }

    if (idx_a == -1 || idx_b == -1) {
        int invalido = (idx_a == -1) ? codice_a : codice_b;
        fprintf(out, "codice %d non valido\n", invalido);
        fclose(out);
        printf("%d.%d: codice %d non valido. Tempo di elaborazione 0.00 secondi\n", codice_a, codice_b, invalido);
        pthread_exit(NULL);
    }
    
    //attore_stampa(lista, idx_a, stdout);
    //attore_stampa(lista, idx_b, stdout);

    // BFS
    abr_attore *explored = NULL;

    int *visited = calloc(n, sizeof(int));
    int front = 0, back = 0;

    int *precedente = calloc(n, sizeof(int));

    visited[back++] = idx_a;
    explored = abr_inserisci(explored, lista[idx_a]->codice);

    bool found = false;

    while (front < back && !found) {
        int curr_idx = visited[front++];
        attore *curr = lista[curr_idx];

        for (int i = 0; i < curr->numcop; i++) {
            int vicino_cod = curr->cop[i];
            int vicino_idx = -1;

            attore **v=bsearch(&vicino_cod, lista, n, sizeof(attore *), &bcomparator);
            if(v==NULL) {
                perror("Attore non trovato");
                continue;
            }
            vicino_idx = (*v)->index;

            if (vicino_idx == -1 || abr_ricerca(explored, vicino_cod))
                continue;

            explored = abr_inserisci(explored, vicino_cod);
            precedente[vicino_idx] = curr->codice;
            visited[back++] = vicino_idx;

            if (vicino_cod == codice_b) {
                found = true;
                break;
            }
        }
    }


    clock_t end = times(&end_time);
    double elapsed = (double)(end - start) / sysconf(_SC_CLK_TCK);

    if (!found) {
        fprintf(out, "non esistono cammini da %d a %d\n", codice_a, codice_b);
        fclose(out);
        printf("%d.%d: Nessun cammino. Tempo di elaborazione %.2f secondi\n", codice_a, codice_b, elapsed);
        free(visited);
        free(precedente);
        abr_distruggi(explored);
        pthread_exit(NULL);
    }

    int path[n];
    int len = 0;
    int curr_cod = codice_b;

    while (curr_cod != 0) {
        for (int i = 0; i < n; i++) {
            if (lista[i]->codice == curr_cod) {
                path[len++] = i;
                break;
            }
        }
        int idx = -1;
        for (int i = 0; i < n; i++) {
            if (lista[i]->codice == curr_cod) {
                idx = lista[i]->index;
                break;
            }
        }
        curr_cod = (idx != -1) ? precedente[idx] : 0;
    }

    for (int i = len - 1; i >= 0; i--) {
        attore *a = lista[path[i]];
        fprintf(out, "%d\t%s\t%d\n", a->codice, a->nome, a->anno);
    }

    fclose(out);
    printf("%d.%d: Lunghezza minima %d. Tempo di elaborazione %.2f secondi\n",
           codice_a, codice_b, len - 1, elapsed);

    free(visited);
    free(precedente);
    abr_distruggi(explored);
    pthread_exit(NULL);
}
