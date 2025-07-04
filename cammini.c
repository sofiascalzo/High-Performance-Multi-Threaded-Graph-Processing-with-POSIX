
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdint.h>
#include <time.h>

#include "attori.h"
#include "ricerca.h"

typedef struct {  
    attore **attori;
    char **buffer;
    int *cindex;
    int *dati;
    int bufsize;
    pthread_mutex_t *mutex;
    pthread_cond_t *empty;
    pthread_cond_t *full;
    int num_attori;
} dati_consumatori;

typedef struct {
    attore **attori;
    char **buffer;
    int *pindex;
    int *dati;
    int bufsize;
    pthread_mutex_t *mutex;
    pthread_cond_t *empty;
    pthread_cond_t *full;
    FILE *fp;
} dati_produttore;


typedef struct{
    int *pipe;     // 0: costruzione, 1: lettura piipe
} signal_args;

// CONSUMATORE
void *cbody(void *arg) {
    dati_consumatori *a = (dati_consumatori *)arg;

    while (1) {
        pthread_mutex_lock(a->mutex);

        while (*(a->dati) == 0) {
            pthread_cond_wait(a->empty, a->mutex);
        }

        char *line = a->buffer[*(a->cindex) % a->bufsize];

        if (line == NULL) {
            pthread_mutex_unlock(a->mutex);
            break;
        }

        (*(a->cindex))++;
        (*(a->dati))--;

        pthread_cond_signal(a->full);
        pthread_mutex_unlock(a->mutex);

        // PARSA LINEA: codice, numcop, cop...
        int codice, numcop, nread;
        char *ptr = line;
        if (sscanf(ptr, "%d\t%d%n", &codice, &numcop, &nread) < 2) continue;
        ptr += nread;

        while (*ptr != '\t' && *ptr != '\0') ptr++; // skip to cop list
        if (*ptr == '\t') ptr++;

        int *cop = malloc(sizeof(int) * numcop);
        for (int i = 0; i < numcop; i++) {
            sscanf(ptr, "%d", &cop[i]);
            while (*ptr != '\t' && *ptr != '\n' && *ptr != '\0') ptr++;
            if (*ptr == '\t') ptr++;
        }

        attore **b=bsearch(&codice, a->attori, a->num_attori, sizeof(*a->attori), &bcomparator);
        if(b==NULL) perror("Attore non trovato");
        (*b)->numcop = numcop;
        (*b)->cop = cop;

        free(line);
    }

    pthread_exit(NULL);
}

// PRODUTTORE: nel main
void produttore(dati_produttore *a) {
    char *line = NULL;
    size_t len = 0;

    while (getline(&line, &len, a->fp) != -1) {
        pthread_mutex_lock(a->mutex);

        while (*(a->dati) == a->bufsize) {
            pthread_cond_wait(a->full, a->mutex);
        }

        a->buffer[*(a->pindex) % a->bufsize] = line;
        (*(a->pindex))++;
        (*(a->dati))++;

        pthread_cond_signal(a->empty);
        pthread_mutex_unlock(a->mutex);

        line = NULL; // importante per fare getline() su nuova allocazione
        len = 0;
    }
    free(line);

    // segnala fine lavoro ai consumatori mettendo linee NULL
    for (int i = 0; i < a->bufsize; i++) {
        pthread_mutex_lock(a->mutex);
        while (*(a->dati) == a->bufsize) {
            pthread_cond_wait(a->full, a->mutex);
        }
        a->buffer[*(a->pindex) % a->bufsize] = NULL;
        (*(a->pindex))++;
        (*(a->dati))++;
        pthread_cond_signal(a->empty);
        pthread_mutex_unlock(a->mutex);
    }

    fclose(a->fp);
}



void *tgestore(void *arg){
    //deve liberare lui le risorse o il main?
    
    signal_args *s_arg=(signal_args *)arg;

    int *pipe=s_arg->pipe;
    free(s_arg);

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);

    fprintf(stdout, "PID processo gestore dei segnali %d\n", getpid());

    while(1){
        int sig;
        int e = sigwait(&set, &sig);
        

        if (e<0) fprintf(stderr,"Errore siginfo");
        if (*pipe==0){
           fprintf(stdout, "Costruzione del grafo in corso\n"); 
        }
        else if (*pipe == 1){ //scrive sulla pipe e poi fa break uscendo dalla pthread_exit
            int fd = open("cammini.pipe", O_WRONLY);
            int fine= -1;
            int e=write(fd, &fine, sizeof(int)); 
            if(e!=sizeof(int)) fprintf(stderr,"Errore scrittura pipe");
            e=write(fd, &fine, sizeof(int));
            if(e!=sizeof(int)) fprintf(stderr,"Errore scrittura pipe");
            close(fd);
            fprintf(stderr, "Fine gestione dei segnali con -1\n"); //esce con sigint
            break;
        } else {
            fprintf(stderr, "Fine gestione dei segnali con kill\n"); //nuovo stato, pipe=2 perche quando si chiude la pipe la write essendo bloccante non chiamava la pthred exit
            break;
        }
    }

    pthread_exit(NULL);
}


