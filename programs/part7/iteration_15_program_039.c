/* reload_coverage.c
 * Program to trigger GCC's reload pass initialization code
 * Specifically targets lines 1381-1399 in reload.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent constant propagation */
volatile int global_seed1 = 12345;
volatile int global_seed2 = 67890;
volatile short global_short = 1000;
volatile char global_char = 42;

/* Prevent inlining and IPA optimizations */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, short input3, char input4) {
    /* Declare many local variables to exhaust registers */
    int v1 = input1;
    int v2 = input2 + 1;
    int v3 = v1 * v2;
    int v4 = input3 * 2;
    int v5 = input4 << 3;
    short v6 = (short)(v1 + v2);
    short v7 = (short)(v3 - v4);
    char v8 = (char)(v5 & 0xFF);
    char v9 = (char)(v4 | 0x7F);
    
    /* More variables with different types */
    long v10 = (long)v1 * v2;
    long v11 = (long)v3 + v4;
    int v12 = v6 * v7;
    int v13 = v8 ^ v9;
    short v14 = (short)(v12 >> 2);
    char v15 = (char)(v13 & 0x3F);
    
    /* Additional variables to increase pressure */
    int v16 = v10 & 0xFFFFFFFF;
    int v17 = v11 & 0xFFFFFFFF;
    int v18 = v14 * 3;
    int v19 = v15 + 64;
    int v20 = v16 | v17;
    int v21 = v18 ^ v19;
    int v22 = v20 << 1;
    int v23 = v21 >> 1;
    int v24 = v22 + v23;
    int v25 = v24 * 7;
    int v26 = v25 - 100;
    int v27 = v26 / 3;
    int v28 = v27 | 0x5555;
    int v29 = v28 & 0xAAAA;
    int v30 = v29 ^ 0x1234;
    
    /* Complex expressions with many operands - forces reloads */
    v1 = (v2 & v3) | (v4 << (v5 & 0x3));
    v6 = (short)((v7 * v8) + (v9 << 2) - v10);
    v11 = (v12 * v13) + (v14 << (v15 & 0x7)) - (v16 >> 1);
    v17 = ((v18 & v19) | (v20 ^ v21)) + ((v22 << 3) - (v23 >> 2));
    
    /* More complex expressions keeping many values live */
    v24 = (v25 * v26) + (v27 << (v28 & 0xF)) - (v29 >> (v30 & 0x3));
    v10 = (v11 & v12) | (v13 << 1) ^ (v14 >> 2);
    v15 = (char)((v16 + v17) & 0xFF);
    v18 = (v19 | v20) ^ (v21 & v22);
    
    /* Inline assembly to create register constraints */
    asm volatile("" : "+r"(v1), "+r"(v2) : : "cc", "memory");
    asm volatile("" : "+m"(v3), "+r"(v4) : : "cc");
    
    /* More operations with volatile memory accesses */
    volatile int mem1 = v5;
    volatile int mem2 = v6;
    v7 = mem1 + mem2;
    
    volatile long mem3 = v10;
    volatile long mem4 = v11;
    v12 = (int)(mem3 ^ mem4);
    
    /* Additional complex expressions */
    v13 = ((v14 * v15) << 2) | ((v16 + v17) >> 1);
    v18 = (v19 ^ v20) + (v21 & v22) - (v23 | v24);
    v25 = (v26 << (v27 & 0x7)) | (v28 >> (v29 & 0x7));
    v30 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    
    /* Even more operations to ensure all values are used */
    v30 += v10 + v11 + v12 + v13 + v14 + v15;
    v30 += v16 + v17 + v18 + v19 + v20 + v21;
    v30 += v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    
    /* Create addressing mode pressure */
    int* ptr = &v30;
    volatile int* volatile_ptr = ptr;
    v1 = *volatile_ptr + v2;
    
    /* More memory operations */
    int arr[4] = {v3, v4, v5, v6};
    volatile int vol_arr[4];
    for (int i = 0; i < 4; i++) {
        vol_arr[i] = arr[i];
        v7 += vol_arr[i];
    }
    
    /* Final aggregation to prevent dead code elimination */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9;
    result ^= v10 ^ v11 ^ v12 ^ v13 ^ v14 ^ v15;
    result ^= v16 ^ v17 ^ v18 ^ v19 ^ v20 ^ v21;
    result ^= v22 ^ v23 ^ v24 ^ v25 ^ v26 ^ v27 ^ v28 ^ v29 ^ v30;
    
    return result;
}

int main(int argc, char** argv) {
    /* Use command line arguments to prevent constant folding */
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_limit < 1) loop_limit = 100;
    
    int total_result = 0;
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* Mix different types and volatile inputs */
        int result = create_reload_pressure(
            global_seed1 + i,
            global_seed2 - i,
            global_short + (short)i,
            global_char + (char)(i & 0xFF)
        );
        
        total_result ^= result;
        
        /* Modify globals to create side effects */
        global_seed1 += result & 0xF;
        global_seed2 -= result & 0x7;
        global_short ^= (short)result;
        global_char += (char)(result & 0x3F);
    }
    
    /* Use the result to prevent optimization */
    printf("Result: 0x%08x\n", total_result & 0xFFFFFFFF);
    
    return (total_result & 0xFF);
}
