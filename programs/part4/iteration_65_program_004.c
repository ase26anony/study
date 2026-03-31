/* test-mcf-debug.c
 * 
 * This test creates a high register pressure scenario to trigger the
 * min-cost flow solver's debug output in GCC's IRA, specifically to
 * cover the node label printing logic for artificial source/sink nodes
 * (new_exit_index, new_entry_index) in mcf.cc.
 *
 * Compile with a GCC configured with --enable-checking (defines MCF_DEBUG):
 *   gcc-debug -O3 -funroll-loops -c test-mcf-debug.c -o test.o
 *
 * The coverage occurs at compile-time during the register allocation phase.
 */

/* Force noinline to prevent merging with main */
#define NOINLINE __attribute__((noinline))

/* Aggressive optimization on the pressure function */
#define OPTIMIZE_O3 __attribute__((optimize("O3")))

/* Memory barrier to extend live ranges */
#define MEMORY_BARRIER asm volatile("" : : : "memory")

/* Function to create massive register pressure */
NOINLINE OPTIMIZE_O3
static int create_register_pressure(int seed) {
    /* Declare many volatile variables to prevent optimization
     * and force register allocation pressure */
    volatile int v0 = seed + 0;
    volatile int v1 = seed + 1;
    volatile int v2 = seed + 2;
    volatile int v3 = seed + 3;
    volatile int v4 = seed + 4;
    volatile int v5 = seed + 5;
    volatile int v6 = seed + 6;
    volatile int v7 = seed + 7;
    volatile int v8 = seed + 8;
    volatile int v9 = seed + 9;
    volatile int v10 = seed + 10;
    volatile int v11 = seed + 11;
    volatile int v12 = seed + 12;
    volatile int v13 = seed + 13;
    volatile int v14 = seed + 14;
    volatile int v15 = seed + 15;
    volatile int v16 = seed + 16;
    volatile int v17 = seed + 17;
    
    /* Additional non-volatile temporaries */
    int t0, t1, t2, t3, t4;
    
    /* Complex control flow with nested loops */
    for (int i = 0; i < 4; ++i) {
        /* Memory barrier forces variables to be live across it */
        MEMORY_BARRIER;
        
        /* Switch with multiple cases creates control flow edges */
        switch (i % 3) {
            case 0:
                /* Chain arithmetic operations creating data dependencies */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 + v6;
                v7 = v5 * v8;
                v9 = v7 - v10;
                
                /* More chaining */
                t2 = v11 + v12;
                t3 = v13 * v14;
                v15 = t2 - t3;
                v16 = v15 + v17;
                break;
                
            case 1:
                /* Different operation pattern */
                t0 = v1 - v2;
                t1 = v3 * v4;
                v5 = t0 + t1;
                v6 = v5 - v7;
                v8 = v6 * v9;
                v10 = v8 + v11;
                
                t2 = v12 - v13;
                t3 = v14 * v15;
                v16 = t2 + t3;
                v17 = v16 - v0;
                break;
                
            case 2:
                /* Yet another pattern */
                t0 = v2 * v3;
                t1 = v4 + v5;
                v6 = t0 - t1;
                v7 = v6 * v8;
                v9 = v7 + v10;
                v11 = v9 - v12;
                
                t2 = v13 * v14;
                t3 = v15 + v16;
                v17 = t2 - t3;
                v0 = v17 * v1;
                break;
        }
        
        /* Nested conditional to extend live ranges further */
        if (i & 1) {
            t4 = v0 + v1 + v2;
            v3 = t4 * v4;
            v5 = v3 - v6;
        } else {
            t4 = v7 - v8 - v9;
            v10 = t4 * v11;
            v12 = v10 + v13;
        }
        
        /* Another memory barrier */
        MEMORY_BARRIER;
        
        /* Small inner loop to encourage unrolling */
        for (int j = 0; j < 2; ++j) {
            v14 = v14 + v15 + j;
            v16 = v16 * v17 - j;
        }
    }
    
    /* Combine all results to prevent dead code elimination */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17;
    
    return result;
}

/* Main function exists only to make the file compilable and
 * ensure the pressure function is called */
int main(void) {
    int result = create_register_pressure(42);
    
    /* Use result to prevent optimization */
    asm volatile("" : : "r"(result));
    
    return 0;
}
