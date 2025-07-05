# Laboratorio2-progetto
- Parsing del file di testo name.basics.tsv
  - Apro il file passato come primo argomento da linea di comando di tipo `.tsv`, salto la prima linea con i nomi delle colonne e poi faccio un ciclo `while ((line = reader.readLine()) != null)` ovvero fno a fine file, leggo riga per riga e faccio lo split per `\t` per separare i campi, ognuno dei quali viene salvato in una variabile.
  - Il campo codice e anno essendo interi devono essere castati ad intero considerando solo la sottostringa con i primi due caratteri `nm` rimossi e inseriti nella lista di Attori.
  - I controlli che vengono fatti sono che il campo `Birth` sia definito e che la persona sia una `Attrice/Attore` che il codice inizi con `nm` (altrimenti si salta la riga).
  - Dalla lettura del primo file oltre a essere inizializzati i campi di attore (eccetto `Attore.coprotagonisti[]`), creo anche la lista di nomi ordinati per codice degli attori validi che vengono salvati nel file `nomi.txt`.
- Implementazione della FIFO nell'algoritmo BFS
  - 
- Costruzione dei nodi intermedi del cammino.c
- Comunicazione tra il thread gestore comunica per l'interruzione con il main per l'interruzione 
