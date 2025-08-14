# Actor Collaboration Graph and Shortest Path Search

**Overview:**
This project builds and analyzes an undirected graph of actors and their collaborations, based on IMDb datasets. The workflow is divided into two main components implemented in different programming languages for educational purposes:

**Title:**
Actor Collaboration Graph and Shortest Path Search

**Overview:**
This project constructs and analyzes an undirected graph of actors and their collaborations, using IMDb datasets. The implementation is divided into multiple components written in different programming languages, each responsible for a distinct stage of the workflow.


### **1. Java – Graph Construction (`CreaGrafo.java`)**

* **Input:** `name.basics.tsv` and `title.principals.tsv`.
* **Filtering:**

  * Only actors/actresses with a known birth year are considered.
  * Excludes other professions (e.g., directors, producers).
* **Data Structures:**

  * `Map<Integer, Attore>` associates each unique integer actor ID with its corresponding `Attore` instance.
  * Each `Attore` stores its code, name, birth year, and a `Set<Integer>` of co-star codes.
* **Output Files:**

  * `nomi.txt`: actor list in ascending ID order, containing code, name, and birth year.
  * `grafo.txt`: adjacency list for each actor (number of co-stars + sorted co-star codes).
* **Performance Constraints:**

  * Single-pass reading of each input file.
  * All lookups and updates in constant or logarithmic time (no linear scans).
  * Execution under 5 minutes on reference machine.


### **2. C – Shortest Path Search (`cammini.c`)**

* **Input:** `nomi.txt`, `grafo.txt`, and number of consumer threads.
* **Data Loading:**

  * Actors stored in a sorted array to enable O(log n) lookups via `bsearch`.
  * Graph loaded concurrently using a **producer-consumer model**:

    * Producer thread reads lines from `grafo.txt`.
    * Multiple consumer threads parse adjacency lists and populate `numcop` and `cop` arrays.
* **Shortest Path Algorithm:**

  * Implemented as **Breadth-First Search (BFS)** using a dynamically allocated **FIFO queue** capable of holding an arbitrary number of elements.
  * Tracks visited nodes using a balanced **binary search tree (BST)** with `shuffle()` transformation to avoid degenerate cases.
  * Path reconstruction stores parent references and retrieves intermediate nodes in order.
* **Multithreading:**

  * For each `(a, b)` pair read from a named pipe, a **detached thread** computes the shortest path.
  * Output written to file `a.b` and a summary printed to stdout, including path length or “no path” message and elapsed time in seconds (measured with `times()`).
* **Signal Handling:**

  * Dedicated signal handler thread prints the process PID and reacts to `SIGINT`:

    * During graph loading: prints status message and continues.
    * During pipe reading: waits 20 seconds, frees all resources, and terminates.
* **Memory Management:**

  * All allocations performed with `malloc`/`realloc`.
  * No global variables.
  * Verified with **Valgrind** to have **no memory leaks or invalid accesses**.
  

## **Techniques and Implementation Details:**

* Efficient parsing of large TSV files with tab-delimited fields.
* Graph representation optimized for large datasets (\~400M edges).
* FIFO queue implemented without fixed size, ensuring scalability for BFS on large graphs.
* BST with key shuffling to maintain balance during ordered insertions.
* Detached worker threads to avoid `pthread_join()` overhead.
* POSIX-compliant signal handling and multi-thread synchronization.
* Makefile builds both Java and C components with a single `make` command; optimized compilation (`-O3`) for performance.
* Project passes the provided automated verification script (`controllaProgetto.py`) and meets all performance and correctness constraints.

The final deliverable is a modular, multi-language system capable of efficiently handling IMDb-scale datasets, supporting both structural graph analysis and detailed collaboration queries.


## **How to Run**
 * **Compilation**

  * From the project root directory:

  * ```make```


  * This will: Compile CreaGrafo.java into Java bytecode - Compile cammini.c into the cammini.out executable with -O3 optimization.

* **Graph Construction**

  * Run CreaGrafo.java to generate nomi.txt and grafo.txt:

  * ```java CreaGrafo name.basics.tsv title.principals.tsv```


  * For the complete project, this will also generate partecipazioni.txt.

**Shortest Path Search**

  Start cammini.out, specifying number of consumer threads:

  ```./cammini.out nomi.txt grafo.txt 4```


  The program will wait for (a, b) pairs sent via a named pipe cammini.pipe.

**Sending Actor Pairs to Pipe**

  In another terminal:

  ```./cammini.py 148 8570840```


  This will write to the pipe, trigger BFS computation, and produce files named 148.8570840 with the path result.


**Project Verification**

  Run the provided test script to ensure compliance:

  ```python3 controllaProgetto.py```


  Add -r option for reduced project testing.
