/* reload_coverage.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_seed1 = 12345;
volatile int global_seed2 = 67890;
volatile int global_seed3 = 54321;

/* Non-inlineable function with massive register pressure */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3) {
    /* Declare many local variables of different types to exhaust registers */
    int v1 = input1 + 1;
    int v2 = input2 - 1;
    int v3 = v1 * v2;
    short v4 = (short)(input3 & 0xFFFF);
    char v5 = (char)(input1 & 0xFF);
    long v6 = (long)input1 * input2;
    int v7 = v3 ^ v1;
    int v8 = v2 << 3;
    short v9 = (short)(v4 + 5);
    char v10 = v5 ^ 0x55;
    long v11 = v6 + 1000;
    int v12 = v7 | v8;
    int v13 = v12 - v3;
    short v14 = v9 * 2;
    char v15 = v10 + 1;
    long v16 = v11 >> 2;
    int v17 = v13 & 0x0F0F0F0F;
    int v18 = v17 * 3;
    short v19 = v14 | 0x1234;
    char v20 = ~v15;
    long v21 = v16 * 7;
    int v22 = v18 + v17;
    int v23 = v22 ^ v18;
    short v24 = v19 & 0x00FF;
    char v25 = v20 << 2;
    long v26 = v21 - 5000;
    int v27 = v23 >> 4;
    int v28 = v27 * v22;
    short v29 = v24 + v19;
    char v30 = v25 | 0x0F;
    
    /* Complex expressions requiring multiple registers simultaneously */
    v1 = (v2 & v3) | (v4 << (v5 & 7));
    v6 = (v7 * v8) - (v9 * v10) + (v11 >> 3);
    v12 = ((v13 + v14) * (v15 - v16)) & 0xFFFFFFFF;
    v17 = (v18 ^ v19) | (v20 << (v21 & 15));
    v22 = v23 * v24 - v25 * v26 + v27;
    v28 = ((v29 & v30) << 4) | ((v1 & v2) >> 2);
    
    /* More interleaved operations to keep values live */
    v3 = v4 * v5 + v6 / 2;
    v7 = v8 | v9 ^ v10;
    v11 = v12 + v13 - v14;
    v15 = v16 & v17 | v18;
    v19 = v20 * v21 + v22;
    v23 = v24 ^ v25 & v26;
    v27 = v28 + v29 - v30;
    
    /* Inline assembly to create register constraints */
    asm volatile("" : "+r"(v1), "+r"(v2) : : "cc", "memory");
    asm volatile("" : "+m"(v3), "+r"(v4) : : "cc");
    
    /* Complex addressing mode simulation */
    int* ptr1 = &v5;
    int* ptr2 = &v6;
    int* ptr3 = &v7;
    
    /* Force memory accesses that compete for address registers */
    volatile int mem1 = *ptr1 + *ptr2;
    volatile int mem2 = *ptr3 - *ptr1;
    
    /* More operations with volatile memory */
    v8 = mem1 * 2;
    v9 = mem2 + v8;
    
    /* Additional inline assembly with specific constraints */
    asm volatile("addl %1, %0" : "+r"(v10) : "r"(v11) : "cc");
    asm volatile("movl %1, %0" : "=r"(v12) : "m"(v13) :);
    
    /* Even more variables to increase pressure */
    int v31 = v1 + v2 + v3;
    int v32 = v4 * v5 * v6;
    int v33 = v7 ^ v8 ^ v9;
    int v34 = v10 | v11 | v12;
    int v35 = v13 & v14 & v15;
    int v36 = v16 + v17 + v18;
    int v37 = v19 - v20 - v21;
    int v38 = v22 * v23 * v24;
    int v39 = v25 ^ v26 ^ v27;
    int v40 = v28 | v29 | v30;
    
    /* Complex expression using all variables */
    int result = v31 ^ v32 + v33 - v34 & v35 | v36 * v37 + v38 - v39 ^ v40;
    
    /* Mix in all the remaining variables */
    result += v1 - v2 + v3 * v4 / (v5 + 1);
    result ^= v6 | v7 & v8;
    result -= v9 + v10 - v11 * v12;
    result |= v13 ^ v14 & v15;
    result *= v16 + v17 - v18;
    result &= v19 | v20 ^ v21;
    result += v22 - v23 + v24 * v25;
    result ^= v26 | v27 & v28;
    result -= v29 + v30 - v31 * v32;
    
    /* Final aggregation to prevent dead code elimination */
    return result & 0x7FFFFFFF; /* Keep positive */
}

int main(int argc, char *argv[]) {
    volatile int loop_limit;
    
    /* Use command line arguments to determine loop count */
    if (argc > 1) {
        loop_limit = atoi(argv[1]);
        if (loop_limit <= 0) loop_limit = 100;
    } else {
        loop_limit = 50;
    }
    
    /* Use volatile to prevent loop unrolling */
    volatile int i;
    int total_result = 0;
    
    /* Loop to ensure function is called multiple times */
    for (i = 0; i < loop_limit; i++) {
        /* Mix global volatiles with loop counter */
        int input1 = global_seed1 + i;
        int input2 = global_seed2 - i;
        int input3 = global_seed3 ^ i;
        
        /* Call the high-pressure function */
        int result = create_reload_pressure(input1, input2, input3);
        
        /* Accumulate results with complex expression */
        total_result ^= result + i;
        total_result = (total_result << 3) | (total_result >> 29); /* Rotate */
        
        /* Occasionally modify globals to create side effects */
        if ((i & 15) == 0) {
            global_seed1 ^= result;
            global_seed2 += i;
            global_seed3 -= result & 0xFF;
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Final result: %d\n", total_result & 0xFF);
    
    return (total_result & 0xFF) == 0 ? 0 : 1;
}
