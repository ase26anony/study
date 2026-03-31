/* test_mcf_debug.c
 * Compile with: gcc-debug -O3 -funroll-loops -c test_mcf_debug.c -o test_mcf_debug.o
 * Requires GCC built with --enable-checking (defines MCF_DEBUG)
 */

/* Force noinline to prevent optimization across function boundaries */
#define NOINLINE __attribute__((noinline))

/* High register pressure function with complex control flow */
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
    
    /* Memory barrier to force liveness across it */
    asm volatile("" : : : "memory");
    
    /* Complex nested control flow with loops */
    for (int i = 0; i < 4; i++) {  /* Constant bound encourages unrolling */
        /* Switch creates multiple control flow edges */
        switch ((v0 + i) % 5) {
            case 0:
                /* Chain of dependent operations */
                t0 = v1 + v2;
                t1 = v3 * v4;
                v5 = t0 - t1;
                v6 = v5 * v7;
                /* Memory barrier */
                asm volatile("" : : : "memory");
                v8 = v6 + v9;
                v10 = v8 - v11;
                break;
                
            case 1:
                /* Different operation chain */
                t2 = v12 * v13;
                t3 = v14 + v15;
                v16 = t2 / (t3 + 1);
                v17 = v16 * v0;
                /* Force cross-case liveness */
                v1 = v17 + v2;
                v3 = v1 - v4;
                break;
                
            case 2:
                /* More complex data flow */
                t4 = v5 * v6;
                v7 = t4 + v8;
                v9 = v7 - v10;
                v11 = v9 * v12;
                v13 = v11 + v14;
                /* Memory barrier */
                asm volatile("" : : : "memory");
                break;
                
            case 3:
                /* Interleaved operations */
                v15 = v16 + v17;
                v0 = v15 * v1;
                v2 = v0 - v3;
                v4 = v2 + v5;
                v6 = v4 * v7;
                break;
                
            case 4:
                /* All variables used together */
                t0 = v0 + v1 + v2 + v3;
                t1 = v4 * v5 * v6;
                t2 = v7 - v8 - v9;
                t3 = v10 + v11 + v12;
                t4 = v13 * v14 * v15;
                v16 = t0 + t1 + t2 + t3 + t4;
                v17 = v16 % (v0 + 1);
                break;
        }
        
        /* Cross-iteration dependencies */
        if (i > 0) {
            v0 = v0 + v17;
            v1 = v1 + v16;
            v2 = v2 + v15;
        }
        
        /* Nested conditional */
        if (v0 > 100) {
            v3 = v3 * 2;
            v4 = v4 / 2;
            /* Another memory barrier */
            asm volatile("" : : : "memory");
        } else {
            v5 = v5 + v6;
            v7 = v7 - v8;
        }
        
        /* Small inner loop */
        for (int j = 0; j < 2; j++) {
            v9 = v9 + v10;
            v11 = v11 - v12;
            /* Create data dependencies */
            v13 = v9 * v11;
            v14 = v13 + j;
        }
    }
    
    /* Final computation using all variables */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17;
    
    /* Force register clobbering for x86 (portable fallback) */
    #ifdef __i386__
    asm volatile("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi");
    #elif defined(__x86_64__)
    asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                 "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
    #endif
    
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
    
    /* Deeply nested conditionals */
    if (w0 > 0) {
        if (w1 > w0) {
            w2 = w2 + w1;
            asm volatile("" : : : "memory");
        } else {
            w3 = w3 * w0;
        }
        
        for (int k = 0; k < 3; k++) {
            w4 = w4 + k;
            w5 = w5 - k;
            w6 = w4 * w5;
            
            switch (k) {
                case 0: w7 = w6 + 1; break;
                case 1: w8 = w6 * 2; break;
                case 2: w9 = w6 / 3; break;
            }
        }
    }
    
    return w0 + w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8 + w9;
}

/* Main function that calls pressure functions */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Call high pressure function multiple times with different seeds */
    result += high_pressure_function(argc);
    result += high_pressure_function(argc + 1);
    result += high_pressure_function(argc + 2);
    
    /* Call alternate function */
    result += alternate_pressure_function(argc);
    result += alternate_pressure_function(argc + 10);
    
    /* Use result to prevent dead code elimination */
    asm volatile("" : "+r"(result));
    
    return result % 256;  /* Return non-zero to be safe */
}
