/* test_reload.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent constant propagation */
volatile int global_seed1 = 12345;
volatile int global_seed2 = 67890;
volatile int global_seed3 = 54321;

/* Prevent inlining and inter-procedural optimization */
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
    int v10 = v2 | v3;
    int v11 = v3 & v1;
    short v12 = (short)(v4 + v5);
    char v13 = (char)(v6 ^ 0x55);
    long v14 = v7 - v8;
    int v15 = v9 << 2;
    int v16 = v10 >> 1;
    int v17 = v11 * 3;
    short v18 = (short)(v12 | 0xAA);
    char v19 = (char)(v13 & 0xF0);
    long v20 = v14 + 1000;
    int v21 = v15 ^ v16;
    int v22 = v17 + v1;
    int v23 = v9 - v10;
    short v24 = (short)(v12 * 2);
    char v25 = (char)(v13 + 1);
    long v26 = v20 ^ v7;
    int v27 = v21 & v22;
    int v28 = v23 | v15;
    int v29 = v16 + v17;
    short v30 = (short)(v18 - v4);
    
    /* Complex expressions requiring multiple registers simultaneously */
    v1 = (v2 & v3) | (v4 << (v5 & 3));
    v6 = (v7 % 256) + (v8 & 0xFF);
    v9 = (v10 * v11) - (v12 + v13);
    v14 = (v15 ^ v16) | (v17 & v18);
    v19 = (v20 >> 8) + (v21 << 2);
    v22 = (v23 + v24) * (v25 - v26);
    v27 = (v28 & v29) | (v30 ^ v1);
    
    /* More complex expressions with multiple uses of same variables */
    v2 = v3 + (v4 * v5) - (v6 << (v7 & 3));
    v8 = (v9 & v10) | (v11 ^ v12) + (v13 * v14);
    v15 = (v16 >> (v17 & 3)) + (v18 << (v19 & 3));
    v20 = v21 - v22 + v23 * v24 - v25;
    
    /* Use inline assembly to create specific register constraints */
    asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3) : : "cc", "memory");
    asm volatile("" : "+m"(v4), "+r"(v5), "+r"(v6) : : "cc");
    
    /* More operations keeping many values live */
    v7 = v8 ^ v9 ^ v10;
    v11 = v12 + v13 + v14;
    v15 = v16 * v17 / (v18 + 1);
    v19 = v20 & v21 | v22;
    v23 = v24 ^ v25 ^ v26;
    v27 = v28 + v29 - v30;
    
    /* Create addressing mode pressure with array accesses */
    int temp_array[8];
    temp_array[0] = v1;
    temp_array[1] = v2;
    temp_array[2] = v3;
    temp_array[3] = v4;
    temp_array[4] = v5;
    temp_array[5] = v6;
    temp_array[6] = v7;
    temp_array[7] = v8;
    
    /* More complex operations using array elements */
    v9 = temp_array[0] + temp_array[1] * temp_array[2];
    v10 = temp_array[3] | temp_array[4] & temp_array[5];
    v11 = temp_array[6] ^ temp_array[7] + temp_array[0];
    
    /* Final aggregation to prevent dead code elimination */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10;
    result += v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18;
    result += v19 ^ v20 ^ v21 ^ v22 ^ v23 ^ v24 ^ v25;
    result += v26 ^ v27 ^ v28 ^ v29 ^ v30;
    
    return result & 0x7FFFFFFF; /* Ensure positive result */
}

int main(int argc, char *argv[]) {
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_limit < 1) loop_limit = 1;
    if (loop_limit > 10000) loop_limit = 10000;
    
    int total_result = 0;
    
    /* Loop to ensure the function is called multiple times */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* Use volatile globals and argc to create unknown values */
        int input1 = global_seed1 + argc + i;
        int input2 = global_seed2 - argc * i;
        int input3 = global_seed3 ^ (argc + i);
        
        total_result ^= create_reload_pressure(input1, input2, input3);
        
        /* Modify globals slightly to change inputs */
        global_seed1 = (global_seed1 * 1103515245 + 12345) & 0x7FFFFFFF;
        global_seed2 = (global_seed2 * 1664525 + 1013904223) & 0x7FFFFFFF;
    }
    
    /* Use the result to prevent optimization */
    printf("Result: %d\n", total_result & 0xFF);
    
    return (total_result & 0xFF) == 0 ? 0 : 1;
}