int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Uso: %s nomi.txt grafo.txt <numconsumatori>\n", argv[0]);
        exit(1);
    }

    int n;
    attore **attori = attori_leggi(argv[1], &n);
    if (attori == NULL) {
        fprintf(stderr, "Errore nella lettura del file attori\n");
        exit(1);
    }

    sigset_t all_signals;
    sigfillset(&all_signals);
    pthread_sigmask(SIG_BLOCK, &all_signals, NULL);

    int pipe = 0;

    signal_args *s_arg = malloc(sizeof(signal_args));
    s_arg->pipe = &pipe;

    pthread_t signal_thread;
    pthread_create(&signal_thread, NULL, tgestore, s_arg);
    pthread_detach(signal_thread);

    int tc = atoi(argv[3]);
    assert(tc > 0);

    int bufsize = 10;
    char **buffer = malloc(sizeof(char *) * bufsize);
    int pindex = 0, cindex = 0, dati = 0;

    pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
    pthread_cond_t full = PTHREAD_COND_INITIALIZER;

    pthread_t cons[tc];
    dati_consumatori ac = {
        .attori = attori,
        .buffer = buffer,
        .cindex = &cindex,
        .dati = &dati,
        .bufsize = bufsize,
        .mutex = &mu,
        .empty = &empty,
        .full = &full,
        .num_attori = n
    };

    for (int i = 0; i < tc; i++) {
        pthread_create(&cons[i], NULL, cbody, &ac);
    }

    FILE *fp = fopen(argv[2], "rt");
    if (fp == NULL) {
        perror("Errore apertura grafo.txt");
        exit(1);        
    }

    dati_produttore ap = {
        .attori = attori,
        .buffer = buffer,
        .pindex = &pindex,
        .dati = &dati,
        .bufsize = bufsize,
        .mutex = &mu,
        .empty = &empty,
        .full = &full,
        .fp = fp
    };

    produttore(&ap);

    for (int i = 0; i < tc; i++) {
        pthread_join(cons[i], NULL);
       }

    pthread_mutex_destroy(&mu);
    pthread_cond_destroy(&empty);
    pthread_cond_destroy(&full);
    
    //se non esiste gia la crea
    if (access("cammini.pipe", F_OK) != -1) {
        unlink("cammini.pipe");
    }
    if (mkfifo("cammini.pipe", 0666) == -1) {
            perror("mkfifo");
            exit(1);
        }

    int fd = open("cammini.pipe", O_RDONLY);
    if (fd < 0){
        fprintf(stderr,"Errore apertura della pipe");
        exit(1);
    }


    pipe=1;

    int ab[2];
    while(1){
        ab[0]=0;ab[1]=0;
        ssize_t letti = read(fd, &ab[0], sizeof(int));
        letti = read(fd, &ab[1], sizeof(int));
        printf("%d -> %d\n", ab[0], ab[1]);

        if (letti==0 || ab[0]==-1 || ab[1]==-1){ 
            break;
        }
        if (letti < 0 ){
            printf("Errore lettura pipe");
            break;
        }
        if (letti != sizeof(int)) continue;

        bfs_args *arg = malloc(sizeof(bfs_args));
        arg->lista = attori;
        arg->n = n;
        arg->codice_a = ab[0];
        arg->codice_b = ab[1];
        
        pthread_t t;
        pthread_create(&t, NULL, &bfs, arg);
        pthread_detach(t); // obbligatorio perche non so quanti thread verrano creati e comunque sono tutte indipendenti

    }

    close(fd);
    pipe=2;
    if(kill(getpid(), SIGINT) != 0) //perche se si chiude la pipe in scrittura il tgestore non pthread_exit, quindi invio sigint in modo che tgestore scriva -1 nell pipe e esca
    
    sleep(20);
    unlink("cammini.pipe");

    for (int i = 0; i < n; i++) {
        attore_distruggi(attori[i]);
    }
    free(attori); 

    free(buffer); 

    return 0;
}


//gcc -o cammini cammini.c attori.c
