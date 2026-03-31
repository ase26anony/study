/* test-mcf-debug.c
 * Test program to trigger MCF_DEBUG code paths in GCC's min-cost flow solver.
 * Compile with: gcc-debug -O3 -funroll-loops -c test-mcf-debug.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Force noinline to prevent optimization from removing the pressure function */
#define NOINLINE __attribute__((noinline))

/* Create register pressure with volatile variables */
#define DECL_VARS \
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9; \
    volatile int v10, v11, v12, v13, v14, v15, v16, v17, v18;

/* Memory barrier to force variables to be live across it */
#define MEMORY_BARRIER asm volatile("" : : : "memory")

/* Function with extreme register pressure and complex control flow */
NOINLINE __attribute__((optimize("O3")))
static int high_pressure_function(int seed) {
    DECL_VARS;
    int i, j, result = seed;
    
    /* Initialize all volatile variables with different values */
    v0 = seed + 1; v1 = seed + 2; v2 = seed + 3; v3 = seed + 4; v4 = seed + 5;
    v5 = seed + 6; v6 = seed + 7; v7 = seed + 8; v8 = seed + 9; v9 = seed + 10;
    v10 = seed + 11; v11 = seed + 12; v12 = seed + 13; v13 = seed + 14;
    v14 = seed + 15; v15 = seed + 16; v16 = seed + 17; v17 = seed + 18;
    v18 = seed + 19;
    
    /* Complex nested loops to create overlapping live ranges */
    for (i = 0; i < 4; i++) {  /* Small constant for unrolling */
        MEMORY_BARRIER;
        
        /* Switch with multiple cases creating complex CFG */
        switch (i % 3) {
            case 0:
                /* Chain computations keeping many variables live */
                v0 = v1 + v2;
                v3 = v4 * v5;
                v6 = v7 - v8;
                v9 = v10 / (v11 + 1);
                v12 = v13 | v14;
                v15 = v16 & v17;
                v18 = v0 + v3 + v6;
                result += v9 + v12 + v15 + v18;
                MEMORY_BARRIER;
                break;
                
            case 1:
                /* Different computation pattern */
                v1 = v2 * v3;
                v4 = v5 + v6;
                v7 = v8 - v9;
                v10 = v11 * v12;
                v13 = v14 | v15;
                v16 = v17 & v18;
                v0 = v1 + v4 + v7;
                result += v10 + v13 + v16 + v0;
                MEMORY_BARRIER;
                break;
                
            case 2:
                /* Yet another pattern */
                v2 = v3 + v4;
                v5 = v6 * v7;
                v8 = v9 - v10;
                v11 = v12 / (v13 + 1);
                v14 = v15 | v16;
                v17 = v18 & v0;
                v1 = v2 + v5 + v8;
                result += v11 + v14 + v17 + v1;
                MEMORY_BARRIER;
                break;
        }
        
        /* Inner loop with more operations */
        for (j = 0; j < 2; j++) {
            /* Mix all variables in complex expressions */
            int t0 = v0 + v1 + v2;
            int t1 = v3 * v4 * v5;
            int t2 = v6 - v7 - v8;
            int t3 = v9 | v10 | v11;
            int t4 = v12 & v13 & v14;
            int t5 = v15 ^ v16 ^ v17;
            
            result += t0 * t1 - t2 + t3 | t4 & t5;
            
            /* Rotate values to extend live ranges */
            int tmp = v0;
            v0 = v1; v1 = v2; v2 = v3; v3 = v4; v4 = v5;
            v5 = v6; v6 = v7; v7 = v8; v8 = v9; v9 = v10;
            v10 = v11; v11 = v12; v12 = v13; v13 = v14;
            v14 = v15; v15 = v16; v16 = v17; v17 = v18;
            v18 = tmp;
            
            MEMORY_BARRIER;
        }
        
        /* Conditional block with more variable usage */
        if (result & 1) {
            v0 = v1 + v2;
            v3 = v4 - v5;
            v6 = v7 * v8;
            result += v0 * v3 / (v6 + 1);
        } else {
            v9 = v10 | v11;
            v12 = v13 & v14;
            v15 = v16 ^ v17;
            result += v9 + v12 - v15;
        }
        
        MEMORY_BARRIER;
    }
    
    /* Final mixing of all variables */
    result = result + v0 - v1 + v2 * v3 - v4 / (v5 + 1) +
             (v6 | v7) & (v8 ^ v9) + v10 - v11 * v12 +
             v13 & v14 | v15 ^ v16 + v17 - v18;
    
    return result;
}

/* Additional pressure function with different pattern */
NOINLINE __attribute__((optimize("O3")))
static int more_pressure(int x) {
    volatile int a0, a1, a2, a3, a4, a5, a6, a7, a8, a9;
    volatile int b0, b1, b2, b3, b4, b5, b6, b7;
    
    a0 = x; a1 = x+1; a2 = x+2; a3 = x+3; a4 = x+4;
    a5 = x+5; a6 = x+6; a7 = x+7; a8 = x+8; a9 = x+9;
    b0 = x+10; b1 = x+11; b2 = x+12; b3 = x+13;
    b4 = x+14; b5 = x+15; b6 = x+16; b7 = x+17;
    
    /* Series of dependent computations */
    for (int k = 0; k < 3; k++) {
        a0 = a1 + a2;
        a3 = a4 * a5;
        a6 = a7 - a8;
        a9 = b0 / (b1 + 1);
        b2 = b3 | b4;
        b5 = b6 & b7;
        
        MEMORY_BARRIER;
        
        b0 = a0 + a3;
        b1 = a6 * a9;
        b3 = b2 + b5;
        
        /* Complex conditional */
        if (k == 1) {
            a1 = a2 * a3;
            a4 = a5 + a6;
            b4 = b5 | b6;
        } else if (k == 2) {
            a7 = a8 - a9;
            b6 = b7 & a0;
            b7 = a1 ^ a2;
        }
        
        MEMORY_BARRIER;
    }
    
    return a0 + a3 + a6 + a9 + b0 + b2 + b5;
}

/* Main function that calls pressure functions */
int main(void) {
    int result = 0;
    
    /* Call pressure functions multiple times with different seeds */
    result += high_pressure_function(42);
    result += more_pressure(100);
    result += high_pressure_function(123);
    result += more_pressure(456);
    
    return result & 0xFF;  /* Return non-zero to prevent dead code elimination */
}
