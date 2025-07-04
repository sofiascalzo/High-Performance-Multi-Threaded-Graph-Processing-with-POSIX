# definizione del compilatore e dei flag di compilazione
# che vengono usate dalle regole implicite
CC=gcc
CFLAGS=-std=gnu11 -Wall -g -O3 -pthread
LDLIBS=-lm -pthread 
JAVA=javac


# su https://www.gnu.org/software/make/manual/make.html#Implicit-Rules
# sono elencate le regole implicite e le variabili 
# usate dalle regole implicite 

# Variabili automatiche: https://www.gnu.org/software/make/manual/make.html#Automatic-Variables
# nei comandi associati ad ogni regola:
#  $@ viene sostituito con il nome del target
#  $< viene sostituito con il primo prerequisito
#  $^ viene sostituito con tutti i prerequisiti

# elenco degli eseguibili da creare
EXECS=cammini.out CreaGrafo.class

# primo target: gli eseguibili sono precondizioni
# quindi verranno tutti creati
all: $(EXECS) 

# regola per la creazione degli eseguibili
%.out: %.o attori.o ricerca.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# pattern rule che esprime che tutti i .o 
%.o: %.c ricerca.h attori.h

	$(CC) $(CFLAGS) -c $<

%.class: %.java
	$(JAVA) $< 


# esempio di target che non corrisponde a una compilazione
# ma esegue la cancellazione dei file oggetto e degli eseguibili
clean: 
	rm *.o $(EXECS)
	
	
	
	
	
