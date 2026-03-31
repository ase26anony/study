/* reload_coverage.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 42;
volatile int global_mask = 0xFFFF;

/* Non-inlineable function to create maximum register pressure */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3, int input4) {
    /* Declare many local variables to exhaust registers */
    int v1 = input1 + 1;
    int v2 = input2 * 2;
    int v3 = input3 | 0xAA;
    int v4 = input4 ^ 0x55;
    short v5 = (short)(v1 + v2);
    char v6 = (char)(v3 & 0xFF);
    long v7 = (long)v4 * 3L;
    int v8 = v1 ^ v2;
    int v9 = v3 + v4;
    int v10 = v5 * 2;
    int v11 = v6 | 0x80;
    long v12 = v7 >> 2;
    int v13 = v8 & v9;
    short v14 = (short)(v10 + v11);
    char v15 = (char)(v12 & 0xFF);
    int v16 = v13 * v14;
    int v17 = v15 + 100;
    long v18 = v12 + v16;
    int v19 = v17 ^ v18;
    int v20 = v19 << 3;
    int v21 = v20 | 0x0F;
    int v22 = v21 - v16;
    int v23 = v22 & 0xFF00;
    int v24 = v23 + v17;
    int v25 = v24 * 2;
    int v26 = v25 ^ v19;
    int v27 = v26 | 0x3333;
    int v28 = v27 - v22;
    int v29 = v28 << 1;
    int v30 = v29 & 0x7F;
    
    /* Complex expressions with multiple operands to force reloads */
    v1 = (v2 & v3) | (v4 << (v5 & 3));
    v6 = (v7 * v8) - (v9 >> (v10 % 4));
    v11 = ((v12 + v13) * (v14 | v15)) ^ (v16 & v17);
    v18 = (v19 + v20) - (v21 * v22) / (v23 | 1);
    v24 = ((v25 << 2) | (v26 >> 3)) + (v27 & v28) - (v29 ^ v30);
    
    /* More complex expressions with data dependencies */
    v2 = v1 * v3 + v4 / (v5 | 1);
    v3 = (v2 << v6) | (v7 >> v8);
    v4 = v9 * v10 - v11 * v12 + v13;
    v5 = (v14 & v15) | (v16 ^ v17) | (v18 & v19);
    v6 = v20 + v21 - v22 + v23 - v24;
    
    /* Use inline assembly to create register constraints */
    asm volatile("" : "+r"(v1), "+r"(v2) : : "cc", "memory");
    asm volatile("" : "+m"(v3), "+r"(v4) : : "cc");
    
    /* More operations to keep values live */
    v7 = v1 * v2 + v3 * v4;
    v8 = (v5 << 3) | (v6 >> 2);
    v9 = v7 ^ v8 ^ v1 ^ v2;
    v10 = v3 + v4 + v5 + v6;
    
    /* Complex addressing mode simulation */
    int* ptr1 = &v1;
    int* ptr2 = &v2;
    int* ptr3 = &v3;
    
    /* Force memory accesses that compete for address registers */
    v11 = *ptr1 + *ptr2;
    v12 = *ptr3 - v11;
    *ptr1 = v12;
    *ptr2 = v11 ^ v12;
    
    /* More arithmetic with many live values */
    v13 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    v14 = v11 * v12 - v13;
    v15 = (v14 & 0xFF) | (v13 << 8);
    v16 = v15 ^ v14 ^ v13;
    v17 = v16 * 3 + v15 * 2;
    v18 = v17 >> 4;
    v19 = v18 | v17;
    v20 = v19 & 0xABCD;
    
    /* Final aggregation to prevent dead code elimination */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10;
    result ^= v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18 ^ v19 ^ v20;
    result ^= v21 ^ v22 ^ v23 ^ v24 ^ v25 ^ v26 ^ v27 ^ v28 ^ v29 ^ v30;
    
    return result & global_mask;
}

int main(int argc, char** argv) {
    int total = 0;
    
    /* Use volatile loop counter to prevent optimization */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    if (iterations > 1000) iterations = 1000;
    
    /* Initialize volatile inputs */
    volatile int input1 = argc + global_seed;
    volatile int input2 = argc * 2;
    volatile int input3 = argc | 0x1234;
    volatile int input4 = argc ^ 0x5678;
    
    printf("Starting reload pressure test with %d iterations...\n", iterations);
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < iterations; i++) {
        /* Modify inputs slightly each iteration */
        int a = input1 + i;
        int b = input2 - i;
        int c = input3 ^ i;
        int d = input4 | i;
        
        /* Call the high-pressure function */
        int result = create_reload_pressure(a, b, c, d);
        
        /* Accumulate results with complex expression */
        total = (total * 31 + result) & 0xFFFFFF;
        
        /* Prevent loop unrolling with volatile side effect */
        asm volatile("" : : "r"(total) : "memory");
    }
    
    printf("Final result: %d\n", total);
    return total & 0xFF;
}
