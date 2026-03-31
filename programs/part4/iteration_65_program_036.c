/* test-mcf-debug.c
 * Test program to trigger MCF_DEBUG output in GCC's min-cost flow solver.
 * Compile with: gcc-debug -O3 -funroll-loops -c test-mcf-debug.c -o test.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Force noinline to prevent optimization from removing the pressure function */
#define NOINLINE __attribute__((noinline))

/* Force O3 optimization specifically for the pressure function */
#define HIGH_PRESSURE NOINLINE __attribute__((optimize("O3")))

/* Memory barrier to force variables to be live across it */
#define MEMORY_BARRIER asm volatile("" : : : "memory")

/* High register pressure function with complex control flow */
HIGH_PRESSURE int create_register_pressure(int seed) {
    /* 18 volatile variables to prevent optimization and increase pressure */
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
    for (int i = 0; i < 4; i++) {  /* Small constant loop for unrolling */
        MEMORY_BARRIER;
        
        /* Switch with multiple cases creating different live ranges */
        switch (i & 3) {
            case 0:
                /* Chain dependencies to create data flow */
                t0 = v0 + v1;
                t1 = v2 * v3;
                v4 = t0 - t1;
                v5 = v4 + v6;
                v7 = v5 * v8;
                MEMORY_BARRIER;
                v9 = v7 + v10;
                v11 = v9 - v12;
                break;
                
            case 1:
                /* Different variable usage pattern */
                t2 = v13 + v14;
                t3 = v15 * v16;
                v0 = t2 - t3;
                v1 = v0 + v17;
                v2 = v1 * v3;
                MEMORY_BARRIER;
                v4 = v2 + v5;
                v6 = v4 - v7;
                v8 = v6 * v9;
                break;
                
            case 2:
                /* More complex chains */
                t4 = v10 + v11;
                v12 = t4 * v13;
                v14 = v12 - v15;
                v16 = v14 + v17;
                v0 = v16 * v1;
                MEMORY_BARRIER;
                v2 = v0 + v3;
                v4 = v2 - v5;
                v6 = v4 * v7;
                v8 = v6 + v9;
                break;
                
            case 3:
                /* Use all variables in a long dependency chain */
                v10 = v11 + v12;
                v13 = v10 * v14;
                v15 = v13 - v16;
                v17 = v15 + v0;
                v1 = v17 * v2;
                MEMORY_BARRIER;
                v3 = v1 + v4;
                v5 = v3 - v6;
                v7 = v5 * v8;
                v9 = v7 + v10;
                v11 = v9 - v12;
                break;
        }
        
        MEMORY_BARRIER;
        
        /* Conditional block extending live ranges */
        if (i & 1) {
            t0 = v0 + v2 + v4 + v6 + v8;
            v1 = t0 * v3;
            v5 = v1 + v7;
            v9 = v5 - v11;
        } else {
            t1 = v1 + v3 + v5 + v7 + v9;
            v0 = t1 * v2;
            v4 = v0 + v6;
            v8 = v4 - v10;
        }
        
        MEMORY_BARRIER;
        
        /* Nested loop to increase pressure further */
        for (int j = 0; j < 2; j++) {
            /* Mix variables across iterations */
            v12 = v13 + v14 + j;
            v15 = v16 * v17 - j;
            v0 = v12 + v15;
            v1 = v0 * (i + j);
            
            /* Another memory barrier inside nested loop */
            MEMORY_BARRIER;
        }
    }
    
    /* Final computation using all variables */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17;
    
    MEMORY_BARRIER;
    
    return result;
}

/* Secondary function with different control flow pattern */
NOINLINE int additional_pressure(int x) {
    volatile int a = x * 2;
    volatile int b = x * 3;
    volatile int c = x * 4;
    volatile int d = x * 5;
    volatile int e = x * 6;
    volatile int f = x * 7;
    
    /* If-else chain creating multiple edges in CFG */
    if (x > 100) {
        a = b + c;
        d = e * f;
    } else if (x > 50) {
        b = c + d;
        e = f * a;
    } else if (x > 25) {
        c = d + e;
        f = a * b;
    } else {
        d = e + f;
        a = b * c;
    }
    
    /* Small unrolled loop */
    for (int i = 0; i < 3; i++) {
        a = a + b + i;
        c = c * d - i;
        MEMORY_BARRIER;
    }
    
    return a + b + c + d + e + f;
}

/* Main function to ensure everything gets compiled */
int main() {
    int sum = 0;
    
    /* Call pressure functions multiple times with different seeds */
    for (int i = 0; i < 10; i++) {
        sum += create_register_pressure(i);
        sum += additional_pressure(i);
    }
    
    return sum % 256;  /* Return non-zero to prevent optimization */
}
