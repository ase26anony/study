/* reload_coverage.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent constant propagation */
volatile int global_seed1 = 12345;
volatile int global_seed2 = 67890;
volatile int global_seed3 = 24680;
volatile short global_short1 = 1000;
volatile short global_short2 = 2000;
volatile char global_char1 = 'A';
volatile char global_char2 = 'B';

/* Prevent inlining and IPA optimizations */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3) {
    /* Declare many local variables of different types to exhaust registers */
    int v1 = input1 + global_seed1;
    int v2 = input2 * global_seed2;
    int v3 = v1 ^ v2;
    int v4 = v2 - v1;
    int v5 = v3 | v4;
    int v6 = v4 & v5;
    int v7 = v5 ^ v6;
    int v8 = v6 + v7;
    int v9 = v7 - v8;
    int v10 = v8 | v9;
    
    /* More variables with different types */
    short s1 = (short)(v1 + global_short1);
    short s2 = (short)(v2 - global_short2);
    short s3 = s1 * s2;
    short s4 = s2 / (s1 ? s1 : 1);
    
    char c1 = (char)(v3 + global_char1);
    char c2 = (char)(v4 + global_char2);
    char c3 = c1 & c2;
    char c4 = c2 | c1;
    
    long l1 = (long)v5 * (long)v6;
    long l2 = (long)v7 + (long)v8;
    long l3 = l1 ^ l2;
    long l4 = l2 - l1;
    
    /* Even more variables to increase pressure */
    int v11 = v9 << 2;
    int v12 = v10 >> 1;
    int v13 = v11 * v12;
    int v14 = v12 + v11;
    int v15 = v13 ^ v14;
    int v16 = v14 & v15;
    int v17 = v15 | v16;
    int v18 = v16 - v17;
    int v19 = v17 + v18;
    int v20 = v18 ^ v19;
    
    /* Complex expressions with many operands to force reloads */
    v1 = (v2 & v3) | (v4 << (v5 & 3)) - (v6 * v7) + (v8 ^ v9);
    v2 = ((v10 * v11) >> (v12 & 7)) + ((v13 & v14) | (v15 ^ v16));
    v3 = (v17 - v18) * (v19 + v20) - ((s1 * s2) | (s3 & s4));
    
    /* Use inline assembly to create register constraints */
    asm volatile("" : "+r"(v1), "+r"(v2) : : "cc", "memory");
    asm volatile("" : "+m"(v3), "+r"(v4) : : "cc");
    
    /* More complex operations mixing all variables */
    l1 = (l2 * l3) + (l4 ^ (long)v1);
    l2 = (l3 - l4) & ((long)v2 | (long)v3);
    
    s1 = (short)((s2 + s3) * (s4 - c1));
    s2 = (short)((c2 & c3) | (c4 ^ s1));
    
    /* Create addressing mode pressure with array accesses */
    int arr[8];
    for (int i = 0; i < 8; i++) {
        arr[i] = v1 + i * v2;
    }
    
    /* Complex expression using array elements */
    v5 = arr[0] * arr[1] + arr[2] - arr[3] | arr[4] & arr[5] ^ arr[6] + arr[7];
    
    /* More operations keeping many values live */
    v6 = (v7 << (v8 & 3)) + (v9 >> (v10 & 7)) - (v11 * v12);
    v7 = (v13 & v14) | (v15 ^ v16) + (v17 - v18);
    v8 = v19 * v20 + (s1 * s2) - (c3 * c4);
    
    /* Additional variables to ensure maximum pressure */
    int v21 = l1 + l2;
    int v22 = l3 - l4;
    int v23 = v21 ^ v22;
    int v24 = v22 & v21;
    int v25 = v23 | v24;
    int v26 = v24 - v25;
    int v27 = v25 + v26;
    int v28 = v26 ^ v27;
    int v29 = v27 & v28;
    int v30 = v28 | v29;
    
    /* Final complex expression using all variables */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10 ^
                 v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18 ^ v19 ^ v20 ^
                 v21 ^ v22 ^ v23 ^ v24 ^ v25 ^ v26 ^ v27 ^ v28 ^ v29 ^ v30 ^
                 s1 ^ s2 ^ s3 ^ s4 ^ c1 ^ c2 ^ c3 ^ c4 ^
                 (int)(l1 ^ l2 ^ l3 ^ l4);
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(result) : "memory");
    
    return result;
}

int main(int argc, char **argv) {
    /* Use volatile to prevent loop optimizations */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 1;
    
    int total_result = 0;
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < iterations; i++) {
        /* Use different inputs each iteration to prevent constant folding */
        int input1 = argc + i + global_seed1;
        int input2 = argc * i + global_seed2;
        int input3 = argc ^ i + global_seed3;
        
        /* Call the high-pressure function */
        int result = create_reload_pressure(input1, input2, input3);
        
        /* Accumulate result to create side effect */
        total_result ^= result;
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: 0x%08x\n", total_result & 0xFFFFFFFF);
    
    return total_result & 0xFF;
}
