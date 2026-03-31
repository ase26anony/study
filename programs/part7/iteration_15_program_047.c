/* test_reload.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent constant propagation */
volatile int global_seed1 = 12345;
volatile int global_seed2 = 67890;
volatile int global_seed3 = 54321;

/* Non-inlineable function with massive register pressure */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3) {
    /* Declare many local variables of different types */
    int v1 = input1;
    int v2 = input2;
    int v3 = input3;
    short v4 = (short)(input1 + 1);
    short v5 = (short)(input2 - 1);
    char v6 = (char)(input3 & 0xFF);
    char v7 = (char)((input1 ^ input2) & 0xFF);
    long v8 = (long)input1 * input2;
    long v9 = (long)input2 * input3;
    int v10 = v1 + v2;
    int v11 = v2 - v3;
    int v12 = v3 * v1;
    int v13 = v1 | v2;
    int v14 = v2 & v3;
    int v15 = v3 ^ v1;
    int v16 = v10 << 2;
    int v17 = v11 >> 1;
    int v18 = v12 + v13;
    int v19 = v14 - v15;
    int v20 = v16 | v17;
    int v21 = v18 & v19;
    int v22 = v20 ^ v21;
    int v23 = v22 + v1;
    int v24 = v23 - v2;
    int v25 = v24 * v3;
    int v26 = v25 | v4;
    int v27 = v26 & v5;
    int v28 = v27 ^ v6;
    int v29 = v28 + v7;
    int v30 = v29 - (int)v8;
    
    /* Complex expressions with multiple uses of variables */
    v1 = (v2 & v3) | (v4 << (v5 & 3));
    v2 = v6 * v7 - (v8 >> 4);
    v3 = (v9 & 0xFFFF) + (v10 << 1) - (v11 >> 2);
    v4 = (short)((v12 & 0xFF) | ((v13 & 0xFF) << 8));
    v5 = (short)(v14 + v15 - v16);
    
    /* More operations keeping many values live */
    v6 = (char)((v17 ^ v18 ^ v19) & 0xFF);
    v7 = (char)((v20 | v21 | v22) & 0xFF);
    v8 = (long)v23 * v24 + v25;
    v9 = (long)v26 * v27 - v28;
    
    /* Inline assembly to force specific register usage */
    asm volatile("" : "+r"(v1), "+r"(v2) : : "cc", "memory");
    asm volatile("" : "+r"(v3), "+r"(v4) : : "cc", "memory");
    
    /* More arithmetic to maintain pressure */
    v10 = v1 + v2 + v3 + v4;
    v11 = v5 * v6 - v7;
    v12 = (v8 & 0xFFFFFFFF) ^ (v9 & 0xFFFFFFFF);
    v13 = v10 << (v11 & 3);
    v14 = v12 >> (v13 & 3);
    v15 = v14 + v15 + v16;
    v16 = v17 - v18 + v19;
    v17 = v20 * v21 / (v22 + 1);
    v18 = v23 | v24 | v25;
    v19 = v26 & v27 & v28;
    v20 = v29 ^ v30 ^ v1;
    
    /* Complex addressing mode simulation */
    int* ptr1 = &v1;
    int* ptr2 = &v2;
    int* ptr3 = &v3;
    
    /* Force memory accesses that compete for address registers */
    volatile int mem1 = *ptr1 + *ptr2;
    volatile int mem2 = *ptr2 - *ptr3;
    volatile int mem3 = *ptr3 * *ptr1;
    
    /* Use all variables in final computation */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ 
                 (int)(v8 & 0xFF) ^ (int)(v9 & 0xFF) ^
                 v10 ^ v11 ^ v12 ^ v13 ^ v14 ^ v15 ^
                 v16 ^ v17 ^ v18 ^ v19 ^ v20 ^
                 v21 ^ v22 ^ v23 ^ v24 ^ v25 ^
                 v26 ^ v27 ^ v28 ^ v29 ^ v30 ^
                 mem1 ^ mem2 ^ mem3;
    
    return result;
}

/* Another high-pressure function with different patterns */
__attribute__((noinline, noipa))
int secondary_pressure(int base) {
    /* Even more variables */
    int a1 = base + 1, a2 = base + 2, a3 = base + 3;
    int a4 = base + 4, a5 = base + 5, a6 = base + 6;
    int a7 = base + 7, a8 = base + 8, a9 = base + 9;
    int a10 = base + 10, a11 = base + 11, a12 = base + 12;
    int a13 = base + 13, a14 = base + 14, a15 = base + 15;
    int a16 = base + 16, a17 = base + 17, a18 = base + 18;
    int a19 = base + 19, a20 = base + 20;
    
    /* Chain computations to create dependencies */
    a1 = a1 * a2 + a3;
    a2 = a2 - a4 * a5;
    a3 = a3 | a6 & a7;
    a4 = a4 ^ a8 << a9;
    a5 = a5 + a10 - a11;
    a6 = a6 * a12 / (a13 + 1);
    a7 = a7 & a14 | a15;
    a8 = a8 ^ a16 ^ a17;
    a9 = a9 + a18 - a19;
    a10 = a10 * a20 + a1;
    
    /* More operations */
    a11 = (a1 << 3) | (a2 >> 2);
    a12 = (a3 & 0xFF00) | (a4 & 0xFF);
    a13 = a5 + a6 + a7 + a8;
    a14 = a9 * a10 - a11;
    a15 = a12 ^ a13 ^ a14;
    a16 = a15 + a16 + a17;
    a17 = a18 - a19 + a20;
    a18 = a1 * a2 - a3;
    a19 = a4 | a5 | a6;
    a20 = a7 & a8 & a9;
    
    /* Force register constraints with inline asm */
    asm volatile("" : "+r"(a1), "+r"(a2), "+r"(a3) : : "cc");
    asm volatile("" : "+r"(a4), "+r"(a5), "+r"(a6) : : "cc");
    
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
           a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18 + a19 + a20;
}

int main(int argc, char** argv) {
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_limit < 1) loop_limit = 1;
    if (loop_limit > 1000) loop_limit = 1000;
    
    int total_result = 0;
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* Mix calls to create different reload patterns */
        int r1 = create_reload_pressure(
            global_seed1 + i,
            global_seed2 - i,
            global_seed3 ^ i
        );
        
        int r2 = secondary_pressure(global_seed1 ^ i);
        
        total_result ^= r1;
        total_result += r2;
        
        /* Modify globals slightly */
        global_seed1 = (global_seed1 * 1103515245 + 12345) & 0x7FFFFFFF;
        global_seed2 = (global_seed2 * 1664525 + 1013904223) & 0x7FFFFFFF;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: 0x%08x\n", total_result & 0xFFFFFFFF);
    
    return (total_result & 0xFF);
}
