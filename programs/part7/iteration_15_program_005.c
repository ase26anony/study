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
    /* Declare many local variables of different types to exhaust registers */
    int v1 = input1;
    int v2 = input2;
    int v3 = input3;
    int v4 = v1 * v2;
    int v5 = v2 + v3;
    int v6 = v3 - v1;
    short s1 = (short)(v1 & 0xFFFF);
    short s2 = (short)(v2 & 0xFFFF);
    short s3 = (short)(v3 & 0xFFFF);
    char c1 = (char)(v1 & 0xFF);
    char c2 = (char)(v2 & 0xFF);
    char c3 = (char)(v3 & 0xFF);
    long l1 = (long)v1 * v2;
    long l2 = (long)v2 * v3;
    long l3 = (long)v3 * v1;
    int v7 = v4 ^ v5;
    int v8 = v5 | v6;
    int v9 = v6 & v4;
    int v10 = v7 << 3;
    int v11 = v8 >> 2;
    int v12 = v9 << 1;
    int v13 = v10 + v11;
    int v14 = v11 - v12;
    int v15 = v12 * v13;
    int v16 = v13 / (v14 ? v14 : 1);
    int v17 = v14 % (v15 ? v15 : 1);
    int v18 = v15 ^ v16;
    int v19 = v16 | v17;
    int v20 = v17 & v18;
    int v21 = v18 << (v19 & 3);
    int v22 = v19 >> (v20 & 3);
    int v23 = v20 << (v21 & 3);
    int v24 = v21 >> (v22 & 3);
    int v25 = v22 + v23;
    int v26 = v23 - v24;
    int v27 = v24 * v25;
    int v28 = v25 ^ v26;
    int v29 = v26 | v27;
    int v30 = v27 & v28;
    
    /* Complex addressing mode simulation with inline assembly */
    /* Force specific register constraints to create conflicts */
    asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3) : : "cc", "memory");
    
    /* Complex expressions requiring multiple operands simultaneously */
    v1 = (v4 * v5) + (v6 << (v7 & 7)) - (v8 >> (v9 & 7));
    v2 = ((v10 & v11) | (v12 ^ v13)) * ((v14 + v15) - (v16 % (v17 ? v17 : 1)));
    v3 = ((v18 << 2) | (v19 >> 3)) + ((v20 & 0xF0F0) ^ (v21 | 0x0F0F));
    
    /* More complex operations with mixed types */
    l1 = (long)v1 * (long)v2 + (long)v3;
    l2 = (long)v4 * (long)v5 - (long)v6;
    l3 = (long)v7 * (long)v8 / ((long)v9 ? (long)v9 : 1);
    
    s1 = (short)((v10 + v11) & 0xFFFF);
    s2 = (short)((v12 - v13) & 0xFFFF);
    s3 = (short)((v14 * v15) & 0xFFFF);
    
    c1 = (char)((v16 ^ v17) & 0xFF);
    c2 = (char)((v18 | v19) & 0xFF);
    c3 = (char)((v20 & v21) & 0xFF);
    
    /* Even more operations to keep values live */
    v4 = v22 + v23 - v24;
    v5 = v25 * v26 / (v27 ? v27 : 1);
    v6 = (v28 << 4) | (v29 >> 4);
    v7 = v30 ^ v1 ^ v2;
    v8 = (v3 & v4) | (v5 ^ v6);
    v9 = (v7 << 1) + (v8 >> 1);
    
    /* Another inline asm to force register pressure */
    asm volatile("" : "+r"(v4), "+r"(v5), "+r"(v6), "+r"(v7), "+r"(v8), "+r"(v9)
                 : : "cc", "memory");
    
    /* Complex expression using many live variables simultaneously */
    int result = v1 + v2 - v3 * v4 / (v5 ? v5 : 1) + 
                 (v6 << (v7 & 3)) - (v8 >> (v9 & 3)) +
                 (v10 ^ v11 | v12) & (v13 + v14 - v15) +
                 (v16 % (v17 ? v17 : 1)) * (v18 & v19 | v20) -
                 (v21 << 2) + (v22 >> 2) * (v23 & 0xFF) +
                 (v24 | v25) ^ (v26 & v27) + (v28 - v29) * v30;
    
    /* Use all variables in final computation to prevent elimination */
    result += s1 + s2 + s3 + c1 + c2 + c3;
    result += (int)(l1 & 0xFFFFFFFF) + (int)(l2 & 0xFFFFFFFF) + 
              (int)(l3 & 0xFFFFFFFF);
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_limit < 1) loop_limit = 1;
    if (loop_limit > 1000) loop_limit = 1000;
    
    int total = 0;
    
    /* Loop with volatile limit to prevent unrolling */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* Use volatile globals and argc to create unknown inputs */
        int input1 = global_seed1 + argc + i;
        int input2 = global_seed2 * argc - i;
        int input3 = global_seed3 ^ argc ^ i;
        
        /* Call the high-pressure function */
        total += create_reload_pressure(input1, input2, input3);
        
        /* Modify globals slightly to change inputs */
        global_seed1 = (global_seed1 * 1103515245 + 12345) & 0x7FFFFFFF;
        global_seed2 = (global_seed2 * 1664525 + 1013904223) & 0x7FFFFFFF;
        global_seed3 = (global_seed3 * 214013 + 2531011) & 0x7FFFFFFF;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
