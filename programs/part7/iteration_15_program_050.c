/* reload_coverage.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 42;
volatile int global_mask = 0xFFFF;
volatile int global_inc = 1;

/* Prevent inlining and IPA optimizations */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3) {
    /* Declare many local variables of different types to exhaust registers */
    int v1 = input1 + global_inc;
    int v2 = input2 - global_inc;
    int v3 = v1 * v2;
    short v4 = (short)(v3 >> 8);
    char v5 = (char)(v3 & 0xFF);
    long v6 = (long)v1 * (long)v2;
    int v7 = v1 ^ v2;
    int v8 = v3 | v7;
    short v9 = (short)(v4 + v5);
    char v10 = (char)(v5 * 2);
    int v11 = input3 + global_seed;
    int v12 = v11 << 3;
    int v13 = v12 & global_mask;
    long v14 = v6 + (long)v13;
    int v15 = ~v8;
    short v16 = (short)(v9 - v10);
    char v17 = (char)(v10 ^ 0x55);
    int v18 = v13 * v15;
    int v19 = v18 >> 4;
    long v20 = v14 - (long)v19;
    int v21 = v7 + v8 + v11;
    short v22 = (short)(v16 * 3);
    char v23 = (char)(v17 | 0xAA);
    int v24 = v19 ^ v21;
    int v25 = v24 & 0x0F0F0F0F;
    long v26 = v20 * 2L;
    int v27 = v25 + v18;
    short v28 = (short)(v22 + v23);
    char v29 = (char)(v23 ^ v17);
    int v30 = v27 - v24;
    
    /* Complex expressions with multiple operands - forces reload decisions */
    v1 = (v2 & v3) | (v4 << (v5 & 7));
    v6 = (long)v7 * (long)v8 - (long)v9;
    v10 = (char)((v11 * v12) >> (v13 & 3));
    v14 = v14 + (long)((v15 * v16) / (v17 + 1));
    v18 = ((v19 << 2) + (v20 >> 3)) ^ v21;
    v22 = (short)((v23 * v24) & (v25 | 0x1234));
    v26 = v26 - (long)((v27 * v28) / (v29 + 1));
    v30 = ((v1 ^ v2) << 4) | ((v3 & v4) >> 2);
    
    /* More interleaved operations to keep values live */
    int t1 = v1 + v2 + v3 + v4;
    int t2 = v5 * v6 + v7 - v8;
    short t3 = (short)(v9 | v10) + (short)(v11 & v12);
    char t4 = (char)(v13 ^ v14) * (char)(v15 | v16);
    long t5 = (long)v17 * (long)v18 - (long)v19;
    int t6 = v20 ^ v21 ^ v22 ^ v23;
    
    /* Inline assembly to create register constraints and prevent optimizations */
    asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3) : : "cc", "memory");
    asm volatile("" : "+m"(v4), "+r"(v5) : : "cc");
    
    /* More complex expressions */
    v24 = (v25 << (v26 & 15)) | (v27 >> (v28 & 15));
    v29 = (char)((v30 & 0x7F) + (t1 & 0x7F));
    t2 = t2 * t3 - t4;
    t5 = t5 + (long)t6 * 2L;
    
    /* Additional inline assembly with specific constraints */
    asm volatile("addl %1, %0" : "+r"(v1) : "r"(v2) : "cc");
    asm volatile("movw %1, %0" : "=r"(t3) : "r"(v4) : "cc");
    
    /* Final computation using all variables to prevent dead code elimination */
    int result = v1 ^ v2 ^ v3 ^ (v4 << 8) ^ (v5 << 16) ^ 
                 (v6 & 0xFFFFFFFF) ^ (v7 << 4) ^ v8 ^ 
                 (v9 << 8) ^ (v10 << 16) ^ v11 ^ v12 ^ 
                 v13 ^ (v14 & 0xFFFFFFFF) ^ v15 ^ 
                 (v16 << 8) ^ (v17 << 16) ^ v18 ^ v19 ^ 
                 (v20 & 0xFFFFFFFF) ^ v21 ^ v22 ^ 
                 (v23 << 8) ^ v24 ^ (v25 << 4) ^ 
                 (v26 & 0xFFFFFFFF) ^ v27 ^ v28 ^ 
                 (v29 << 8) ^ v30 ^ t1 ^ t2 ^ 
                 (t3 << 8) ^ (t4 << 16) ^ (t5 & 0xFFFFFFFF) ^ t6;
    
    return result & 0xFF; /* Return small value to avoid overflow issues */
}

int main(int argc, char *argv[]) {
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    volatile int total = 0;
    
    /* Use command line arguments and volatile variables as inputs */
    volatile int input_base = argc;
    volatile int input_mul = (argc > 1) ? atoi(argv[1]) : 1;
    volatile int input_add = (argc > 2) ? atoi(argv[2]) : 0;
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* Varying inputs to prevent constant propagation */
        int input1 = input_base + i;
        int input2 = input_mul * i;
        int input3 = input_add ^ i;
        
        /* Call the high-pressure function */
        int result = create_reload_pressure(input1, input2, input3);
        
        /* Accumulate results with volatile access */
        total += result;
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return (total & 0xFF) == 0 ? 0 : 1;
}
