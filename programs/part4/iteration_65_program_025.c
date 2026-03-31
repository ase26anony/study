/* test-mcf-debug.c
 * Test case for GCC's Min-Cost Flow solver debug output.
 * Compile with: gcc-debug -O3 -funroll-loops -c test-mcf-debug.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Force optimization level for high-pressure function */
__attribute__((noinline, optimize("O3")))
void high_pressure_function(void) {
    /* 18 volatile variables to force register pressure */
    volatile int v0 = 0, v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10, v11 = 11;
    volatile int v12 = 12, v13 = 13, v14 = 14, v15 = 15, v16 = 16, v17 = 17;
    
    /* Additional non-volatile temporaries */
    int t0, t1, t2, t3, t4;
    
    /* Complex control flow with nested loops */
    for (int outer = 0; outer < 4; ++outer) {
        /* Memory barrier to force liveness across it */
        asm volatile("" : : : "memory");
        
        /* Switch with multiple cases creating control flow edges */
        switch (outer & 3) {
            case 0:
                /* Chain dependencies to create data flow */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 + v6;
                v7 = v5 * v8;
                
                /* More operations keeping many vars live */
                t2 = v9 + v10;
                v11 = t2 - v12;
                v13 = v11 * v14;
                v15 = v13 + v16;
                v17 = v15 - v0;
                break;
                
            case 1:
                /* Different operation pattern */
                t0 = v1 - v2;
                t1 = v3 * v4;
                v5 = t0 + t1;
                v6 = v5 - v7;
                v8 = v6 * v9;
                
                t2 = v10 + v11;
                v12 = t2 - v13;
                v14 = v12 * v15;
                v16 = v14 + v17;
                v0 = v16 - v1;
                break;
                
            case 2:
                /* Yet another pattern */
                t0 = v2 + v3;
                t1 = v4 - v5;
                v6 = t0 * t1;
                v7 = v6 + v8;
                v9 = v7 - v10;
                
                t2 = v11 * v12;
                v13 = t2 + v14;
                v15 = v13 - v16;
                v17 = v15 * v0;
                v1 = v17 + v2;
                break;
                
            case 3:
                /* Complex chain using all variables */
                t0 = v3 - v4;
                t1 = v5 * v6;
                t2 = v7 + v8;
                t3 = v9 - v10;
                t4 = v11 * v12;
                
                v13 = t0 + t1;
                v14 = t2 - t3;
                v15 = t4 * v13;
                v16 = v14 + v15;
                v17 = v16 - v0;
                v1 = v17 * v2;
                v3 = v1 + v4;
                break;
        }
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
        
        /* Inner loop with constant bound to encourage unrolling */
        for (int inner = 0; inner < 3; ++inner) {
            /* Mix volatile and non-volatile operations */
            t0 = v0 + v1 + inner;
            v2 = v3 * t0;
            v4 = v5 - v6 + inner;
            v7 = v8 * v9;
            v10 = v11 + v12 - inner;
            
            /* Cross-case dependencies */
            if (inner == 1) {
                v13 = v14 * v15;
                v16 = v17 + v0;
            } else {
                v13 = v14 + v15;
                v16 = v17 - v0;
            }
        }
    }
    
    /* Final use of all variables to extend live ranges */
    t0 = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    t1 = v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(t0), "r"(t1) : "memory");
}

/* Secondary function with different control flow pattern */
__attribute__((noinline, optimize("O3")))
void secondary_pressure(void) {
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    volatile int g = 7, h = 8, i = 9, j = 10, k = 11, l = 12;
    
    /* Diamond-shaped control flow */
    for (int iter = 0; iter < 5; ++iter) {
        if (iter & 1) {
            a = b + c;
            d = e * f;
            g = h - i;
            j = k + l;
        } else {
            a = b - c;
            d = e / (f ? f : 1);
            g = h + i;
            j = k - l;
        }
        
        /* Loop with switch inside */
        for (int inner = 0; inner < 2; ++inner) {
            switch ((iter + inner) & 3) {
                case 0: a = b + c; break;
                case 1: d = e * f; break;
                case 2: g = h - i; break;
                case 3: j = k + l; break;
            }
        }
    }
}

/* Main function that calls pressure functions */
int main(void) {
    high_pressure_function();
    secondary_pressure();
    return 0;
}
