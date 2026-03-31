/* reload_coverage.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_seed1 = 12345;
volatile int global_seed2 = 67890;
volatile int global_seed3 = 13579;

/* Non-inlineable function to create maximum register pressure */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3) {
    /* Declare many local variables with different types to exhaust registers */
    int v1 = input1 + 1;
    int v2 = input2 - 1;
    int v3 = input3 * 2;
    short v4 = (short)(input1 & 0xFFFF);
    short v5 = (short)(input2 >> 8);
    char v6 = (char)(input3 & 0xFF);
    long v7 = (long)input1 * input2;
    long v8 = (long)input2 * input3;
    int v9 = v1 ^ v2;
    int v10 = v3 | v9;
    short v11 = (short)(v4 + v5);
    char v12 = (char)(v6 ^ 0x55);
    long v13 = v7 - v8;
    int v14 = v10 << 3;
    int v15 = v9 >> 2;
    short v16 = (short)(v11 * 2);
    char v17 = (char)(v12 + 1);
    long v18 = v13 / 2;
    int v19 = v14 & 0x0F0F;
    int v20 = v15 | 0x00FF;
    int v21 = v1 + v2 + v3;
    int v22 = v9 * v10;
    int v23 = v14 ^ v15;
    int v24 = v19 | v20;
    int v25 = v21 & v22;
    int v26 = v23 ^ v24;
    int v27 = v25 | v26;
    int v28 = v27 + input1;
    int v29 = v28 - input2;
    int v30 = v29 * input3;
    
    /* Complex expressions requiring multiple registers simultaneously */
    v1 = (v2 & v3) | (v4 << (v5 & 3));
    v6 = (v7 % 256) + (v8 % 128) - (v9 & 0x7F);
    v10 = (v11 * v12) + (v13 >> 4) - (v14 & 0xFF);
    v15 = ((v16 << 2) | (v17 & 0xF)) ^ (v18 & 0xFFFF);
    v19 = (v20 + v21) * (v22 - v23) / (v24 | 1);
    v25 = (v26 ^ v27) & (v28 | v29) + (v30 << 1);
    
    /* More intermediate calculations keeping values live */
    int t1 = v1 * v2 + v3;
    int t2 = v4 * v5 - v6;
    int t3 = v7 / 8 + v8 % 16;
    int t4 = v9 & v10 | v11;
    int t5 = v12 << 4 ^ v13;
    int t6 = v14 + v15 * v16;
    int t7 = v17 | v18 & v19;
    int t8 = v20 ^ v21 + v22;
    int t9 = v23 * v24 - v25;
    int t10 = v26 | v27 & v28;
    
    /* Use inline assembly to create register constraints */
    asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3) : : "cc", "memory");
    asm volatile("" : "+r"(v4), "+r"(v5) : : "cc");
    asm volatile("" : "+m"(v6), "+m"(v7) : : "cc");
    
    /* Complex addressing mode simulation */
    int* ptr1 = &v1;
    int* ptr2 = &v2;
    int* ptr3 = &v3;
    
    /* Force memory accesses that compete for address registers */
    v1 = *ptr1 + *ptr2;
    v2 = *ptr3 + v1;
    v3 = v1 - *ptr1;
    
    /* More arithmetic to keep values live */
    v4 = (v1 & 0xFF) | (v2 << 8);
    v5 = (v3 ^ 0xAA) + (v4 & 0x55);
    v6 = (v5 * 3) / 2;
    v7 = v6 + v1 - v2;
    v8 = v7 | v3 ^ v4;
    v9 = v8 & v5 + v6;
    v10 = v9 << 2 | v7 >> 1;
    
    /* Final aggregation to prevent dead code elimination */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6;
    result += v7 ^ v8 ^ v9 ^ v10;
    result += t1 ^ t2 ^ t3 ^ t4 ^ t5;
    result += t6 ^ t7 ^ t8 ^ t9 ^ t10;
    result += v11 ^ v12 ^ v13 ^ v14 ^ v15;
    result += v16 ^ v17 ^ v18 ^ v19 ^ v20;
    result += v21 ^ v22 ^ v23 ^ v24 ^ v25;
    result += v26 ^ v27 ^ v28 ^ v29 ^ v30;
    
    return result & 0x7FFFFFFF; /* Keep positive */
}

int main(int argc, char** argv) {
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_limit < 1) loop_limit = 1;
    if (loop_limit > 1000) loop_limit = 1000;
    
    int total_result = 0;
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* Use volatile globals and argc to prevent constant propagation */
        int input1 = global_seed1 + argc + i;
        int input2 = global_seed2 - argc * i;
        int input3 = global_seed3 ^ (argc + i * 3);
        
        /* Call the high-pressure function */
        int result = create_reload_pressure(input1, input2, input3);
        
        /* Mix results to create data dependencies between iterations */
        total_result ^= result;
        total_result += i;
        
        /* Prevent loop unrolling with volatile side effect */
        asm volatile("" : : "r"(total_result) : "memory");
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Final result: %d\n", total_result & 0xFF);
    
    return (total_result & 0xFF) == 0 ? 0 : 1;
}
