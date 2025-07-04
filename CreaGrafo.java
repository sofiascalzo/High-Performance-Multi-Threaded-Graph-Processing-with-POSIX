import java.io.*;
import java.nio.file.*;
import java.util.*;

public class CreaGrafo {

    public static void main(String[] args) throws IOException {
        long startTotale = System.nanoTime();

        if (args.length < 2) {
            System.err.println("Uso: java CreaGrafo <name.basics.tsv> <title.principals.tsv>");
            return;
        }

        String inputNomi = args[0];
        String inputTitoli = args[1];
        String outputNomi = "nomi.txt";
        String outputGrafo = "grafo.txt";

        Map<Integer, Attore> attori = new HashMap<>(400_000);

        // lettura primo file
        BufferedReader reader = Files.newBufferedReader(Paths.get(inputNomi));
        reader.readLine();
        String line;
        while ((line = reader.readLine()) != null) {
            String[] campi = line.split("\t");
            if (campi.length < 6) continue;
            String nconst = campi[0];
            String nome = campi[1];
            String birth = campi[2];
            String professioni = campi[4];

            if (!birth.equals("\\N") && (professioni.contains("actor") || professioni.contains("actress"))) {
                try {
                    int codice = Integer.parseInt(nconst.substring(2));
                    int anno = Integer.parseInt(birth);
                    Attore a = new Attore(codice, nome, anno);
                    attori.put(codice, a);
                } catch (NumberFormatException e) {
                }
            }
        }
        reader.close();

        // scrivi in nomi
        BufferedWriter writerNomi = Files.newBufferedWriter(Paths.get(outputNomi));
        List<Attore> listaAttori = new ArrayList<>(attori.values());
        Collections.sort(listaAttori, new Comparator<Attore>() {
            public int compare(Attore a1, Attore a2) {
                return a1.getCodice() - a2.getCodice();
            }
        });

        for (Attore a : listaAttori) {
            writerNomi.write(a.getCodice() + "\t" + a.getNome() + "\t" + a.getAnno());
            writerNomi.newLine();
        }
        writerNomi.close();

        // costruzione grafo
        long archi = 0;
        reader = Files.newBufferedReader(Paths.get(inputTitoli));
        reader.readLine(); 
        String currentTconst = null;
        List<Integer> cast = new ArrayList<>(1000); 

        while ((line = reader.readLine()) != null) {
            String[] campi = line.split("\t");
            if (campi.length < 4) continue;

            String tconst = campi[0];
            String nconst = campi[2];
            if (!nconst.startsWith("nm")) continue;

            int codice;
            try {
                codice = Integer.parseInt(nconst.substring(2));
            } catch (NumberFormatException e) {
                continue;
            }

            if (!attori.containsKey(codice)) continue;

            if (!tconst.equals(currentTconst) && currentTconst != null) {
                archi += aggiornaCoprotagonisti(cast, attori);
                cast.clear();
            }

            cast.add(codice);
            currentTconst = tconst;
        }
        reader.close();
        archi += aggiornaCoprotagonisti(cast, attori); // ultimo titolo

        // scrivi grafo
        BufferedWriter writerGrafo = Files.newBufferedWriter(Paths.get(outputGrafo));
        for (Attore a : listaAttori) {
            List<Integer> ordinati = new ArrayList<>(a.getCoprotagonisti());
            Collections.sort(ordinati);
            writerGrafo.write(a.getCodice() + "\t" + ordinati.size());
            for (Integer c : ordinati) {
                writerGrafo.write("\t" + c);
            }
            writerGrafo.newLine();
        }
        writerGrafo.close();

        long endTotale = System.nanoTime();
        System.err.println("Totale archi (non orientati): " + archi);
        System.err.println("Totale archi (orientati): " + (archi * 2));
        System.err.printf("Tempo totale di esecuzione: %.2f secondi%n", (endTotale - startTotale) / 1e9);
    }

    public static long aggiornaCoprotagonisti(List<Integer> cast, Map<Integer, Attore> attori) {
        long nuoviArchi = 0;
        for (int i = 0; i < cast.size(); i++) {
            int a1 = cast.get(i);
            for (int j = i + 1; j < cast.size(); j++) {
                int a2 = cast.get(j);
                if (a1 != a2) {
                    boolean aggiunto1 = attori.get(a1).getCoprotagonisti().add(a2);
                    boolean aggiunto2 = attori.get(a2).getCoprotagonisti().add(a1);
                    if (aggiunto1 && aggiunto2) nuoviArchi++;
                }
            }
        }
        return nuoviArchi;
    }
}

class Attore {
    private final int codice;
    private final String nome;
    private final int anno;
    private final Set<Integer> coprotagonisti;

    public Attore(int codice, String nome, int anno) {
        this.codice = codice;
        this.nome = nome;
        this.anno = anno;
        this.coprotagonisti = new HashSet<>(); 
    }

    public int getCodice() {
        return codice;
    }

    public String getNome() {
        return nome;
    }

    public int getAnno() {
        return anno;
    }

    public Set<Integer> getCoprotagonisti() {
        return coprotagonisti;
    }
}
