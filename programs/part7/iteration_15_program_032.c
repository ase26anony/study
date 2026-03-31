/* test_reload.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_seed = 42;
volatile int global_mask = 0xFFFF;

/* Prevent inlining and IPA optimizations */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3, 
                           int input4, int input5, int input6) {
    /* Declare many local variables to exhaust registers */
    int v1 = input1 + global_seed;
    int v2 = input2 ^ global_mask;
    int v3 = input3 * 3;
    int v4 = input4 | 0xAA;
    int v5 = input5 & 0x55;
    int v6 = input6 << 2;
    
    /* More variables with different types */
    short s1 = v1 & 0xFF;
    short s2 = v2 & 0xFF;
    char c1 = v3 & 0x3F;
    char c2 = v4 & 0x3F;
    long l1 = v5 * 17L;
    long l2 = v6 * 23L;
    
    /* Even more integer variables */
    int v7 = v1 + v2;
    int v8 = v3 - v4;
    int v9 = v5 ^ v6;
    int v10 = v7 * v8;
    int v11 = v9 + v10;
    int v12 = s1 * s2;
    int v13 = c1 | c2;
    int v14 = l1 & 0xFFFFFFFF;
    int v15 = l2 & 0xFFFFFFFF;
    
    /* Additional variables to increase pressure */
    int v16 = v11 << 3;
    int v17 = v12 >> 2;
    int v18 = v13 * 7;
    int v19 = v14 ^ v15;
    int v20 = v16 | v17;
    int v21 = v18 & v19;
    int v22 = v20 + v21;
    int v23 = v22 * 11;
    int v24 = v23 - 19;
    int v25 = v24 ^ 0xDEADBEEF;
    
    /* Complex expressions requiring multiple registers */
    int complex1 = (v1 * v2) + (v3 << v4) - (v5 & v6);
    int complex2 = (v7 | v8) ^ (v9 * v10) + (v11 >> 2);
    int complex3 = (v12 & v13) | (v14 << 1) - (v15 * 3);
    
    /* Force specific register usage with inline asm */
    asm volatile("" : "+r"(v1), "+r"(v2) : : "cc", "memory");
    
    /* More operations keeping many values live */
    v16 = complex1 * complex2;
    v17 = complex3 ^ complex1;
    v18 = v16 + v17;
    v19 = v18 * 2;
    v20 = v19 - complex2;
    
    /* Use volatile memory access to force reloads */
    volatile int mem_var = v20;
    v21 = mem_var + v25;
    
    /* More arithmetic with all variables */
    v22 = (v1 & v2) | (v3 << v4);
    v23 = (v5 * v6) + (v7 - v8);
    v24 = (v9 ^ v10) & (v11 | v12);
    v25 = (v13 + v14) - (v15 * v16);
    
    /* Another inline asm to create register constraints */
    asm volatile("" 
                 : "=r"(v22), "=r"(v23) 
                 : "0"(v22), "1"(v23), "r"(v24), "r"(v25)
                 : "cc");
    
    /* Final computation using all variables */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10;
    result ^= v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18;
    result ^= v19 ^ v20 ^ v21 ^ v22 ^ v23 ^ v24 ^ v25;
    result ^= complex1 ^ complex2 ^ complex3;
    result ^= s1 ^ s2 ^ c1 ^ c2;
    result ^= (l1 & 0xFF) ^ (l2 & 0xFF);
    
    return result & 0xFF;  /* Return small value */
}

int main(int argc, char **argv) {
    int total = 0;
    
    /* Use volatile loop counter to prevent optimization */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    if (iterations > 1000) iterations = 1000;
    
    /* Initialize volatile inputs */
    volatile int in1 = argc;
    volatile int in2 = argc * 2;
    volatile int in3 = argc + 1;
    volatile int in4 = argc | 0x55;
    volatile int in5 = argc & 0xAA;
    volatile int in6 = argc << 3;
    
    printf("Starting reload pressure test with %d iterations...\n", iterations);
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < iterations; i++) {
        /* Modify inputs slightly each iteration */
        int arg1 = in1 + i;
        int arg2 = in2 ^ i;
        int arg3 = in3 * (i + 1);
        int arg4 = in4 | i;
        int arg5 = in5 & i;
        int arg6 = in6 + (i << 2);
        
        /* Call the high-pressure function */
        int result = create_reload_pressure(arg1, arg2, arg3, arg4, arg5, arg6);
        
        /* Accumulate result to prevent elimination */
        total += result;
        
        /* Modify globals to create side effects */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7FFFFFFF;
        global_mask = global_mask ^ (i << 8);
    }
    
    printf("Result: %d\n", total & 0xFF);
    return total & 0xFF;
}
