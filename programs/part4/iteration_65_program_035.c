/* test_mcf_debug.c
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_debug.c -o test_mcf_debug.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Prevent optimizations from eliminating variables */
#define KEEP_ALIVE(var) asm volatile("" : : "r"(var) :)

/* Memory barrier to force register spills */
#define MEMORY_BARRIER() asm volatile("" : : : "memory")

/* Function attribute to ensure optimization and prevent inlining */
#define HIGH_PRESSURE __attribute__((noinline, optimize("O3")))

/* High register pressure function */
HIGH_PRESSURE
int create_register_pressure(int seed) {
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
    
    /* Complex control flow with nested loops */
    for (int i = 0; i < 4; i++) {
        MEMORY_BARRIER();
        
        /* Switch creates multiple control flow edges */
        switch (i % 5) {
            case 0:
                /* Chain dependencies to extend live ranges */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 + v6;
                v7 = v5 * v8;
                v9 = v7 - v10;
                break;
                
            case 1:
                t0 = v11 + v12;
                t1 = v13 * v14;
                t2 = v15 - v16;
                v0 = t0 + t1;
                v1 = t2 * v17;
                v2 = v0 - v1;
                v3 = v2 + v4;
                break;
                
            case 2:
                t0 = v5 * v6;
                t1 = v7 + v8;
                t2 = v9 - v10;
                t3 = v11 * v12;
                v13 = t0 + t1;
                v14 = t2 - t3;
                v15 = v13 * v14;
                v16 = v15 + v17;
                break;
                
            case 3:
                t0 = v0 * v1;
                t1 = v2 + v3;
                t2 = v4 - v5;
                t3 = v6 * v7;
                t4 = v8 + v9;
                v10 = t0 - t1;
                v11 = t2 * t3;
                v12 = t4 - v10;
                v13 = v11 + v12;
                break;
                
            case 4:
                /* More complex dependency chain */
                t0 = v14 + v15;
                t1 = v16 * v17;
                t2 = v0 - v1;
                t3 = v2 + v3;
                t4 = v4 * v5;
                v6 = t0 - t1;
                v7 = t2 + t3;
                v8 = t4 - v6;
                v9 = v7 * v8;
                v10 = v9 + v11;
                break;
        }
        
        MEMORY_BARRIER();
        
        /* Cross-iteration dependencies to force liveness across loops */
        if (i > 0) {
            v12 = v12 + v13;
            v14 = v14 - v15;
            v16 = v16 * v17;
        }
        
        /* Conditional block with variable usage */
        if (v0 > v1) {
            t0 = v2 + v3;
            v4 = v4 * t0;
        } else {
            t0 = v5 - v6;
            v7 = v7 / (t0 ? t0 : 1);
        }
        
        /* Nested loop to increase pressure */
        for (int j = 0; j < 2; j++) {
            v8 = v8 + v9;
            v10 = v10 - v11;
            v12 = v12 * v13;
            v14 = v14 + v15;
        }
    }
    
    /* Final computation using all variables */
    t0 = v0 + v1 + v2 + v3;
    t1 = v4 * v5 * v6;
    t2 = v7 - v8 - v9;
    t3 = v10 + v11 + v12;
    t4 = v13 * v14 * v15;
    
    int result = t0 + t1 + t2 + t3 + t4 + v16 + v17;
    
    /* Force all variables to be considered live at end */
    KEEP_ALIVE(v0); KEEP_ALIVE(v1); KEEP_ALIVE(v2); KEEP_ALIVE(v3);
    KEEP_ALIVE(v4); KEEP_ALIVE(v5); KEEP_ALIVE(v6); KEEP_ALIVE(v7);
    KEEP_ALIVE(v8); KEEP_ALIVE(v9); KEEP_ALIVE(v10); KEEP_ALIVE(v11);
    KEEP_ALIVE(v12); KEEP_ALIVE(v13); KEEP_ALIVE(v14); KEEP_ALIVE(v15);
    KEEP_ALIVE(v16); KEEP_ALIVE(v17);
    
    return result;
}

/* Secondary function with different control flow pattern */
HIGH_PRESSURE
int more_pressure(int x) {
    volatile int a = x * 2;
    volatile int b = x + 3;
    volatile int c = x - 4;
    volatile int d = x * x;
    volatile int e = x + 10;
    volatile int f = x - 7;
    volatile int g = x * 3;
    volatile int h = x + 15;
    
    int sum = 0;
    
    /* Loop with early exits to create complex CFG */
    for (int i = 0; i < 8; i++) {
        if (i == 3) continue;
        if (i == 5) break;
        
        switch (i) {
            case 0: sum += a + b; break;
            case 1: sum += c * d; break;
            case 2: sum += e - f; break;
            case 4: sum += g / 2; break;
            case 6: sum += h * a; break;
            case 7: sum += b - c; break;
        }
        
        /* Cross-iteration variable usage */
        a = a + 1;
        b = b - 1;
        c = c * 2;
        d = d / 2;
    }
    
    return sum;
}

/* Main function to ensure compilation */
int main() {
    int result1 = create_register_pressure(42);
    int result2 = more_pressure(100);
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(result1), "r"(result2));
    
    return (result1 + result2) > 0 ? 0 : 1;
}
