Initial: a=1, b=2, c=3 (in memory)

1. Load b from memory → b=2
2. Compute a = 2 + 1 = 3
3. Store a=3 to memory
   ────────────────────── MEMORY BARRIER ──────────────────────
4. Load a from memory → a=3
5. Compute c = 3 * 2 = 6
6. Store c=6 to memory
7. Load c from memory → c=6
8. Load a from memory → a=3
9. Compute b = 6 - 3 = 3
10. Store b=3 to memory

Final: a=3, b=3, c=6
