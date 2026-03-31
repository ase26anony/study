/* test-mcf-debug.c
 * 
 * This test is designed to trigger the uncovered debug dump lines in GCC's
 * mcf.cc (Min-Cost Flow solver) when compiled with a GCC configured with
 * --enable-checking (defining MCF_DEBUG).
 *
 * The function `high_pressure_region` creates a scenario with many
 * simultaneously live variables across complex control flow, forcing IRA
 * to build a fixup graph that requires artificial source/sink nodes
 * (new_exit_index, new_entry_index). When MCF_DEBUG is active and the
 * fixup graph is dumped, the uncovered lines printing "NEW_EXIT" and
 * "NEW_ENTRY" should be executed.
 *
 * Compile with a debug-enabled GCC (configured with --enable-checking):
 *   gcc-debug -O3 -funroll-loops -c test-mcf-debug.c -o test.o
 * or
 *   gcc-debug -O2 -fno-expensive-optimizations -c test-mcf-debug.c -o test.o
 */

/* Force noinline to prevent the compiler from merging or eliminating the
 * high-pressure function entirely. */
#define NOINLINE __attribute__((noinline))

/* Use volatile to prevent dead-code elimination and force memory traffic,
 * increasing register pressure. */
#define VOL volatile

/* Memory barrier to force variables to be live across it. */
#define MEMORY_BARRIER asm volatile("" : : : "memory")

/* Function attribute to enable aggressive optimization (unrolling, etc.)
 * specifically for this function. */
NOINLINE __attribute__((optimize("O3")))
void high_pressure_region(int seed) {
    /* Declare 18 volatile integer variables to create high register pressure.
     * All are made live across loops and conditionals. */
    VOL int v0 = seed;
    VOL int v1 = seed + 1;
    VOL int v2 = seed + 2;
    VOL int v3 = seed + 3;
    VOL int v4 = seed + 4;
    VOL int v5 = seed + 5;
    VOL int v6 = seed + 6;
    VOL int v7 = seed + 7;
    VOL int v8 = seed + 8;
    VOL int v9 = seed + 9;
    VOL int v10 = seed + 10;
    VOL int v11 = seed + 11;
    VOL int v12 = seed + 12;
    VOL int v13 = seed + 13;
    VOL int v14 = seed + 14;
    VOL int v15 = seed + 15;
    VOL int v16 = seed + 16;
    VOL int v17 = seed + 17;

    /* Additional non-volatile temporaries to increase the allocator's working set. */
    int t0, t1, t2, t3, t4;

    /* Complex control flow: outer switch with multiple cases. */
    switch (seed % 5) {
        case 0:
            /* Chain arithmetic operations to create data dependencies. */
            v0 = v1 + v2;
            v3 = v0 * v4;
            v5 = v3 - v6;
            v7 = v5 / (v8 + 1);
            break;
        case 1:
            v9 = v10 * v11;
            v12 = v9 | v13;
            v14 = v12 & v15;
            v16 = v14 ^ v17;
            break;
        case 2:
            v1 = v2 * v3;
            v4 = v5 + v6;
            v7 = v8 - v9;
            v10 = v11 ^ v12;
            break;
        case 3:
            v13 = v14 + v15;
            v16 = v17 * v0;
            v1 = v2 - v3;
            v4 = v5 | v6;
            break;
        case 4:
            v7 = v8 * v9;
            v10 = v11 + v12;
            v13 = v14 - v15;
            v16 = v17 & v0;
            break;
    }

    MEMORY_BARRIER;

    /* Nested loops with constant bounds to encourage unrolling. */
    for (int i = 0; i < 4; ++i) {
        /* Use a subset of variables in each iteration to extend live ranges. */
        if (i % 2 == 0) {
            v0 = v1 + v2;
            v3 = v4 * v5;
            v6 = v7 - v8;
            t0 = v9 + v10;
        } else {
            v11 = v12 * v13;
            v14 = v15 + v16;
            v17 = v0 - v1;
            t1 = v2 + v3;
        }

        /* Inner conditional chain. */
        if (v0 > v1) {
            v4 = v5 + v6;
            v7 = v8 * v9;
            t2 = v10 - v11;
        } else if (v2 < v3) {
            v12 = v13 + v14;
            v15 = v16 * v17;
            t3 = v0 - v1;
        } else {
            v2 = v3 + v4;
            v5 = v6 * v7;
            t4 = v8 - v9;
        }

        /* More arithmetic chains to increase pressure. */
        v10 = v11 + v12;
        v13 = v14 * v15;
        v16 = v17 + v0;
        v1 = v2 * v3;

        MEMORY_BARRIER;
    }

    /* Final mixing to ensure all variables are live at the end of the loop. */
    v0 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
         v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17;

    /* Use the result to prevent the entire function from being optimized away. */
    asm volatile("" : "+r" (v0));
}

/* Trivial main function to ensure the high-pressure function is compiled
 * and called. The return value is irrelevant for coverage. */
int main() {
    high_pressure_region(42);
    return 0;
}
