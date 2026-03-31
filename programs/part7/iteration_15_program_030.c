/* test_reload.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int global_seed = 42;
volatile int global_mask = 0xFFFFFFFF;

/* Function with massive register pressure - marked to prevent inlining */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3, int input4) {
    /* Declare many local variables to exhaust registers */
    int v1 = input1 + 1;
    int v2 = input2 * 2;
    int v3 = input3 | 0xFF;
    int v4 = input4 ^ 0xAA;
    short v5 = (short)(input1 * 3);
    short v6 = (short)(input2 + 5);
    char v7 = (char)(input3 & 0x7F);
    char v8 = (char)(input4 | 0x55);
    long v9 = (long)input1 * input2;
    long v10 = (long)input3 * input4;
    int v11 = v1 + v2;
    int v12 = v3 - v4;
    short v13 = v5 + v6;
    char v14 = v7 ^ v8;
    long v15 = v9 >> 2;
    long v16 = v10 << 1;
    int v17 = v11 * v12;
    int v18 = v13 | v14;
    long v19 = v15 + v16;
    int v20 = v17 & 0xFFFF;
    int v21 = v18 ^ 0x1234;
    long v22 = v19 * 3;
    int v23 = v20 + v21;
    long v24 = v22 - v19;
    int v25 = v23 << 3;
    int v26 = v25 >> 1;
    int v27 = v26 | 0xAA55;
    int v28 = v27 ^ v23;
    long v29 = v24 + v22;
    int v30 = v28 & v27;
    
    /* Complex expressions requiring multiple registers */
    v1 = (v2 & v3) | (v4 << (v5 & 7));
    v6 = (v7 * v8) - (v9 & 0xFF);
    v10 = (v11 ^ v12) + (v13 | v14);
    v15 = (v16 >> 4) * (v17 & 0xF);
    v18 = (v19 % 256) + (v20 << 2);
    v21 = (v22 & 0xFFFF) | (v23 << 16);
    v24 = (v25 * v26) / (v27 + 1);
    v28 = (v29 ^ v30) & (v1 | v2);
    
    /* More operations to keep values live */
    v3 = v4 + v5 * v6 - v7;
    v8 = (v9 & v10) | (v11 ^ v12);
    v13 = v14 << (v15 & 3);
    v16 = v17 + v18 - v19 * v20;
    v21 = v22 | v23 & v24 ^ v25;
    v26 = v27 * v28 + v29 - v30;
    
    /* Use inline assembly to create register constraints */
    asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3) : : "cc", "memory");
    asm volatile("" : "+m"(v4), "+r"(v5) : : "cc");
    
    /* Complex addressing mode simulation */
    int* ptr1 = &v6;
    int* ptr2 = &v7;
    int* ptr3 = &v8;
    
    *ptr1 = (*ptr2 + *ptr3) * v9;
    *ptr2 = (*ptr1 - *ptr3) & v10;
    *ptr3 = (*ptr1 ^ *ptr2) | v11;
    
    /* More arithmetic with mixed types */
    v12 = (int)v13 + (int)v14 * (int)v15;
    v16 = (long)v17 * (long)v18 - (long)v19;
    v20 = (v21 & 0xFF) << (v22 & 0x7);
    v23 = (v24 | 0xFFFF0000) ^ (v25 & 0x0000FFFF);
    v26 = v27 * 3 + v28 / 2 - v29 % 7;
    v30 = (v1 ^ v2 ^ v3 ^ v4 ^ v5) & 0x7FFFFFFF;
    
    /* Final aggregation to prevent dead code elimination */
    int result = v1 + v2 - v3 * v4 + v5 / (v6 + 1) 
                 + (v7 & v8) | (v9 ^ v10) 
                 + (v11 << 2) - (v12 >> 1)
                 + v13 * v14 - v15 + v16
                 + (v17 & v18) | (v19 ^ v20)
                 + v21 * v22 - v23 + v24
                 + (v25 & v26) | (v27 ^ v28)
                 + v29 * v30;
    
    return result & 0x7FFF;  /* Keep result bounded */
}

/* Another high-pressure function with different patterns */
__attribute__((noinline, noipa))
int secondary_pressure(int base) {
    /* Even more variables */
    int a1 = base * 2, a2 = base + 3, a3 = base ^ 0xCC;
    int a4 = base | 0x55, a5 = base & 0xAA;
    int a6 = a1 + a2, a7 = a3 - a4, a8 = a5 * a6;
    int a9 = a7 | a8, a10 = a9 ^ a1, a11 = a2 & a3;
    int a12 = a4 + a5, a13 = a6 - a7, a14 = a8 * a9;
    int a15 = a10 | a11, a16 = a12 ^ a13, a17 = a14 & a15;
    int a18 = a16 + a17, a19 = a18 - a1, a20 = a19 * a2;
    
    /* Complex expression chain */
    a1 = (a2 << (a3 & 3)) + (a4 >> (a5 & 3));
    a6 = (a7 & a8) | (a9 ^ a10);
    a11 = a12 * a13 - a14 / (a15 + 1);
    a16 = (a17 | a18) & (a19 ^ a20);
    
    /* Force memory operations */
    volatile int mem1 = a1;
    volatile int mem2 = a2;
    
    a3 = mem1 + mem2;
    a4 = mem1 - mem2;
    
    /* More operations */
    for (int i = 0; i < 3; i++) {
        a5 = a6 + a7 - a8 * i;
        a9 = a10 ^ a11 | a12 & i;
        a13 = a14 << (a15 & 3) + i;
    }
    
    return a1 + a2 - a3 + a4 - a5 + a6 - a7 + a8;
}

int main(int argc, char **argv) {
    /* Use command line arguments to create variable inputs */
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_limit < 10) loop_limit = 10;
    if (loop_limit > 1000) loop_limit = 1000;
    
    int total_result = 0;
    
    /* Loop to ensure the function is called multiple times */
    for (volatile int iteration = 0; iteration < loop_limit; iteration++) {
        /* Create varying inputs to prevent constant propagation */
        int input1 = global_seed + iteration;
        int input2 = global_mask - iteration;
        int input3 = iteration * 7;
        int input4 = (iteration << 3) ^ 0x5A5A;
        
        /* Call the high-pressure function */
        int result1 = create_reload_pressure(input1, input2, input3, input4);
        
        /* Call secondary function */
        int result2 = secondary_pressure(result1 + iteration);
        
        /* Combine results */
        total_result ^= result1;
        total_result += result2;
        total_result &= 0xFFFFFF;
        
        /* Modify global to prevent optimization */
        global_seed ^= result1;
        global_mask &= result2;
    }
    
    /* Print result to create observable side effect */
    printf("Final result: %d (0x%08X)\n", total_result, total_result);
    
    return total_result & 0xFF;
}
