/* test-mcf-debug.c
 * 
 * This test aims to trigger the debug dump lines in GCC's min-cost flow solver
 * (mcf.cc) that handle special node indices (ENTRY_BLOCK, EXIT_BLOCK,
 * new_exit_index, new_entry_index). The function `high_pressure_region`
 * creates a scenario with high register pressure and complex control flow
 * that forces IRA to build a fixup graph requiring artificial source/sink nodes.
 *
 * Compile with a GCC configured with --enable-checking (defining MCF_DEBUG):
 *   gcc-debug -O3 -funroll-loops -c test-mcf-debug.c -o test.o
 */

/* Prevent inlining to preserve the structure of the high-pressure region */
__attribute__((noinline, optimize("O3")))
void high_pressure_region(int seed) {
    /* Declare many volatile variables to force register pressure.
     * Volatile prevents elimination and forces memory traffic. */
    volatile int v0 = seed + 1;
    volatile int v1 = seed + 2;
    volatile int v2 = seed + 3;
    volatile int v3 = seed + 4;
    volatile int v4 = seed + 5;
    volatile int v5 = seed + 6;
    volatile int v6 = seed + 7;
    volatile int v7 = seed + 8;
    volatile int v8 = seed + 9;
    volatile int v9 = seed + 10;
    volatile int v10 = seed + 11;
    volatile int v11 = seed + 12;
    volatile int v12 = seed + 13;
    volatile int v13 = seed + 14;
    volatile int v14 = seed + 15;
    volatile int v15 = seed + 16;
    volatile int v16 = seed + 17;
    volatile int v17 = seed + 18;
    /* Total: 18 volatile variables */
    
    /* Additional non-volatile temporaries to increase working set */
    int t1, t2, t3, t4;
    
    /* Memory barrier to force liveness across it */
    asm volatile("" : : : "memory");
    
    /* Complex loop to encourage unrolling and extend live ranges */
    for (int i = 0; i < 4; ++i) {
        /* Chain arithmetic operations to create data dependencies */
        t1 = v0 + v1;
        t2 = v2 * v3;
        v4 = t1 - t2;
        v5 = v4 ^ v6;
        
        /* Switch with multiple cases to create control-flow edges */
        switch (i) {
            case 0:
                v7 = v5 | v8;
                v9 = v7 << 2;
                break;
            case 1:
                v10 = v9 + v11;
                v12 = v10 * 3;
                break;
            case 2:
                v13 = v12 - v14;
                v15 = v13 / 2;
                break;
            case 3:
                v16 = v15 ^ v17;
                v0 = v16 + i;  /* Modify v0 to create loop-carried dependency */
                break;
            default:
                v1 = v2;  /* Unreachable but adds to CFG complexity */
                break;
        }
        
        /* More arithmetic chains using different variable subsets */
        t3 = v3 + v4 + v5;
        t4 = v6 * v7 * v8;
        v9 = t3 - t4;
        v10 = v9 & v11;
        
        /* Another memory barrier to split live ranges */
        asm volatile("" : : : "memory");
        
        /* Nested conditional to increase CFG complexity */
        if (v10 > 100) {
            v11 = v12 + v13;
            v14 = v11 * v15;
        } else if (v10 < 50) {
            v12 = v13 - v14;
            v15 = v12 / 3;
        } else {
            v13 = v14 | v15;
            v16 = v13 ^ 0xFF;
        }
        
        /* Use all variables in a complex expression to keep them live */
        v17 = (v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + 
               v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16) % 256;
    }
    
    /* Final use of all variables to extend liveness to function end */
    asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4),
                      "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9),
                      "r"(v10), "r"(v11), "r"(v12), "r"(v13), "r"(v14),
                      "r"(v15), "r"(v16), "r"(v17) : "memory");
}

/* Main function exists only to make the file compilable and call the
 * high-pressure function. The return value is irrelevant for coverage. */
int main() {
    high_pressure_region(42);
    return 0;
}
