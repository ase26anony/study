/* test_mcf_coverage.c
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_coverage.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Force noinline to prevent optimization from removing the function */
#define NOINLINE __attribute__((noinline))

/* High register pressure function with complex control flow */
NOINLINE __attribute__((optimize("O3")))
int high_pressure_function(int seed) {
    /* Declare many volatile variables to prevent optimization */
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
    
    /* Memory barrier to force variables to be live across it */
    asm volatile("" : : : "memory");
    
    /* Complex nested loops to create extended live ranges */
    for (int i = 0; i < 4; i++) {  /* Constant bound encourages unrolling */
        /* Switch statement creates complex control flow */
        switch (i % 5) {
            case 0:
                /* Chain of dependent operations */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 + v6;
                /* Memory barrier */
                asm volatile("" : : : "memory");
                break;
                
            case 1:
                /* Different dependency chain */
                t2 = v7 ^ v8;
                t3 = v9 | v10;
                v11 = t2 & t3;
                v12 = v11 << 2;
                /* Memory barrier */
                asm volatile("" : : : "memory");
                break;
                
            case 2:
                /* Cross-case variable usage */
                v13 = v4 + v12;
                v14 = v5 - v11;
                t4 = v13 * v14;
                v15 = t4 / 3;
                /* Memory barrier */
                asm volatile("" : : : "memory");
                break;
                
            case 3:
                /* More complex operations */
                v16 = (v0 * v7) + (v1 * v8);
                v17 = (v2 * v9) - (v3 * v10);
                /* Memory barrier */
                asm volatile("" : : : "memory");
                break;
                
            case 4:
                /* Use all variables in a complex expression */
                v0 = v1 + v2 + v3 + v4;
                v5 = v6 + v7 + v8 + v9;
                v10 = v11 + v12 + v13 + v14;
                v15 = v16 + v17 + v0 + v5;
                /* Memory barrier */
                asm volatile("" : : : "memory");
                break;
        }
        
        /* Additional operations that keep variables live across iterations */
        if (i & 1) {
            v0 = v0 ^ v15;
            v1 = v1 | v16;
        } else {
            v2 = v2 & v17;
            v3 = v3 ^ v0;
        }
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
        
        /* Nested conditional to create more control flow edges */
        for (int j = 0; j < 2; j++) {
            if (j == 0) {
                v4 = v4 + v1;
                v5 = v5 - v2;
            } else {
                v6 = v6 * v3;
                v7 = v7 / (v4 + 1);
            }
            
            /* Small conditional inside nested loop */
            int temp = v8 + v9;
            v10 = (temp > 100) ? v10 + 1 : v10 - 1;
        }
    }
    
    /* Final computation using all variables */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17;
    
    /* One more memory barrier before return */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Secondary function with different control flow pattern */
NOINLINE __attribute__((optimize("O2")))
int secondary_pressure_function(int x) {
    volatile int a = x * 2;
    volatile int b = x * 3;
    volatile int c = x * 4;
    volatile int d = x * 5;
    volatile int e = x * 6;
    volatile int f = x * 7;
    volatile int g = x * 8;
    volatile int h = x * 9;
    
    /* Complex if-else chain */
    if (x > 0) {
        a = b + c;
        d = e - f;
        asm volatile("" : : : "memory");
    } else if (x < 0) {
        g = h * a;
        b = c / d;
        asm volatile("" : : : "memory");
    } else {
        e = f ^ g;
        h = a | b;
        asm volatile("" : : : "memory");
    }
    
    /* Loop with early exit */
    for (int i = 0; i < 8; i++) {
        if (i == 4) break;
        a = a + i;
        b = b - i;
        asm volatile("" : : : "memory");
    }
    
    return a + b + c + d + e + f + g + h;
}

/* Main function to ensure everything gets compiled */
int main(int argc, char **argv) {
    int result1 = high_pressure_function(argc);
    int result2 = secondary_pressure_function(argc);
    return result1 + result2;
}
