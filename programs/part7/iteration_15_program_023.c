/* test_reload.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_seed = 42;
volatile int global_mask = 0xFFFFFFFF;

/* Force register pressure function to not be optimized away */
__attribute__((noinline, noipa, optimize("O0")))
int create_reload_pressure(int input1, int input2, int input3) {
    /* Declare many local variables to exhaust registers */
    int v1 = input1 + 1;
    int v2 = input2 * 2;
    int v3 = input3 ^ 0x55;
    int v4 = v1 + v2;
    int v5 = v2 - v3;
    int v6 = v3 * v4;
    int v7 = v4 ^ v5;
    int v8 = v5 | v6;
    int v9 = v6 & v7;
    int v10 = v7 << 2;
    int v11 = v8 >> 1;
    int v12 = v9 + v10;
    int v13 = v10 - v11;
    int v14 = v11 * v12;
    int v15 = v12 ^ v13;
    int v16 = v13 | v14;
    int v17 = v14 & v15;
    int v18 = v15 << 3;
    int v19 = v16 >> 2;
    int v20 = v17 + v18;
    int v21 = v18 - v19;
    int v22 = v19 * v20;
    int v23 = v20 ^ v21;
    int v24 = v21 | v22;
    int v25 = v22 & v23;
    int v26 = v23 << 1;
    int v27 = v24 >> 3;
    int v28 = v25 + v26;
    int v29 = v26 - v27;
    int v30 = v27 * v28;
    
    /* Mix different integer types to create more register pressure */
    short s1 = v1 & 0xFFFF;
    short s2 = v2 & 0xFFFF;
    short s3 = v3 & 0xFFFF;
    char c1 = v4 & 0xFF;
    char c2 = v5 & 0xFF;
    char c3 = v6 & 0xFF;
    long l1 = (long)v7 * v8;
    long l2 = (long)v9 * v10;
    
    /* Complex expressions with multiple uses of variables */
    v1 = (v2 & v3) | (v4 << (v5 & 3)) - v6;
    v2 = (v3 * v4) + (v5 << (v6 & 7)) - v7;
    v3 = (v4 ^ v5) | (v6 & v7) + (v8 >> 1);
    v4 = (v5 - v6) * (v7 + v8) ^ (v9 & 0xFF);
    v5 = (v6 | v7) & (v8 ^ v9) + (v10 << 2);
    
    /* More complex expressions forcing spill/reload */
    v6 = ((v7 * v8) + (v9 << v10)) - ((v11 & v12) | (v13 ^ v14));
    v7 = ((v8 & v9) | (v10 << v11)) + ((v12 - v13) * (v14 ^ v15));
    v8 = ((v9 | v10) & (v11 ^ v12)) - ((v13 << 3) + (v14 >> 2));
    v9 = ((v10 * v11) - (v12 & v13)) | ((v14 + v15) ^ (v16 & 0xFF));
    v10 = ((v11 ^ v12) + (v13 << 1)) & ((v14 | v15) - (v16 >> 3));
    
    /* Use inline assembly to create specific register constraints */
    asm volatile ("" : "+r" (v1), "+r" (v2), "+r" (v3) : : "cc", "memory");
    asm volatile ("" : "+r" (v4), "+r" (v5), "+r" (v6) : : "cc", "memory");
    
    /* Operations mixing different types */
    s1 = (s1 + s2) * s3;
    s2 = (s2 - s3) & s1;
    s3 = (s3 | s1) ^ s2;
    
    c1 = (c1 + c2) * c3;
    c2 = (c2 - c3) & c1;
    c3 = (c3 | c1) ^ c2;
    
    l1 = l1 + l2 * (v1 + v2);
    l2 = l2 - l1 * (v3 - v4);
    
    /* More complex expressions with all variables live */
    v11 = (v12 & v13) | (v14 << (v15 & 3)) + v16;
    v12 = (v13 * v14) - (v15 >> (v16 & 7)) ^ v17;
    v13 = (v14 ^ v15) & (v16 | v17) + (v18 << 1);
    v14 = (v15 + v16) * (v17 - v18) | (v19 & 0xFF);
    v15 = (v16 & v17) | (v18 ^ v19) - (v20 >> 2);
    
    /* Force memory accesses with volatile */
    volatile int mem1 = v21;
    volatile int mem2 = v22;
    volatile int mem3 = v23;
    
    v16 = v17 + mem1 * mem2 - mem3;
    v17 = v18 - mem2 * mem3 + mem1;
    v18 = v19 ^ mem3 * mem1 - mem2;
    
    /* Even more complex expressions */
    v19 = ((v20 * v21) + (v22 << v23)) & ((v24 | v25) ^ (v26 & v27));
    v20 = ((v21 & v22) | (v23 << v24)) - ((v25 ^ v26) + (v27 >> 1));
    v21 = ((v22 | v23) & (v24 ^ v25)) + ((v26 << 2) - (v27 & 3));
    v22 = ((v23 * v24) - (v25 & v26)) ^ ((v27 + v28) | (v29 & 0xFF));
    v23 = ((v24 ^ v25) + (v26 << 1)) | ((v27 & v28) - (v29 >> 3));
    
    /* Final complex computation using all variables */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10;
    result ^= v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18 ^ v19 ^ v20;
    result ^= v21 ^ v22 ^ v23 ^ v24 ^ v25 ^ v26 ^ v27 ^ v28 ^ v29 ^ v30;
    result ^= s1 ^ s2 ^ s3 ^ c1 ^ c2 ^ c3 ^ (int)l1 ^ (int)l2;
    
    return result & 0xFF; /* Return small value to prevent overflow */
}

int main(int argc, char *argv[]) {
    /* Use command line arguments to create variable inputs */
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_limit < 1) loop_limit = 1;
    if (loop_limit > 1000) loop_limit = 1000;
    
    volatile int seed1 = argc + global_seed;
    volatile int seed2 = argc * 2 + global_mask;
    volatile int seed3 = argc ^ 0xAA;
    
    int total = 0;
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* Modify seeds slightly each iteration */
        seed1 = (seed1 * 1103515245 + 12345) & 0x7FFFFFFF;
        seed2 = (seed2 * 1664525 + 1013904223) & 0x7FFFFFFF;
        seed3 = (seed3 * 214013 + 2531011) & 0x7FFFFFFF;
        
        /* Call the high register pressure function */
        total += create_reload_pressure(seed1, seed2, seed3);
        
        /* Prevent loop unrolling */
        asm volatile ("" : : : "memory");
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return (total & 0xFF) == 0 ? 0 : 1;
}
