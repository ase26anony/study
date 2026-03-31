/* reload_test.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_seed1 = 12345;
volatile int global_seed2 = 67890;
volatile int global_seed3 = 54321;

/* Prevent inlining and inter-procedural optimization */
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
    
    /* Mix different integer types for more register pressure */
    short s1 = v1 & 0xFFFF;
    short s2 = v2 & 0xFFFF;
    short s3 = v3 & 0xFFFF;
    char c1 = v4 & 0xFF;
    char c2 = v5 & 0xFF;
    char c3 = v6 & 0xFF;
    long l1 = v7;
    long l2 = v8;
    long l3 = v9;
    
    /* Complex expressions with many live values */
    v1 = (v2 & v3) | (v4 << (v5 & 3));
    v6 = v7 * v8 - v9 + (v10 >> 1);
    v11 = (v12 ^ v13) + (v14 & v15) * v16;
    v17 = ((v18 | v19) << 2) - (v20 & 0x7F);
    
    /* Force register usage with inline assembly */
    asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3) : : "cc", "memory");
    asm volatile("" : "+r"(v4), "+r"(v5), "+r"(v6) : : "cc", "memory");
    
    /* More operations keeping values live */
    s1 = (s2 + s3) * c1;
    c2 = (c3 ^ s1) & 0x7F;
    l1 = l2 * l3 + v1;
    l2 = l1 - l3 * v2;
    
    /* Complex addressing mode simulation */
    int* ptr1 = &v1;
    int* ptr2 = &v2;
    int* ptr3 = &v3;
    
    /* Memory operations that compete for address registers */
    *ptr1 = *ptr2 + *ptr3;
    *ptr2 = *ptr1 - *ptr3;
    *ptr3 = *ptr1 ^ *ptr2;
    
    /* More variables to increase pressure */
    int v21 = v1 + v2 + v3;
    int v22 = v4 * v5 / (v6 + 1);
    int v23 = (v7 << 4) | (v8 >> 4);
    int v24 = v9 ^ v10 ^ v11;
    int v25 = v12 + v13 - v14;
    int v26 = v15 & v16 & v17;
    int v27 = v18 | v19 | v20;
    int v28 = v21 * v22;
    int v29 = v23 + v24;
    int v30 = v25 - v26;
    
    /* Use all variables in final computation */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10;
    result ^= v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18 ^ v19 ^ v20;
    result ^= v21 ^ v22 ^ v23 ^ v24 ^ v25 ^ v26 ^ v27 ^ v28 ^ v29 ^ v30;
    result += s1 + s2 + s3 + c1 + c2 + c3;
    result += (int)(l1 & 0xFFFFFFFF) + (int)(l2 & 0xFFFFFFFF) + (int)(l3 & 0xFFFFFFFF);
    
    return result;
}

int main(int argc, char** argv) {
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    volatile int seed1 = argc + global_seed1;
    volatile int seed2 = argc * 2 + global_seed2;
    volatile int seed3 = argc * 3 + global_seed3;
    
    int total = 0;
    
    /* Loop to prevent unrolling and maintain pressure */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* Call the high-pressure function */
        total += create_reload_pressure(seed1 + i, seed2 + i * 2, seed3 + i * 3);
        
        /* Modify seeds to prevent constant propagation */
        seed1 ^= i;
        seed2 += i;
        seed3 -= i;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
