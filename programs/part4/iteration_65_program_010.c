/* test-mcf-debug.c
 * 
 * A test designed to trigger the uncovered debug dump lines in GCC's
 * mcf.cc (Min-Cost Flow solver) when compiled with a GCC configured
 * with --enable-checking (defining MCF_DEBUG).
 *
 * The function `high_pressure_ira_test` creates a scenario with many
 * simultaneously live variables across complex control flow, forcing
 * IRA to build a dense conflict graph that requires the min-cost flow
 * solver to add artificial source/sink nodes (new_entry_index,
 * new_exit_index). When MCF_DEBUG is defined, the fixup graph dump
 * will print labels for these special nodes via the uncovered code.
 *
 * Compile with a debugging-enabled GCC (e.g., configured with
 * --enable-checking) using:
 *   gcc-debug -O3 -funroll-loops -c test-mcf-debug.c -o test.o
 * or
 *   gcc-debug -O2 -fno-expensive-optimizations -c test-mcf-debug.c -o test.o
 *
 * Coverage is achieved at compile-time within GCC's IRA phase; the
 * resulting binary need not be executed.
 */

/* Prevent inlining to preserve the intended control flow structure */
__attribute__((noinline, optimize("O3")))
void high_pressure_ira_test(int selector) {
    /* Declare 18 volatile int variables to force register pressure.
     * 'volatile' prevents elimination and forces memory traffic. */
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    volatile int v10, v11, v12, v13, v14, v15, v16, v17;

    /* Additional non-volatile temporaries to increase register working set */
    int t0, t1, t2, t3, t4;

    /* Initialize all volatile variables with distinct values */
    v0 = 0;   v1 = 1;   v2 = 2;   v3 = 3;   v4 = 4;   v5 = 5;
    v6 = 6;   v7 = 7;   v8 = 8;   v9 = 9;   v10 = 10; v11 = 11;
    v12 = 12; v13 = 13; v14 = 14; v15 = 15; v16 = 16; v17 = 17;

    /* Memory barrier to force all variables live across it */
    asm volatile("" : : : "memory");

    /* Outer loop with constant bound to encourage unrolling */
    for (int i = 0; i < 4; ++i) {
        /* Complex switch to create control-flow graph with multiple edges */
        switch ((selector + i) % 5) {
            case 0:
                /* Chain arithmetic operations, extending live ranges */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 + v6;
                v7 = v5 | v8;
                /* Barrier to prevent reordering and extend liveness */
                asm volatile("" : : : "memory");
                v9 = v7 ^ v10;
                v11 = v9 + v12;
                break;

            case 1:
                t2 = v13 - v14;
                v15 = t2 * v16;
                v17 = v15 & v0;
                v1 = v17 + v2;
                asm volatile("" : : : "memory");
                v3 = v1 | v4;
                v5 = v3 ^ v6;
                break;

            case 2:
                t3 = v7 * v8;
                v9 = t3 - v10;
                v11 = v9 + v12;
                v13 = v11 & v14;
                asm volatile("" : : : "memory");
                v15 = v13 | v16;
                v17 = v15 ^ v0;
                break;

            case 3:
                t4 = v1 + v2;
                v3 = t4 * v4;
                v5 = v3 - v6;
                v7 = v5 & v8;
                asm volatile("" : : : "memory");
                v9 = v7 | v10;
                v11 = v9 ^ v12;
                break;

            case 4:
                v13 = v14 + v15;
                v16 = v13 * v17;
                v0 = v16 - v1;
                v2 = v0 & v3;
                asm volatile("" : : : "memory");
                v4 = v2 | v5;
                v6 = v4 ^ v7;
                break;
        }

        /* Cross-iteration data flow to increase liveness across loop */
        v8 = v9 + v10;
        v11 = v12 - v13;
        v14 = v15 * v16;
        v17 = v0 ^ v1;

        /* Another memory barrier to ensure variables remain live */
        asm volatile("" : : : "memory");

        /* Additional nested conditional to split live ranges */
        if (i & 1) {
            v2 = v3 + v4;
            v5 = v6 * v7;
        } else {
            v8 = v9 - v10;
            v11 = v12 & v13;
        }
    }

    /* Final use of all variables to keep them live until the end */
    t0 = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
         v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17;

    /* Prevent dead code elimination */
    asm volatile("" : : "r"(t0) : "memory");
}

/* Trivial main to make the file compilable and ensure the test
 * function is referenced (and thus compiled). */
int main() {
    high_pressure_ira_test(42);
    return 0;
}
