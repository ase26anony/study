/* test_mcf_debug.c - Min-Cost Flow Debug Coverage Test
 * 
 * This test creates a function with extreme register pressure and
 * complex control flow to force GCC's IRA to build a fixup graph
 * with artificial source/sink nodes, potentially triggering the
 * debug dump logic in mcf.cc when compiled with MCF_DEBUG enabled.
 *
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_debug.c -o test.o
 * (assuming gcc-debug is GCC configured with --enable-checking)
 */

/* Prevent inlining to preserve the complex control flow structure */
#define NOINLINE __attribute__((noinline))

/* Force O3 optimization on the pressure function */
#define OPTIMIZE_O3 __attribute__((optimize("O3")))

/* Memory barrier to force variables to be live across it */
#define MEMORY_BARRIER() asm volatile("" : : : "memory")

/* Function to create extreme register pressure */
NOINLINE OPTIMIZE_O3
static int create_register_pressure(int seed) {
    /* Declare many volatile variables to prevent optimization
     * and force register allocation */
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
    for (int i = 0; i < 4; i++) {  /* Small constant for potential unrolling */
        /* Memory barrier to extend live ranges */
        MEMORY_BARRIER();
        
        /* Switch with multiple cases creating different data flows */
        switch (i & 3) {
            case 0:
                /* Chain of dependent operations */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                t2 = v5 ^ v6;
                v7 = v4 | t2;
                v8 = v7 << 2;
                v9 = v8 + v10;
                break;
                
            case 1:
                /* Different chain using different variables */
                t0 = v11 - v12;
                t1 = v13 & v14;
                v15 = t0 * t1;
                t2 = v16 | v17;
                v0 = v15 ^ t2;
                v1 = v0 >> 1;
                v2 = v1 + v3;
                break;
                
            case 2:
                /* More complex data flow */
                t0 = v4 * v5;
                t1 = v6 + v7;
                t2 = v8 - v9;
                t3 = v10 & v11;
                t4 = v12 | v13;
                v14 = t0 + t1;
                v15 = t2 * t3;
                v16 = v14 ^ v15;
                v17 = v16 + t4;
                break;
                
            case 3:
                /* Mix all variables together */
                t0 = v0 + v2 + v4 + v6;
                t1 = v1 * v3 * v5 * v7;
                t2 = v8 ^ v9 ^ v10 ^ v11;
                t3 = v12 & v13 & v14 & v15;
                v16 = t0 - t1;
                v17 = t2 | t3;
                v0 = v16 + v17;
                break;
        }
        
        /* Another memory barrier to force more spill/reload */
        MEMORY_BARRIER();
        
        /* Conditional block with overlapping live ranges */
        if (v0 > 100) {
            t0 = v1 + v2;
            v3 = v4 - v5;
            v6 = t0 * v3;
        } else {
            t0 = v7 | v8;
            v9 = v10 ^ v11;
            v12 = t0 & v9;
        }
        
        /* Nested loop to increase pressure */
        for (int j = 0; j < 2; j++) {
            /* Use different subsets of variables */
            v13 = v14 + v15 + j;
            v16 = v17 * v0 * (j + 1);
            
            /* Conditional inside nested loop */
            if (v13 > v16) {
                v1 = v2 + v3;
                v4 = v5 - v6;
            } else {
                v7 = v8 * v9;
                v10 = v11 ^ v12;
            }
        }
    }
    
    /* Final computation using all variables to keep them live until end */
    t0 = v0 + v1 + v2 + v3;
    t1 = v4 * v5 * v6 * v7;
    t2 = v8 ^ v9 ^ v10 ^ v11;
    t3 = v12 & v13 & v14 & v15;
    t4 = v16 | v17;
    
    /* Return a value dependent on all computations */
    return t0 + t1 + t2 + t3 + t4;
}

/* Secondary function with different control flow pattern */
NOINLINE OPTIMIZE_O3
static int more_pressure(int x) {
    volatile int a = x * 2;
    volatile int b = x + 3;
    volatile int c = x ^ 0x55;
    volatile int d = x & 0xFF;
    volatile int e = x | 0xAA;
    volatile int f = x << 1;
    volatile int g = x >> 2;
    volatile int h = ~x;
    
    int result = 0;
    
    /* Loop with early exit conditions */
    for (int i = 0; i < 8; i++) {
        MEMORY_BARRIER();
        
        /* Complex if-else chain */
        if (i == 0) {
            a = b + c;
            d = e * f;
        } else if (i == 1) {
            b = c - d;
            e = f ^ g;
        } else if (i == 2) {
            c = d & e;
            f = g | h;
        } else if (i == 3) {
            d = e + f;
            g = h * a;
        } else if (i == 4) {
            e = f - g;
            h = a ^ b;
        } else if (i == 5) {
            f = g & h;
            a = b | c;
        } else if (i == 6) {
            g = h + a;
            b = c * d;
        } else {
            h = a - b;
            c = d ^ e;
        }
        
        /* Update result based on all variables */
        result += a + b + c + d + e + f + g + h;
    }
    
    return result;
}

/* Main function that calls the pressure functions */
int main(void) {
    int sum = 0;
    
    /* Call pressure functions multiple times with different seeds */
    for (int i = 0; i < 3; i++) {
        sum += create_register_pressure(i * 100);
        sum += more_pressure(i * 50);
    }
    
    /* Return something based on the computations */
    return sum > 0 ? 0 : 1;
}
