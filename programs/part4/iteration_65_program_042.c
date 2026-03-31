/* test-mcf-debug.c
 * 
 * This test creates a function with extremely high register pressure
 * and complex control flow to trigger GCC's IRA min-cost flow solver
 * to create a fixup graph with artificial source/sink nodes (new_entry_index,
 * new_exit_index). When compiled with a GCC configured with --enable-checking
 * (defining MCF_DEBUG), the debug dump functions in mcf.cc should be invoked,
 * covering the special node label printing logic.
 *
 * Compile with: gcc-debug -O3 -funroll-loops -c test-mcf-debug.c -o test.o
 * (assuming gcc-debug is a GCC built with internal checking enabled)
 */

/* Prevent inlining to preserve the complex control flow structure */
#define NOINLINE __attribute__((noinline))

/* Force O3 optimization within the high-pressure function */
#define HIGH_PRESSURE_OPT __attribute__((optimize("O3")))

/* Memory barrier to force variables to be live across it */
#define MEMORY_BARRIER() asm volatile("" : : : "memory")

/* Function that creates extreme register pressure */
NOINLINE HIGH_PRESSURE_OPT
static void high_pressure_function(void) {
    /* 18 volatile integer variables to force register allocation */
    volatile int v0 = 0, v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10, v11 = 11;
    volatile int v12 = 12, v13 = 13, v14 = 14, v15 = 15, v16 = 16, v17 = 17;
    
    /* Additional non-volatile temporaries to increase working set */
    int t0, t1, t2, t3, t4;
    
    /* Complex control flow: outer switch with multiple cases */
    int selector = v0;
    
    /* Memory barrier to extend live ranges */
    MEMORY_BARRIER();
    
    /* Loop that will be unrolled, replicating live ranges */
    for (int i = 0; i < 4; i++) {
        /* Switch creates multiple control flow edges */
        switch ((selector + i) % 5) {
            case 0:
                /* Chain of arithmetic operations creating data dependencies */
                t0 = v1 + v2;
                t1 = v3 * v4;
                v5 = t0 - t1;
                t2 = v6 / (v7 + 1);
                v8 = v5 * t2;
                v9 = v8 + v10;
                /* Extend liveness of many variables */
                v11 = v9 | v12;
                v13 = v11 ^ v14;
                break;
                
            case 1:
                /* Different operation pattern using another subset */
                t0 = v15 - v16;
                t1 = v17 * v0;
                v1 = t0 + t1;
                t2 = v2 & v3;
                v4 = v1 | t2;
                v5 = v4 ^ v6;
                v7 = v5 + v8;
                v9 = v7 * v10;
                break;
                
            case 2:
                /* Use most variables in a long dependency chain */
                t0 = v11 + v12;
                t1 = v13 - v14;
                t2 = v15 * v16;
                t3 = v17 / (v0 + 1);
                t4 = t0 + t1;
                v1 = t2 - t3;
                v2 = t4 * v1;
                v3 = v2 + v4;
                v5 = v3 - v6;
                v7 = v5 * v8;
                break;
                
            case 3:
                /* Mix of operations */
                v9 = v10 * v11;
                v12 = v13 + v14;
                v15 = v16 - v17;
                v0 = v9 | v12;
                v1 = v15 ^ v0;
                v2 = v1 + v3;
                v4 = v2 * v5;
                break;
                
            case 4:
                /* All variables used in some way */
                t0 = v6 + v7;
                t1 = v8 * v9;
                t2 = v10 - v11;
                t3 = v12 & v13;
                t4 = v14 | v15;
                v16 = t0 + t1;
                v17 = t2 - t3;
                v0 = t4 ^ v16;
                v1 = v17 + v0;
                v2 = v1 * v3;
                v4 = v2 + v5;
                break;
        }
        
        /* Memory barrier between loop iterations */
        MEMORY_BARRIER();
        
        /* Cross-iteration dependencies to force overlapping live ranges */
        if (i > 0) {
            v0 = v0 + v1;
            v2 = v2 - v3;
            v4 = v4 * v5;
        }
    }
    
    /* Final complex conditional structure */
    if (v0 > v1) {
        t0 = v2 + v3;
        t1 = v4 * v5;
        v6 = t0 - t1;
        
        if (v6 < v7) {
            v8 = v9 + v10;
            v11 = v12 * v13;
        } else {
            v14 = v15 - v16;
            v17 = v0 & v1;
        }
        
        /* Nested loop with different variable usage */
        for (int j = 0; j < 3; j++) {
            v2 = v3 + v4;
            v5 = v6 * v7;
            v8 = v9 - v10;
            MEMORY_BARRIER();
        }
    } else if (v1 > v2) {
        t0 = v3 * v4;
        t1 = v5 + v6;
        v7 = t0 / (t1 + 1);
        
        /* Small unrolled loop */
        v8 = v9 + v10; v11 = v12 * v13; v14 = v15 - v16;
        v8 = v9 + v10; v11 = v12 * v13; v14 = v15 - v16;
    } else {
        /* Use all remaining variables */
        v0 = v1 | v2;
        v3 = v4 ^ v5;
        v6 = v7 & v8;
        v9 = v10 + v11;
        v12 = v13 * v14;
        v15 = v16 - v17;
    }
    
    /* Final memory barrier */
    MEMORY_BARRIER();
    
    /* Use all variables one more time to ensure they're live at the end */
    t0 = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
         v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(t0));
}

/* Main function exists only to call the high-pressure function */
int main(void) {
    high_pressure_function();
    return 0;
}
