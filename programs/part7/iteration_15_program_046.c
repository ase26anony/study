/* test_reload.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_seed = 42;
volatile int global_mask = 0xFFFF;

/* Force register pressure with many live variables */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3) {
    /* Declare many local variables to exhaust registers */
    int v1 = input1 + 1;
    int v2 = input2 * 2;
    int v3 = input3 & 0xFF;
    short v4 = (short)(input1 ^ input2);
    char v5 = (char)(input3 >> 4);
    long v6 = (long)input1 * input2;
    int v7 = v1 + v2;
    int v8 = v3 | v4;
    int v9 = v5 * 3;
    long v10 = v6 + v7;
    int v11 = v8 ^ v9;
    short v12 = (short)(v10 & 0xFFFF);
    char v13 = (char)(v11 >> 2);
    int v14 = v12 + v13;
    long v15 = v10 * v14;
    int v16 = v11 & v14;
    short v17 = (short)(v15 >> 16);
    char v18 = (char)(v16 | 0x7F);
    int v19 = v17 * v18;
    long v20 = v15 + v19;
    int v21 = v16 << 3;
    short v22 = (short)(v19 >> 8);
    char v23 = (char)(v21 & 0x3F);
    int v24 = v22 + v23;
    long v25 = v20 ^ v24;
    int v26 = v21 | v24;
    short v27 = (short)(v25 & 0x7FFF);
    char v28 = (char)(v26 ^ 0x55);
    int v29 = v27 * v28;
    long v30 = v25 + v29;
    
    /* Complex expressions requiring multiple registers */
    v1 = (v2 & v3) | (v4 << (v5 & 3));
    v6 = v7 * v8 - v9 + (v10 >> 2);
    v11 = (v12 ^ v13) + (v14 & v15) - (v16 | v17);
    v18 = (v19 * v20) / (v21 + 1) ^ (v22 << 2);
    v23 = (v24 & v25) | (v26 ^ v27) + (v28 * v29);
    v30 = v30 + (v1 * v2) - (v3 / v4) + (v5 << v6);
    
    /* More operations to keep values live */
    int t1 = v1 + v2 + v3 + v4;
    int t2 = v5 * v6 * v7 * v8;
    int t3 = v9 ^ v10 ^ v11 ^ v12;
    int t4 = v13 | v14 | v15 | v16;
    int t5 = v17 & v18 & v19 & v20;
    int t6 = v21 + v22 - v23 * v24;
    int t7 = v25 ^ v26 | v27 & v28;
    int t8 = v29 << 2 | v30 >> 4;
    
    /* Inline assembly to force specific register constraints */
    asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3) : : "cc", "memory");
    asm volatile("" : "+m"(v4), "+r"(v5), "+r"(v6) : : "cc");
    
    /* Complex addressing modes */
    int* ptr1 = &v7;
    int* ptr2 = &v8;
    int* ptr3 = &v9;
    
    /* Force memory accesses */
    volatile int mem1 = *ptr1;
    volatile int mem2 = *ptr2;
    volatile int mem3 = *ptr3;
    
    /* More arithmetic with memory results */
    v10 = v10 + mem1 - mem2 * mem3;
    v11 = v11 ^ (mem1 | mem2) & (mem3 << 2);
    
    /* Additional operations mixing all variables */
    int sum1 = t1 + t2 + t3;
    int sum2 = t4 ^ t5 ^ t6;
    int sum3 = t7 | t8 | v1;
    int sum4 = v2 & v3 & v4;
    
    /* Final complex expression using many live values */
    int result = (sum1 * sum2) + (sum3 << (sum4 & 3)) - 
                 (v5 ^ v6) + (v7 & v8) | (v9 << 4) -
                 (v10 >> 2) * (v11 & 0xFF) + 
                 (v12 | v13) ^ (v14 & v15) -
                 (v16 * v17) + (v18 << 1) | 
                 (v19 ^ v20) & (v21 | v22) +
                 (v23 * v24) - (v25 >> 3) ^ 
                 (v26 & v27) | (v28 << 2) -
                 (v29 * v30) + (t1 ^ t2);
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(result) : "memory");
    
    return result;
}

/* Another high-pressure function with different patterns */
__attribute__((noinline, noipa))
int secondary_pressure(int base) {
    /* Different variable types and patterns */
    unsigned int u1 = base * 3;
    unsigned short u2 = base >> 2;
    unsigned char u3 = base & 0x7F;
    signed int s1 = -base;
    signed short s2 = base ^ 0xAAAA;
    signed char s3 = base | 0x55;
    
    /* Complex bitwise operations */
    u1 = (u1 << u3) | (u2 >> (u3 & 7));
    s1 = (s1 & u1) ^ (s2 | u2) + (s3 * u3);
    u2 = (u2 * s1) - (u3 << 4) | (u1 >> 2);
    s2 = (s2 & s3) | (u1 ^ u2) - (s1 * 3);
    u3 = (u3 + s2) * (s3 - u1) ^ (u2 & 0xF);
    
    /* More variables */
    int a1 = u1 + s1;
    int a2 = u2 * s2;
    int a3 = u3 ^ s3;
    int a4 = a1 & a2;
    int a5 = a3 | a4;
    int a6 = a1 + a2 - a3;
    int a7 = a4 * a5 / (a6 + 1);
    int a8 = a5 ^ a6 ^ a7;
    int a9 = a7 & a8 | a6;
    int a10 = a8 * a9 - a7;
    
    /* Force spills with large expression */
    return (a1 << 2) + (a2 >> 1) * (a3 & 0xFF) - 
           (a4 | a5) ^ (a6 & a7) + (a8 * a9) / 
           (a10 + 1) - (u1 ^ u2) * (s1 | s2) + 
           (u3 << 4) & (s3 * 3);
}

int main(int argc, char** argv) {
    /* Use command line arguments for variability */
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_limit < 1) loop_limit = 1;
    if (loop_limit > 1000) loop_limit = 1000;
    
    volatile int seed1 = argc;
    volatile int seed2 = global_seed;
    volatile int seed3 = global_mask & 0xFF;
    
    int total_result = 0;
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* Mix both high-pressure functions */
        int r1 = create_reload_pressure(seed1 + i, seed2 - i, seed3 ^ i);
        int r2 = secondary_pressure(seed1 ^ i);
        
        /* Combine results to prevent optimization */
        total_result ^= r1;
        total_result += r2;
        total_result = (total_result << 1) | (total_result >> 31); /* rotate */
        
        /* Modify seeds to create different patterns */
        seed1 = (seed1 * 1103515245 + 12345) & 0x7FFFFFFF;
        seed2 = (seed2 ^ seed1) + i;
        seed3 = (seed3 * 1664525 + 1013904223) & 0xFFFF;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: 0x%08x\n", total_result & 0xFFFFFFFF);
    
    return (total_result & 0xFF);
}
