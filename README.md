# Prime Number Search with Threading – Four Variants

This project contains four C++ programs that search for prime numbers using multiple threads. Each variant uses a different combination of task division and printing style.

All variants read configuration from a `config.txt` file in the same folder and can be run directly from Visual Studio Code by pressing the Run button (▶️).

---

## How to Run

1. Open any variant folder (e.g., `variant1_straight_immediate`).
2. Make sure `config.txt` is in the same folder as the `.cpp` file.
3. Open the `.cpp` file in Visual Studio Code.
4. Press the **Run** button (or `Ctrl + F5`).
5. A console window will open, show the thread output.
6. Press any key to close the window after all threads had been outputed.

---

## What Each Variant Does

**Variant 1 – Straight Division + Immediate Print**  
The number range (1 to max_number) is split into equal blocks. For example, with 4 threads and max_number=1000, Thread 0 checks 1–250, Thread 1 checks 251–500, and so on. Each thread prints a prime as soon as it finds one, along with its thread ID and a timestamp. Output appears in real time and may be out of numerical order.

**Variant 2 – Straight Division + Batch Print**  
Uses the same block-based division as Variant 1, but threads do not print while working. Instead, all results are collected and printed together after all threads finish. The output is grouped by thread and appears only at the end.

**Variant 3 – Interleaved Division + Immediate Print**  
Numbers are assigned in a round-robin way: Thread 0 checks 1, 5, 9, 13…; Thread 1 checks 2, 6, 10, 14…; etc. Each thread prints primes immediately with a timestamp. Because of this pattern, some threads may find very few or no primes (for example, with 4 threads, Thread 3 only checks multiples of 4, which are never prime).

**Variant 4 – Interleaved Division + Batch Print**  
Uses the same round-robin assignment as Variant 3, but waits until all threads are done before printing all results at once. Output is grouped by thread and shown neatly at the end.

---

## Config File

Each folder includes a `config.txt` file with two settings:

```
threads = 4
max_number = 100
```

- threads: number of threads to create (must be at least 1)
- max_number: upper limit of the search range (1 to max_number)

You can change these values to test different scenarios. For best performance, keep `max_number` under 100,000.

---

## Notes

- These programs use standard C++ threading and require a C++17-compatible compiler.
- No external libraries are needed.
- The primality test uses trial division, which is simple but slows down for very large numbers. 