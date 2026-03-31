/* test_reload.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent constant propagation */
volatile int global_seed1 = 12345;
volatile int global_seed2 = 67890;
volatile int global_seed3 = 54321;

/* Prevent inlining and IPA optimizations */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3) {
    /* Declare many local variables with different types to create register pressure */
    int v1 = input1;
    int v2 = input2;
    int v3 = input3;
    short v4 = (short)(input1 + 1);
    short v5 = (short)(input2 + 2);
    char v6 = (char)(input3 + 3);
    char v7 = (char)(input1 * 2);
    long v8 = (long)input1 * input2;
    long v9 = (long)input2 * input3;
    int v10 = v1 + v2;
    int v11 = v2 - v3;
    int v12 = v1 * v3;
    int v13 = v10 ^ v11;
    int v14 = v11 | v12;
    int v15 = v12 & v13;
    int v16 = v13 << 2;
    int v17 = v14 >> 1;
    int v18 = v15 + v16;
    int v19 = v16 - v17;
    int v20 = v17 * v18;
    int v21 = v18 / (v19 ? v19 : 1);
    int v22 = v19 % (v20 ? v20 : 1);
    int v23 = v20 ^ v21;
    int v24 = v21 | v22;
    int v25 = v22 & v23;
    int v26 = v23 << 3;
    int v27 = v24 >> 2;
    int v28 = v25 + v26;
    int v29 = v26 - v27;
    int v30 = v27 * v28;
    
    /* Complex expressions with multiple operands to force reloads */
    v1 = (v2 & v3) | (v4 << (v5 & 3));
    v2 = (v6 * v7) + (v8 >> (v9 & 7));
    v3 = (v10 ^ v11) - (v12 | v13);
    v4 = (v14 & v15) + (v16 << (v17 & 3));
    v5 = (v18 * v19) - (v20 >> (v21 & 7));
    
    /* More complex expressions mixing different variable types */
    v6 = (char)((v22 & 0xFF) | ((v23 & 0xFF) << 2));
    v7 = (char)((v24 * v25) & 0xFF);
    v8 = (long)v26 * v27 + (long)v28 * v29;
    v9 = (long)v30 * v1 - (long)v2 * v3;
    
    /* Use inline assembly to create specific register constraints */
    asm volatile("" : "+r"(v10), "+r"(v11) : : "cc", "memory");
    asm volatile("" : "+r"(v12), "+r"(v13) : : "cc", "memory");
    
    /* More arithmetic to keep values live */
    v14 = v10 + v11 + v12 + v13;
    v15 = v10 - v11 * v12 / (v13 ? v13 : 1);
    v16 = (v10 & v11) | (v12 ^ v13);
    v17 = (v10 << 2) + (v11 >> 1);
    v18 = v14 * v15 - v16 * v17;
    v19 = v15 + v16 - v17 + v18;
    v20 = v16 ^ v17 ^ v18 ^ v19;
    
    /* Additional complex expressions */
    v21 = (v14 & v15) | ((v16 & v17) << 1);
    v22 = (v18 * v19) + ((v20 & 0xFF) << 8);
    v23 = (v21 ^ v22) - ((v14 | v15) & 0xFFFF);
    v24 = (v16 << 3) | (v17 >> 2);
    v25 = (v18 + v19) * (v20 - v21);
    
    /* Force memory accesses with volatile pointers */
    volatile int* mem_ptr = &v26;
    *mem_ptr = v22 + v23;
    v27 = *mem_ptr * v24;
    
    /* More register pressure */
    v28 = v25 + v26 + v27;
    v29 = v25 - v26 * v27;
    v30 = (v28 & v29) | (v25 ^ v26);
    
    /* Final complex computation using all variables */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ 
                 v6 ^ v7 ^ (int)(v8 & 0xFFFFFFFF) ^ (int)(v9 & 0xFFFFFFFF) ^
                 v10 ^ v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18 ^ v19 ^
                 v20 ^ v21 ^ v22 ^ v23 ^ v24 ^ v25 ^ v26 ^ v27 ^ v28 ^ v29 ^ v30;
    
    return result;
}

int main(int argc, char** argv) {
    /* Use command line arguments to create variable inputs */
    int input1 = argc + global_seed1;
    int input2 = argc * 2 + global_seed2;
    int input3 = argc * 3 + global_seed3;
    
    /* Volatile loop counter to prevent optimization */
    volatile int loop_count = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_count < 1) loop_count = 1;
    if (loop_count > 1000) loop_count = 1000;
    
    int total_result = 0;
    
    /* Loop to ensure the function is called multiple times */
    for (volatile int i = 0; i < loop_count; i++) {
        /* Mix up inputs slightly each iteration */
        int result = create_reload_pressure(
            input1 + i, 
            input2 - i, 
            input3 ^ i
        );
        total_result ^= result;
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: 0x%08x\n", total_result & 0xFFFFFFFF);
    
    return (total_result & 0xFF);
}
