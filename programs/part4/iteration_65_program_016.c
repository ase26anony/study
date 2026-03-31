/* test_mcf_coverage.c
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_coverage.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Prevent optimizations from eliminating critical code */
#define BARRIER() asm volatile("" : : : "memory")

/* High-pressure function with complex control flow */
__attribute__((noinline, optimize("O3")))
int high_pressure_function(int seed) {
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
    
    /* Complex nested control flow with loops */
    for (int i = 0; i < 4; i++) {  /* Small constant for unrolling */
        BARRIER();  /* Force variables live across barrier */
        
        /* Switch creates multiple control flow edges */
        switch ((seed + i) % 5) {
            case 0:
                /* Chain dependencies to extend live ranges */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 + v6;
                v7 = v5 * v8;
                BARRIER();
                v9 = v7 + v10;
                v11 = v9 - v12;
                break;
                
            case 1:
                t0 = v1 + v3;
                t1 = v5 * v7;
                v0 = t0 - t1;
                v2 = v0 + v4;
                v6 = v2 * v8;
                BARRIER();
                v10 = v6 + v12;
                v14 = v10 - v16;
                break;
                
            case 2:
                t0 = v2 + v4;
                t1 = v6 * v8;
                v1 = t0 - t1;
                v3 = v1 + v5;
                v7 = v3 * v9;
                BARRIER();
                v11 = v7 + v13;
                v15 = v11 - v17;
                break;
                
            case 3:
                t0 = v3 + v5;
                t1 = v7 * v9;
                v2 = t0 - t1;
                v4 = v2 + v6;
                v8 = v4 * v10;
                BARRIER();
                v12 = v8 + v14;
                v16 = v12 - v0;
                break;
                
            case 4:
                t0 = v4 + v6;
                t1 = v8 * v10;
                v3 = t0 - t1;
                v5 = v3 + v7;
                v9 = v5 * v11;
                BARRIER();
                v13 = v9 + v15;
                v17 = v13 - v1;
                break;
        }
        
        BARRIER();
        
        /* Cross-case variable usage to increase interference */
        if (i % 2 == 0) {
            t2 = v0 + v2 + v4;
            t3 = v6 + v8 + v10;
            v12 = t2 * t3;
        } else {
            t2 = v1 + v3 + v5;
            t3 = v7 + v9 + v11;
            v13 = t2 * t3;
        }
        
        /* More arithmetic chains */
        t4 = v14 + v15;
        v16 = t4 * v17;
        v0 = v16 + seed;
        
        BARRIER();
    }
    
    /* Final computation using all variables */
    t0 = v0 + v1 + v2 + v3;
    t1 = v4 + v5 + v6 + v7;
    t2 = v8 + v9 + v10 + v11;
    t3 = v12 + v13 + v14 + v15;
    t4 = v16 + v17;
    
    return t0 + t1 + t2 + t3 + t4;
}

/* Secondary function with different pressure pattern */
__attribute__((noinline, optimize("O3")))
int secondary_pressure(int base) {
    volatile int a0 = base * 2;
    volatile int a1 = base * 3;
    volatile int a2 = base * 4;
    volatile int a3 = base * 5;
    volatile int a4 = base * 6;
    volatile int a5 = base * 7;
    volatile int a6 = base * 8;
    volatile int a7 = base * 9;
    
    int sum = 0;
    
    /* Loop with if-else chain */
    for (int j = 0; j < 3; j++) {
        BARRIER();
        
        if (j == 0) {
            a0 = a1 + a2;
            a3 = a4 * a5;
            sum += a0 - a3;
        } else if (j == 1) {
            a1 = a2 + a3;
            a4 = a5 * a6;
            sum += a1 - a4;
        } else {
            a2 = a3 + a4;
            a5 = a6 * a7;
            sum += a2 - a5;
        }
        
        BARRIER();
        
        /* Cross usage */
        a6 = a0 + a1 + a2;
        a7 = a3 + a4 + a5;
    }
    
    return sum + a6 + a7;
}

/* Main function to ensure compilation */
int main() {
    int result = 0;
    
    /* Call high-pressure functions multiple times with different seeds */
    for (int k = 0; k < 2; k++) {
        result += high_pressure_function(k * 100);
        BARRIER();
        result += secondary_pressure(k * 50);
    }
    
    return result % 256;  /* Prevent elimination of computation */
}
