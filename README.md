# Laboratorio2-progetto

### Parsing del file `name.basics.tsv`

* Il file `.tsv`, passato come primo argomento da linea di comando, viene aperto e letto riga per riga ignorando l'intestazione.
* Ogni riga è suddivisa con `split("\t")`; i campi risultanti vengono salvati in variabili locali.
* I controlli eseguiti includono:

  * presenza del campo `Birth`,
  * che il tipo sia `actor` o `actress`,
  * che il codice inizi con `nm`.
* Il codice identificativo viene convertito in intero eliminando il prefisso `nm`; anche il campo anno viene convertito.
* Gli attori validi vengono inseriti in una lista e salvati nel file `nomi.txt`, ordinati per codice.
  Il campo `coprotagonisti[]` non è ancora inizializzato in questa fase.

### Implementazione della FIFO nell'algoritmo BFS

* La BFS (`ricerca.c`) utilizza un array `visited` come coda FIFO per la gestione degli attori da esplorare.
* L’array è allocato con `calloc(n)`, dove `n` è il numero totale di attori. Si evita così l'overhead di `realloc`, a costo di occupare più memoria.
* La coda è gestita tramite due indici:

  * `front`: posizione del prossimo elemento da estrarre,
  * `back`: posizione in cui inserire il prossimo elemento.

### Costruzione dei nodi intermedi (`cammino.c`)

* Le strutture usate sono:

  * `visited[]`: array di indici degli attori già visitati,
  * `precedente[]`: per tracciare il predecessore di ogni attore,
  * `abr_attore *explored`: ABR usato per verificare se un attore è già stato esplorato.
* Per ogni coprotagonista di un attore:

  * si verifica se è già stato esplorato tramite `abr_ricerca()`,
  * se non lo è, viene aggiunto con `abr_inserisci()`,
  * si registra il predecessore con `precedente[vicino_idx] = curr->codice`.
* Alla fine, se il nodo di destinazione è raggiunto (`codice_b`), il cammino viene ricostruito a ritroso tramite `precedente[]`.

### Comunicazione tra thread e main per l'interruzione

* Il `main` blocca tutti i segnali (`sigfillset` + `pthread_sigmask`) e delega la gestione del `SIGINT` a un thread dedicato.
* Il thread gestore:

  * stampa il PID,
  * attende segnali tramite `sigwait()`,
  * verifica lo stato della pipe:

    * `pipe == 0`: segnala che la costruzione del grafo è in corso,
    * `pipe == 1`: scrive `-1` nella pipe e termina con `pthread_exit()`,
    * se la pipe viene chiusa: effettua un semplice `break` seguito da `pthread_exit(NULL)` per liberare le risorse.
