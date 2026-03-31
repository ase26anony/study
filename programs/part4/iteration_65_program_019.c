/* test-mcf-debug.c
 * 
 * Test case to trigger uncovered lines in GCC's mcf.cc dump_fixup_edge function.
 * Specifically targets the logic that prints labels for special node indices:
 *   ENTRY, ENTRY'', EXIT, EXIT'', NEW_EXIT, NEW_ENTRY
 * 
 * Compile with a GCC configured with --enable-checking (defines MCF_DEBUG):
 *   gcc-debug -O3 -funroll-loops -c test-mcf-debug.c -o test.o
 * 
 * The high register pressure and complex control flow should force IRA's
 * min-cost flow solver to create artificial source/sink nodes in the fixup
 * graph, which are then dumped when MCF_DEBUG is active.
 */

/* Prevent inlining to preserve the complex control flow structure */
#define NOINLINE __attribute__((noinline))

/* Force O3 optimization on this function to encourage unrolling */
#define AGGRESSIVE_OPT __attribute__((optimize("O3")))

/* Memory barrier to force variables to be live across it */
#define MEMORY_BARRIER() asm volatile("" : : : "memory")

/* Function to create extreme register pressure and complex control flow */
NOINLINE AGGRESSIVE_OPT
static void high_pressure_function(void) {
    /* 18 volatile variables to prevent optimization and increase pressure */
    volatile int v0 = 0, v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10, v11 = 11;
    volatile int v12 = 12, v13 = 13, v14 = 14, v15 = 15, v16 = 16, v17 = 17;
    
    /* Additional non-volatile temporaries to increase register working set */
    int t0, t1, t2, t3, t4, t5;
    
    /* Complex nested loops to create many live ranges */
    for (int i = 0; i < 4; i++) {  /* Small constant bound encourages unrolling */
        MEMORY_BARRIER();
        
        /* Large switch with multiple cases creates complex CFG edges */
        switch (i & 3) {
            case 0:
                /* Chain dependencies to create data flow */
                v0 = v1 + v2;
                v3 = v0 * v4;
                v5 = v3 - v6;
                t0 = v5 + v7;
                v8 = t0 / 2;
                break;
                
            case 1:
                /* Different variable subset */
                v9 = v10 * v11;
                v12 = v9 - v13;
                v14 = v12 + v15;
                t1 = v14 * 3;
                v16 = t1 >> 1;
                break;
                
            case 2:
                /* More chained operations */
                v1 = v17 + v0;
                v2 = v1 * v3;
                v4 = v2 - v5;
                t2 = v4 + v6;
                v7 = t2 * 2;
                break;
                
            case 3:
                /* Use remaining variables */
                v10 = v8 + v9;
                v11 = v10 * v12;
                v13 = v11 - v14;
                t3 = v13 + v15;
                v17 = t3 / 4;
                break;
        }
        
        MEMORY_BARRIER();
        
        /* Conditional blocks with overlapping live ranges */
        if (v0 > v1) {
            v2 = v3 + v4;
            v5 = v6 * v7;
            t4 = v8 - v9;
            v10 = t4 + 1;
        } else {
            v11 = v12 + v13;
            v14 = v15 * v16;
            t5 = v17 - v0;
            v1 = t5 - 1;
        }
        
        MEMORY_BARRIER();
        
        /* Nested loop to extend live ranges further */
        for (int j = 0; j < 2; j++) {
            /* Mix all variables to maximize interference */
            v0 = v0 + v1 + v2;
            v3 = v3 + v4 + v5;
            v6 = v6 + v7 + v8;
            v9 = v9 + v10 + v11;
            v12 = v12 + v13 + v14;
            v15 = v15 + v16 + v17;
            
            /* Force spilling with memory barrier */
            MEMORY_BARRIER();
        }
        
        /* Another conditional chain */
        if (v0 & 1) {
            v1 = v2 + v3;
        } else if (v4 & 2) {
            v5 = v6 + v7;
        } else if (v8 & 4) {
            v9 = v10 + v11;
        } else {
            v12 = v13 + v14;
        }
    }
    
    /* Final use of all variables to keep them live until the end */
    v0 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + 
         v11 + v12 + v13 + v14 + v15 + v16 + v17;
    
    /* Prevent dead code elimination */
    MEMORY_BARRIER();
}

/* Main function exists only to make the file compilable and call our function */
int main(void) {
    /* Call the high-pressure function */
    high_pressure_function();
    
    /* Return a value based on the computation to prevent optimization */
    return 0;
}
