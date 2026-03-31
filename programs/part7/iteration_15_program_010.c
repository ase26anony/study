/* reload_coverage.c
 * Program designed to trigger GCC's reload pass initialization
 * Specifically targets lines 1381-1399 in reload.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent constant propagation */
volatile int global_seed1 = 12345;
volatile int global_seed2 = 67890;
volatile int global_seed3 = 13579;
volatile short global_short1 = 1000;
volatile short global_short2 = 2000;
volatile char global_char1 = 42;
volatile char global_char2 = 84;

/* Force no inlining and no inter-procedural analysis */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3) {
    /* Declare many local variables of different types to exhaust registers */
    int v1 = input1 + global_seed1;
    int v2 = input2 - global_seed2;
    int v3 = input3 * global_seed3;
    int v4 = v1 ^ v2;
    int v5 = v2 | v3;
    int v6 = v3 & v1;
    int v7 = v4 + v5;
    int v8 = v5 - v6;
    int v9 = v6 * v7;
    int v10 = v7 ^ v8;
    int v11 = v8 | v9;
    int v12 = v9 & v10;
    int v13 = v10 + v11;
    int v14 = v11 - v12;
    int v15 = v12 * v13;
    int v16 = v13 ^ v14;
    int v17 = v14 | v15;
    int v18 = v15 & v16;
    int v19 = v16 + v17;
    int v20 = v17 - v18;
    
    /* Mix in smaller types to increase register pressure */
    short s1 = global_short1 + v1;
    short s2 = global_short2 - v2;
    short s3 = s1 * s2;
    short s4 = s2 ^ s1;
    
    char c1 = global_char1 + v3;
    char c2 = global_char2 - v4;
    char c3 = c1 * c2;
    char c4 = c2 ^ c1;
    
    long l1 = (long)v1 * v2;
    long l2 = (long)v3 * v4;
    long l3 = l1 + l2;
    long l4 = l2 - l1;
    
    /* Complex expressions with multiple uses of variables */
    v1 = (v2 & v3) | (v4 << (v5 & 3));
    v6 = v7 * v8 - v9 + (v10 >> 2);
    v11 = (v12 | v13) ^ (v14 & v15);
    v16 = v17 + (v18 * v19) - (v20 << 1);
    
    /* Force specific register usage with inline asm */
    asm volatile("" : "+r"(v1), "+r"(v2) : : "cc", "memory");
    asm volatile("" : "+r"(v3), "+r"(v4) : : "cc", "memory");
    
    /* More complex operations with addressing modes */
    int* ptr1 = &v1;
    int* ptr2 = &v2;
    int* ptr3 = &v3;
    
    /* Create addressing mode pressure */
    v5 = *ptr1 + *ptr2;
    v6 = *ptr2 - *ptr3;
    v7 = *ptr3 * *ptr1;
    
    /* Use variables in memory addressing */
    int arr[4] = {v1, v2, v3, v4};
    v8 = arr[0] + arr[1];
    v9 = arr[1] - arr[2];
    v10 = arr[2] * arr[3];
    
    /* More arithmetic to keep values live */
    s1 = (s2 + s3) * (s4 - c1);
    s2 = (s3 ^ s4) | (c2 & c3);
    c1 = c2 + c3 - c4;
    c2 = c3 * c4 / (c1 + 1);
    
    l1 = l2 + l3 * l4;
    l2 = l3 - l4 / (l1 + 1);
    
    /* Even more variables to increase pressure */
    int v21 = v1 + v2 + v3;
    int v22 = v4 + v5 + v6;
    int v23 = v7 + v8 + v9;
    int v24 = v10 + v11 + v12;
    int v25 = v13 + v14 + v15;
    int v26 = v16 + v17 + v18;
    int v27 = v19 + v20 + v21;
    int v28 = v22 + v23 + v24;
    int v29 = v25 + v26 + v27;
    int v30 = v28 + v29 + v30; /* Self-reference to create complexity */
    
    /* Complex expression with many operands */
    int result = (v1 * v2) + (v3 << (v4 & 3)) - (v5 >> 1) +
                 (v6 & v7) | (v8 ^ v9) + (v10 * v11) -
                 (v12 << 2) + (v13 >> 1) & (v14 | v15) ^
                 (v16 + v17) - (v18 * v19) | (v20 & v21) +
                 (v22 ^ v23) - (v24 << 1) + (v25 >> 2) &
                 (v26 | v27) + (v28 * v29) - (v30 ^ result);
    
    /* Mix all types into final result */
    result += s1 + s2 + s3 + s4;
    result += c1 + c2 + c3 + c4;
    result += (int)(l1 & 0xFFFFFFFF) + (int)(l2 & 0xFFFFFFFF);
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(result) : "memory");
    
    return result;
}

int main(int argc, char** argv) {
    /* Use volatile to prevent loop optimizations */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 1;
    if (iterations > 1000) iterations = 1000;
    
    int total = 0;
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < iterations; i++) {
        /* Use volatile inputs to prevent constant propagation */
        volatile int input1 = argc + i + global_seed1;
        volatile int input2 = argc * 2 + i + global_seed2;
        volatile int input3 = argc * 3 + i + global_seed3;
        
        /* Call the high-pressure function */
        int result = create_reload_pressure(input1, input2, input3);
        
        /* Accumulate results with complex operation */
        total = (total ^ result) + (result & 0xFF);
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    /* Use the result to prevent elimination */
    printf("Result: %d\n", total & 0xFFFF);
    
    return (total & 0xFF);
}
