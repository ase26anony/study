/* test_mcf_debug.c
 * 
 * Test program to trigger MCF_DEBUG code paths in GCC's min-cost flow solver.
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_debug.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Force noinline to prevent optimization from removing the pressure function */
#define NOINLINE __attribute__((noinline))

/* Create high register pressure with volatile variables */
NOINLINE __attribute__((optimize("O3")))
static int high_pressure_function(int seed) {
    /* 18 volatile variables to force register pressure */
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
    
    /* Memory barrier to force liveness across operations */
    asm volatile("" : : : "memory");
    
    /* Complex control flow with nested loops */
    for (int i = 0; i < 4; i++) {  /* Small constant for potential unrolling */
        /* Switch with multiple cases creating control flow edges */
        switch (i % 5) {
            case 0:
                /* Chain dependencies to create data flow */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 + v6;
                asm volatile("" : : : "memory");  /* Force liveness */
                break;
                
            case 1:
                t2 = v7 * v8;
                t3 = v9 - v10;
                v11 = t2 + t3;
                v12 = v11 * v13;
                asm volatile("" : : : "memory");
                break;
                
            case 2:
                t4 = v14 + v15;
                v16 = t4 * v17;
                v0 = v16 - v1;  /* Reuse v0 to extend live range */
                asm volatile("" : : : "memory");
                break;
                
            case 3:
                /* Cross-case dependencies */
                v2 = v5 + v12;
                v3 = v2 * v16;
                v6 = v3 - v0;
                asm volatile("" : : : "memory");
                break;
                
            case 4:
                /* Use all variables in a complex expression */
                v7 = (v0 * v1) + (v2 * v3) - (v4 * v5) + (v6 * v7);
                v8 = (v8 * v9) - (v10 * v11) + (v12 * v13) - (v14 * v15);
                v9 = v16 + v17 + v7 + v8;
                asm volatile("" : : : "memory");
                break;
        }
        
        /* Nested conditional to increase control flow complexity */
        if (i & 1) {
            t0 = v0 + v2 + v4 + v6;
            v1 = t0 * v8;
            asm volatile("" : : : "memory");
        } else {
            t1 = v1 + v3 + v5 + v7;
            v0 = t1 * v9;
            asm volatile("" : : : "memory");
        }
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
        
        /* Small inner loop to increase live range spans */
        for (int j = 0; j < 2; j++) {
            v10 = v10 + v11 + v12;
            v13 = v13 * v14 - v15;
            asm volatile("" : : : "memory");
        }
    }
    
    /* Final computation using all variables */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17;
    
    /* One more memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Secondary function with different pressure pattern */
NOINLINE __attribute__((optimize("O3")))
static int alternate_pressure_function(int base) {
    volatile int w0 = base * 2;
    volatile int w1 = base * 3;
    volatile int w2 = base * 4;
    volatile int w3 = base * 5;
    volatile int w4 = base * 6;
    volatile int w5 = base * 7;
    volatile int w6 = base * 8;
    volatile int w7 = base * 9;
    volatile int w8 = base * 10;
    volatile int w9 = base * 11;
    
    /* Complex if-else chain */
    if (base & 1) {
        w0 = w1 + w2;
        w3 = w4 * w5;
        asm volatile("" : : : "memory");
    } else if (base & 2) {
        w6 = w7 - w8;
        w9 = w0 * w6;
        asm volatile("" : : : "memory");
    } else {
        w1 = w2 * w3;
        w4 = w5 + w6;
        asm volatile("" : : : "memory");
    }
    
    return w0 + w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8 + w9;
}

/* Main function that calls pressure functions */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Call high pressure function multiple times with different seeds */
    for (int i = 0; i < 3; i++) {
        result += high_pressure_function(i * 100);
        result += alternate_pressure_function(i * 50);
    }
    
    return result & 0xFF;  /* Return non-zero to be safe */
}
