#ifndef ATTORI_H
#define ATTORI_H

typedef struct {
    int codice;
    char *nome;
    int anno;
    int numcop; 
    int *cop;  
    int index; //mi serve per la bfs 
} attore;

attore *attori_crea(int codice, char *nome, int anno);
void attore_distruggi(attore *a);
attore **attori_leggi(char *f, int *n);
void attori_stampa(attore **lista, int n);
void attore_stampa(attore **lista, int i, FILE *file);
int bcomparator(const void *chiave, const void *elem);

#endif

