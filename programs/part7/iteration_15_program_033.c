/* reload_coverage.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_seed = 42;
volatile int global_mask = 0xFFFF;

/* Non-inlineable function with extreme register pressure */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3) {
    /* Declare many local variables with different types to exhaust registers */
    int v1 = input1 + 1;
    int v2 = input2 * 2;
    int v3 = v1 ^ v2;
    short v4 = (short)(input3 & 0xFFFF);
    char v5 = (char)(input1 & 0xFF);
    long v6 = (long)input1 * input2;
    int v7 = v3 | (int)v4;
    int v8 = v2 - v1;
    short v9 = (short)(v4 + v5);
    char v10 = (char)(v5 ^ 0x55);
    int v11 = v7 << 2;
    long v12 = v6 + (long)v8;
    int v13 = v11 & 0x0F0F0F0F;
    short v14 = v9 >> 1;
    char v15 = v10 + 1;
    int v16 = v13 | v8;
    long v17 = v12 - (long)v11;
    int v18 = v16 ^ v1;
    short v19 = (short)(v14 * 2);
    char v20 = (char)(v15 | 0x0F);
    int v21 = v18 + v3;
    long v22 = v17 * 3;
    int v23 = v21 & v13;
    short v24 = v19 + v14;
    char v25 = v20 ^ v15;
    int v26 = v23 << v5;
    long v27 = v22 / 2;
    int v28 = v26 | v16;
    short v29 = v24 - v19;
    char v30 = (char)(v25 + v20);
    
    /* Complex expressions with multiple operands - forces reload decisions */
    v1 = (v2 & v3) | (v4 << (v5 & 3)) - v6;
    v7 = v8 * v9 - v10 + (v11 >> 2);
    v12 = (v13 ^ v14) + (v15 * v16) - (v17 & 0xFF);
    v18 = ((v19 | v20) << 3) & (v21 ^ v22);
    v23 = (v24 * v25) + (v26 / 2) - (v27 % 100);
    v28 = (v29 & v30) | (v1 << 1) ^ (v2 >> 2);
    
    /* More complex expressions with data dependencies */
    int t1 = v1 + v2 + v3 + v4;
    int t2 = v5 * v6 - v7 / 2;
    short t3 = (short)(v8 & v9 | v10);
    char t4 = (char)(v11 ^ v12 ^ v13);
    long t5 = v14 * v15 + v16 - v17;
    
    /* Even more variables to increase pressure */
    int v31 = t1 ^ t2;
    short v32 = t3 + t4;
    char v33 = (char)(t5 & 0xFF);
    int v34 = v31 << (v32 & 3);
    int v35 = v33 * v34;
    short v36 = (short)(v35 & 0xFFFF);
    char v37 = (char)(v36 >> 4);
    int v38 = v37 + v31;
    long v39 = (long)v38 * v35;
    int v40 = v39 & 0xFFFFFFFF;
    
    /* Inline assembly to create specific register constraints */
    asm volatile (
        "addl %%ebx, %%eax\n\t"
        "subl %%ecx, %%edx\n\t"
        : "+r" (v1), "+r" (v2)
        : "r" (v3), "r" (v4)
        : "cc"
    );
    
    /* More arithmetic to keep values live */
    v5 = (v1 & v2) | (v3 ^ v4);
    v6 = v5 * v6 + v7 - v8;
    v9 = (v10 << 2) | (v11 >> 3);
    v12 = v13 + v14 - v15 * v16;
    
    /* Complex addressing mode simulation */
    int* ptr1 = &v1;
    int* ptr2 = &v2;
    short* ptr3 = &v9;
    char* ptr4 = &v10;
    
    /* Force memory accesses that compete for address registers */
    volatile int mem1 = *ptr1 + *ptr2;
    volatile short mem2 = *ptr3 + *ptr4;
    
    /* More operations using pointer values */
    v17 = (long)(*ptr1) * (*ptr2);
    v18 = (*ptr3) | (*ptr4 << 8);
    
    /* Final aggregation to prevent dead code elimination */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ (int)v7 ^ (int)v8 
                 ^ v9 ^ v10 ^ v11 ^ (int)v12 ^ v13 ^ v14 ^ v15 
                 ^ v16 ^ (int)(v17 & 0xFFFFFFFF) ^ v18 ^ v19 
                 ^ v20 ^ v21 ^ v22 ^ v23 ^ v24 ^ v25 ^ v26 
                 ^ v27 ^ v28 ^ v29 ^ v30 ^ v31 ^ v32 ^ v33 
                 ^ v34 ^ v35 ^ v36 ^ v37 ^ v38 ^ v39 ^ v40
                 ^ t1 ^ t2 ^ t3 ^ t4 ^ (int)(t5 & 0xFFFFFFFF)
                 ^ mem1 ^ mem2;
    
    return result & 0xFF; /* Return small value to avoid overflow issues */
}

int main(int argc, char *argv[]) {
    /* Use command line arguments to create variable inputs */
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_limit < 1) loop_limit = 1;
    if (loop_limit > 10000) loop_limit = 10000;
    
    volatile int seed1 = argc + global_seed;
    volatile int seed2 = argc * 3;
    volatile int seed3 = global_seed ^ argc;
    
    int total_result = 0;
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* Mix up inputs slightly each iteration */
        int input1 = seed1 + i;
        int input2 = seed2 - i;
        int input3 = seed3 ^ i;
        
        /* Call the high-pressure function */
        int result = create_reload_pressure(input1, input2, input3);
        
        /* Accumulate results with complex expression */
        total_result = (total_result ^ result) + (i & 0xF);
        
        /* Prevent loop unrolling with volatile side effect */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Final result: %d\n", total_result & 0xFF);
    
    return (total_result & 0xFF) == 0 ? 0 : 1;
}
