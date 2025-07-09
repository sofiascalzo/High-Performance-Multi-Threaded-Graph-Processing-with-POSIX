# Laboratorio2-progetto
- Parsing del file di testo name.basics.tsv
  - Apro il file passato come primo argomento da linea di comando di tipo `.tsv`, salto la prima linea con i nomi delle colonne e poi faccio un ciclo `while ((line = reader.readLine()) != null)` ovvero fno a fine file, leggo riga per riga e faccio lo split per `\t` per separare i campi, ognuno dei quali viene salvato in una variabile.
  - Il campo codice e anno essendo interi devono essere castati ad intero considerando solo la sottostringa con i primi due caratteri `nm` rimossi e inseriti nella lista di Attori.
  - I controlli che vengono fatti sono che il campo `Birth` sia definito e che la persona sia una `Attrice/Attore` che il codice inizi con `nm` (altrimenti si salta la riga).
  - Dalla lettura del primo file oltre a essere inizializzati i campi di attore (eccetto `Attore.coprotagonisti[]`), creo anche la lista di nomi ordinati per codice degli attori validi che vengono salvati nel file `nomi.txt`.
- Implementazione della FIFO nell'algoritmo BFS
  - La BFS nel file `ricerca.c` usa una fifo `visited` per tenere traccia dei figli da visitare. è inizializzata con una calloc di dimensine `n` (dove n è il numero di attori). La dimensione `n` è sicuramente molto piu del necessario ma non avendo problemi di memoria in questo modo posso evitare l'overhed delle `realloc`.
  - Per simulare una fifo ho due puntatori, uno `front` per tenere traccia del prossimo elemnto su cui fare `pop()` e l'altro `back` per aggiungere in coda.
- Costruzione dei nodi intermedi del cammino.c
  - Per la BFS abbiamo `visited`: array di indici degli attori visitati, `precedente`: array che tiene traccia del predecessore (per ricostruire il cammino), `abr_attore *explored`: un albero binario di ricerca (ABR) usato per registrare gli attori già esplorati.
  - Per la costruzione dei nodi intermedi si scorre la lista di coprotagonisti dell'attore, si controlla che non sia già esplorato con `abr_ricerca()`, se non lo è chiamo `abr_inserisci()`. Con `precedente[vicino_idx] = curr->codice`;, si registra da quale attore si è arrivati a questo vicino, utile per ricostruire il percorso. Alla fine della ricerca, se si trova il nodo di destinazione (`codice_b`), si risale a ritroso con `precedente[]` per costruire il cammino completo.


- Comunicazione tra il thread e il main per l'interruzione
  - Il main crea una maschera completa dei segnali con `sigset_t all_signals; sigfillset(&all_signals); pthread_sigmask(SIG_BLOCK, &all_signals, NULL);` e inizializza un thread apposta per la gestione del segnale sigint, pertanto tutti i segnali sono bloccati e non gestiti eccetto sigint che verra` necessariamnte gestito dal thread gestore. Dopo aver stampato il pid del processo il gestore controlla i seguenti casi:
     - `pipe==0`: segnala che il grafo e` ancoa in costuzione
    - `pipe==1`: scrive -1 nella pipe in modo da segnalere la fine della scrittura e fare pthread_exit()
    - `se la pipe si chiude`: faccio sempliemente break per chiamare la pthread_exit(NULL) dato che questo era l'unico caso in cui non venivano liberate le risorse 
