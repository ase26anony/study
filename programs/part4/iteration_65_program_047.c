/* test_mcf_coverage.c
 * Designed to trigger debug output in GCC's min-cost flow solver
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_coverage.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Prevent inlining to preserve register pressure structure */
#define NOINLINE __attribute__((noinline))
#define OPTIMIZE_O3 __attribute__((optimize("O3")))

/* Memory barrier to force register liveness */
#define MEMORY_BARRIER() asm volatile("" : : : "memory")

/* Function to create extreme register pressure */
NOINLINE OPTIMIZE_O3
static int create_register_pressure(int seed) {
    /* 18 volatile variables to force register allocation */
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
    
    /* Additional non-volatile temporaries */
    int t0, t1, t2, t3, t4;
    
    /* Complex control flow with nested loops */
    for (int i = 0; i < 4; i++) {  /* Small constant for unrolling */
        /* Switch creates multiple control flow edges */
        switch (i & 3) {
            case 0:
                /* Chain dependencies to create data flow */
                t0 = v0 + v1;
                v2 = t0 * v3;
                v4 = v2 - v5;
                MEMORY_BARRIER();
                
                t1 = v6 * v7;
                v8 = t1 + v9;
                v10 = v8 / (v11 + 1);
                break;
                
            case 1:
                /* Different variable subset */
                t2 = v12 ^ v13;
                v14 = t2 | v15;
                v16 = v14 & v17;
                MEMORY_BARRIER();
                
                v0 = v1 + v16;
                v3 = v4 * v2;
                break;
                
            case 2:
                /* More arithmetic chains */
                t3 = v5 * v6;
                v7 = t3 + v8;
                v9 = v7 - v10;
                MEMORY_BARRIER();
                
                v11 = v12 ^ v13;
                v14 = v15 | v16;
                break;
                
            case 3:
                /* Use all variables in complex expressions */
                t4 = v0 * v1 + v2 * v3 - v4 * v5;
                v6 = t4 ^ v7;
                v8 = v6 | v9;
                v10 = v8 & v11;
                MEMORY_BARRIER();
                
                v12 = v13 + v14;
                v15 = v16 * v17;
                v0 = v15 - v12;
                break;
        }
        
        /* Cross-iteration dependencies to extend live ranges */
        if (i > 0) {
            v1 = v2 + v3;
            v4 = v5 * v6;
            v7 = v8 - v9;
            MEMORY_BARRIER();
        }
        
        /* Nested conditional blocks */
        if (v0 > 100) {
            v10 = v11 * v12;
            v13 = v14 + v15;
        } else {
            v16 = v17 ^ v0;
            v1 = v2 | v3;
        }
        
        /* Another memory barrier */
        MEMORY_BARRIER();
        
        /* Small inner loop for additional pressure */
        for (int j = 0; j < 2; j++) {
            v4 = v5 + v6;
            v7 = v8 * v9;
            v10 = v11 - v12;
        }
    }
    
    /* Final computation using all variables */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17;
    
    return result;
}

/* Secondary function with different control flow pattern */
NOINLINE OPTIMIZE_O3
static int more_pressure(int x) {
    volatile int a = x * 2;
    volatile int b = x + 3;
    volatile int c = x - 1;
    volatile int d = x / 2;
    volatile int e = x ^ 0xFF;
    volatile int f = x | 0x0F;
    volatile int g = x & 0xF0;
    volatile int h = x << 2;
    volatile int i = x >> 1;
    volatile int j = ~x;
    
    /* Complex if-else chain */
    if (x > 1000) {
        a = b + c;
        d = e * f;
    } else if (x > 500) {
        g = h | i;
        j = a ^ b;
    } else if (x > 100) {
        c = d - e;
        f = g / h;
    } else {
        i = j & a;
        b = c | d;
    }
    
    MEMORY_BARRIER();
    
    /* Loop with switch inside */
    for (int k = 0; k < 3; k++) {
        switch (k) {
            case 0: a = b + 1; break;
            case 1: c = d * 2; break;
            case 2: e = f - 3; break;
        }
        MEMORY_BARRIER();
    }
    
    return a + b + c + d + e + f + g + h + i + j;
}

/* Main function to ensure compilation */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Call pressure functions multiple times with different seeds */
    for (int i = 0; i < 10; i++) {
        result += create_register_pressure(i * 100);
        result += more_pressure(i * 50);
    }
    
    return result % 256;  /* Prevent compiler from optimizing everything away */
}
