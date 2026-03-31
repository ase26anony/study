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
    /* Declare many local variables with different types to exhaust registers */
    int v1 = input1 + 1;
    int v2 = input2 - 1;
    int v3 = v1 * v2;
    short v4 = (short)(input3 & 0xFFFF);
    char v5 = (char)(input1 ^ input2);
    long v6 = (long)input1 * (long)input2;
    int v7 = v3 + v1;
    int v8 = v2 << 2;
    short v9 = (short)(v4 + 5);
    char v10 = (char)(v5 | 0x7F);
    long v11 = v6 >> 3;
    int v12 = v7 ^ v8;
    int v13 = v12 + v3;
    short v14 = (short)(v9 * 2);
    char v15 = (char)(v10 & 0x3F);
    long v16 = v11 + v6;
    int v17 = v13 - v7;
    int v18 = v8 | v12;
    short v19 = (short)(v14 + v9);
    char v20 = (char)(v15 ^ v5);
    long v21 = v16 * 3;
    int v22 = v17 + v18;
    int v23 = v22 * v13;
    short v24 = (short)(v19 - v14);
    char v25 = (char)(v20 | v10);
    long v26 = v21 / 2;
    int v27 = v23 ^ v17;
    int v28 = v27 + v22;
    short v29 = (short)(v24 * v19);
    char v30 = (char)(v25 & v15);
    
    /* Complex expressions with many live values simultaneously */
    v1 = (v2 & v3) | (v4 << (v5 & 3));
    v6 = (v7 * v8) - (v9 * v10) + (v11 >> 2);
    v12 = ((v13 + v14) * (v15 - v16)) ^ (v17 | v18);
    v19 = (short)((v20 * v21) + (v22 / v23) - (v24 ^ v25));
    v26 = (v27 << (v28 & 7)) + (v29 * v30) - (v1 * v2);
    
    /* More operations keeping values live */
    v3 = v4 + v5 + v6 + v7 + v8;
    v9 = (short)(v10 * v11 + v12 - v13);
    v14 = (short)((v15 | v16) & (v17 ^ v18));
    v19 = (short)(v20 + v21 + v22 + v23);
    v24 = (short)(v25 * v26 / v27 + v28);
    
    /* Inline assembly to create register constraints */
    asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3) : : "cc", "memory");
    asm volatile("" : "+m"(v4), "+r"(v5), "+r"(v6) : : "cc");
    
    /* More complex expressions */
    v7 = ((v8 << 3) + (v9 * v10)) | ((v11 >> 2) & (v12 ^ v13));
    v14 = (short)((v15 + v16) * (v17 - v18) / (v19 | 1));
    v20 = (char)((v21 & v22) | (v23 ^ v24) + (v25 * v26));
    v27 = v28 + v29 + v30 + v1 + v2 + v3;
    
    /* Force memory accesses with volatile */
    volatile int mem1 = v4;
    volatile short mem2 = v9;
    volatile char mem3 = v15;
    
    /* More operations after memory access */
    v5 = (mem1 * v6) + (mem2 / v7) - (mem3 ^ v8);
    v10 = (char)((v11 & v12) | (v13 << (v14 & 3)));
    v16 = (v17 * v18) - (v19 * v20) + (v21 >> 1);
    
    /* Final aggregation to prevent dead code elimination */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10 
                 ^ v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18 ^ v19 ^ v20
                 ^ v21 ^ v22 ^ v23 ^ v24 ^ v25 ^ v26 ^ v27 ^ v28 ^ v29 ^ v30;
    
    return result & 0xFF; /* Return small value */
}

int main(int argc, char *argv[]) {
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    volatile int seed1 = argc + global_seed1;
    volatile int seed2 = argc * 2 + global_seed2;
    volatile int seed3 = argc * 3 + global_seed3;
    
    int total_result = 0;
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* Mix up inputs to prevent constant propagation */
        int input1 = seed1 + i;
        int input2 = seed2 - i;
        int input3 = seed3 ^ i;
        
        /* Call the high-pressure function */
        int result = create_reload_pressure(input1, input2, input3);
        
        /* Accumulate results with complex expression */
        total_result = (total_result * 31 + result) & 0xFFFFFF;
        
        /* Modify seeds to create varying inputs */
        seed1 = (seed1 * 1103515245 + 12345) & 0x7FFFFFFF;
        seed2 = (seed2 * 1664525 + 1013904223) & 0x7FFFFFFF;
        seed3 = (seed3 * 214013 + 2531011) & 0x7FFFFFFF;
    }
    
    /* Use the result to prevent optimization */
    printf("Final result: %d\n", total_result & 0xFF);
    
    return (total_result & 0xFF) == 0 ? 0 : 1;
}
