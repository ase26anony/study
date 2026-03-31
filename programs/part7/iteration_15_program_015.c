/* test_reload.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_seed = 42;
volatile int global_mask = 0xFFFFFFFF;

/* Prevent inlining and IPA optimizations */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3) {
    /* Declare many local variables to exhaust registers */
    int v1 = input1;
    int v2 = input2;
    int v3 = input3;
    int v4 = v1 + v2;
    int v5 = v2 - v3;
    int v6 = v3 * v1;
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
    
    /* Use different integer types to increase complexity */
    short s1 = v1 & 0xFFFF;
    short s2 = v2 & 0xFFFF;
    char c1 = v3 & 0xFF;
    char c2 = v4 & 0xFF;
    long l1 = (long)v5 * v6;
    long l2 = (long)v7 * v8;
    
    /* Complex expressions requiring multiple registers */
    v1 = (v2 & v3) | (v4 << (v5 & 3));
    v2 = (v6 * v7) - (v8 >> (v9 & 3));
    v3 = (v10 ^ v11) + (v12 & v13);
    v4 = (v14 | v15) * (v16 - v17);
    v5 = (v18 << 2) + (v19 >> 1);
    
    /* More operations mixing types */
    s1 = (s1 + s2) * c1;
    c2 = (c1 ^ c2) + (v1 & 0xFF);
    l1 = l1 + l2 + (long)v2;
    l2 = l2 - l1 * (long)v3;
    
    /* Inline assembly to force specific register usage */
    asm volatile("" : "+r"(v1), "+r"(v2) : : "cc", "memory");
    asm volatile("" : "+r"(v3), "+r"(v4) : : "cc", "memory");
    
    /* More variables to increase pressure */
    int v21 = v20 + v19;
    int v22 = v21 * v18;
    int v23 = v22 ^ v17;
    int v24 = v23 | v16;
    int v25 = v24 & v15;
    int v26 = v25 << 1;
    int v27 = v26 >> 2;
    int v28 = v27 + v14;
    int v29 = v28 - v13;
    int v30 = v29 * v12;
    
    /* Complex addressing mode simulation */
    int* ptr1 = &v1;
    int* ptr2 = &v2;
    int* ptr3 = &v3;
    
    /* Force memory accesses */
    volatile int mem1 = *ptr1;
    volatile int mem2 = *ptr2;
    volatile int mem3 = *ptr3;
    
    /* More operations with memory results */
    v21 = v21 + mem1;
    v22 = v22 - mem2;
    v23 = v23 * mem3;
    
    /* Even more variables */
    int v31 = v30 ^ v11;
    int v32 = v31 | v10;
    int v33 = v32 & v9;
    int v34 = v33 << 4;
    int v35 = v34 >> 2;
    int v36 = v35 + v8;
    int v37 = v36 - v7;
    int v38 = v37 * v6;
    int v39 = v38 ^ v5;
    int v40 = v39 | v4;
    
    /* Final complex expression using most variables */
    int result = (v1 + v2 - v3) * (v4 ^ v5) +
                 (v6 << (v7 & 3)) - (v8 >> (v9 & 3)) +
                 (v10 & v11) | (v12 | v13) +
                 (v14 * v15) - (v16 ^ v17) +
                 (v18 + v19) * (v20 - v21) +
                 (v22 & v23) | (v24 ^ v25) +
                 (v26 << 2) - (v27 >> 1) +
                 (v28 + v29) * (v30 ^ v31) +
                 (v32 & v33) | (v34 | v35) +
                 (v36 * v37) - (v38 ^ v39) +
                 (s1 * c1) + (c2 << 2) +
                 (int)((l1 + l2) & 0x7FFFFFFF);
    
    /* Use all variables in return to prevent elimination */
    result += v40;
    
    return result & global_mask;
}

int main(int argc, char *argv[]) {
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    volatile int seed = argc;
    int total = 0;
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* Use volatile inputs to prevent constant propagation */
        volatile int input1 = seed + i;
        volatile int input2 = global_seed * i;
        volatile int input3 = (seed << 3) ^ i;
        
        /* Call the high-pressure function */
        total += create_reload_pressure(input1, input2, input3);
        
        /* Modify globals to create side effects */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
