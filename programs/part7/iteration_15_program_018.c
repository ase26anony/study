/* reload_coverage.c
 * Program to trigger GCC's reload pass initialization code
 * Specifically targets lines 1381-1399 in reload.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent constant propagation */
volatile int global_seed1 = 12345;
volatile int global_seed2 = 67890;
volatile short global_short1 = 1000;
volatile short global_short2 = 2000;
volatile char global_char1 = 42;
volatile char global_char2 = 84;

/* Non-inlineable function to create maximum register pressure */
__attribute__((noinline, noipa, optimize("no-goto")))
int create_reload_pressure(int input1, int input2, short s_input, char c_input) {
    /* Declare many local variables to exhaust registers */
    int v1 = input1 + global_seed1;
    int v2 = input2 - global_seed2;
    int v3 = v1 * v2;
    int v4 = v1 ^ v2;
    int v5 = v3 | v4;
    int v6 = v2 << 3;
    int v7 = v1 >> 2;
    int v8 = v5 + v6;
    int v9 = v7 - v8;
    int v10 = v9 * 13;
    int v11 = v10 & 0xFF;
    int v12 = v11 | 0x80;
    int v13 = v12 ^ v3;
    int v14 = v13 + v4;
    int v15 = v14 - v5;
    int v16 = v15 * v6;
    int v17 = v16 / (v7 + 1);
    int v18 = v17 ^ v8;
    int v19 = v18 | v9;
    int v20 = v19 & v10;
    
    /* Mix in different types */
    short s1 = s_input + global_short1;
    short s2 = s_input - global_short2;
    short s3 = s1 * s2;
    short s4 = s1 ^ s2;
    
    char c1 = c_input + global_char1;
    char c2 = c_input - global_char2;
    char c3 = c1 * c2;
    char c4 = c1 ^ c2;
    
    /* Long variables for additional pressure */
    long l1 = v1 + v2;
    long l2 = v3 * v4;
    long l3 = l1 ^ l2;
    long l4 = l1 | l2;
    long l5 = l3 + l4;
    
    /* More intermediate calculations creating dependencies */
    int v21 = (v11 + s3) * c3;
    int v22 = (v12 - s4) / (c4 + 1);
    int v23 = v21 ^ v22;
    int v24 = v23 << (c1 & 3);
    int v25 = v24 >> (c2 & 3);
    int v26 = v25 + l1;
    int v27 = v26 - l2;
    int v28 = v27 * l3;
    int v29 = v28 / (l4 + 1);
    int v30 = v29 ^ l5;
    
    /* Complex addressing mode simulation */
    int* ptr1 = &v1;
    int* ptr2 = &v2;
    int* ptr3 = &v3;
    
    /* Force memory accesses */
    volatile int mem1 = *ptr1 + *ptr2;
    volatile int mem2 = *ptr3 - *ptr1;
    
    /* Inline assembly to create register constraints */
    asm volatile("" : "+r"(v1), "+r"(v2) : : "cc", "memory");
    asm volatile("" : "+r"(v3), "+r"(v4) : : "cc", "memory");
    
    /* More operations to keep values live */
    v1 = v1 + mem1;
    v2 = v2 - mem2;
    v3 = v3 * v1;
    v4 = v4 / (v2 + 1);
    
    /* Complex expression requiring multiple operands simultaneously */
    int v31 = (v1 * v2) + (v3 << (v4 & 7)) - (v5 >> (v6 & 7)) 
              + (v7 & v8) | (v9 ^ v10) & (v11 | v12);
    
    /* Even more operations */
    int v32 = v31 + s1 + s2 + s3 + s4;
    int v33 = v32 * c1 * c2 * c3 * c4;
    int v34 = v33 ^ v13 ^ v14 ^ v15;
    int v35 = v34 | v16 | v17 | v18;
    int v36 = v35 & v19 & v20 & v21;
    int v37 = v36 + v22 - v23 + v24;
    int v38 = v37 * v25 / (v26 + 1);
    int v39 = v38 ^ v27 ^ v28 ^ v29;
    int v40 = v39 | v30 | v31 | v32;
    
    /* Final aggregation to prevent dead code elimination */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10
                 ^ v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18 ^ v19 ^ v20
                 ^ v21 ^ v22 ^ v23 ^ v24 ^ v25 ^ v26 ^ v27 ^ v28 ^ v29 ^ v30
                 ^ v31 ^ v32 ^ v33 ^ v34 ^ v35 ^ v36 ^ v37 ^ v38 ^ v39 ^ v40
                 ^ s1 ^ s2 ^ s3 ^ s4
                 ^ c1 ^ c2 ^ c3 ^ c4
                 ^ (int)(l1 & 0xFFFFFFFF) ^ (int)(l2 & 0xFFFFFFFF)
                 ^ (int)(l3 & 0xFFFFFFFF) ^ (int)(l4 & 0xFFFFFFFF)
                 ^ (int)(l5 & 0xFFFFFFFF);
    
    return result;
}

int main(int argc, char** argv) {
    /* Use argc to make execution variable */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 1;
    if (iterations > 10000) iterations = 10000;
    
    int total_result = 0;
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < iterations; i++) {
        /* Use different inputs each iteration to prevent optimization */
        int input1 = argc + i + global_seed1;
        int input2 = argc * i + global_seed2;
        short s_input = (short)(global_short1 + i);
        char c_input = (char)(global_char1 + i);
        
        /* Call the high-pressure function */
        int result = create_reload_pressure(input1, input2, s_input, c_input);
        
        /* Aggregate results */
        total_result ^= result;
        
        /* Prevent loop optimization */
        asm volatile("" : : "r"(i) : "memory");
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: 0x%08x\n", total_result & 0xFFFFFFFF);
    
    return (total_result & 0xFF);
}
