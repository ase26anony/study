/* test_reload.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_seed = 42;
volatile int global_mask = 0xFFFF;

/* NOINLINE function to create maximum register pressure */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3) {
    /* Declare many local variables to exhaust registers */
    int v1 = input1 + 1;
    int v2 = input2 * 2;
    int v3 = input3 & 0xFF;
    short v4 = (short)(input1 * 3);
    char v5 = (char)(input2 + 5);
    long v6 = (long)input1 * input2;
    int v7 = v1 ^ v2;
    short v8 = (short)(v3 + v4);
    char v9 = (char)(v5 * 2);
    long v10 = v6 >> 2;
    int v11 = v7 | v3;
    int v12 = v1 * v2 - v3;
    short v13 = (short)(v4 + v8);
    char v14 = (char)(v9 ^ 0x55);
    long v15 = v10 * 3;
    int v16 = v11 & v12;
    int v17 = v7 << 2;
    short v18 = (short)(v13 | 0x1234);
    char v19 = (char)(v14 + 67);
    long v20 = v15 - v6;
    int v21 = v16 + v17;
    int v22 = v12 * v11;
    short v23 = (short)(v18 & 0x00FF);
    char v24 = (char)(v19 * 3);
    long v25 = v20 / 2;
    int v26 = v21 ^ v22;
    int v27 = v1 + v2 + v3;
    short v28 = (short)(v23 + v13);
    char v29 = (char)(v24 | 0xF0);
    long v30 = v25 + v15;
    
    /* Complex expressions to force reloads */
    v1 = (v2 & v3) | (v4 << (v5 & 3));
    v6 = v7 * v8 - v9 + v10;
    v11 = (v12 << 3) ^ (v13 >> 1) & v14;
    v15 = v16 * v17 / (v18 + 1);
    v19 = (v20 & 0xFF) | ((v21 % 256) << 8);
    v22 = v23 * v24 + v25 - v26;
    v27 = (v28 & v29) | (v30 >> 4);
    
    /* More complex operations with many live values */
    int t1 = v1 + v2;
    int t2 = v3 * v4;
    int t3 = v5 & v6;
    int t4 = v7 | v8;
    int t5 = v9 ^ v10;
    int t6 = v11 << 2;
    int t7 = v12 >> 1;
    int t8 = v13 + v14;
    int t9 = v15 * v16;
    int t10 = v17 & v18;
    
    /* Inline assembly to force specific register constraints */
    asm volatile("" : "+r"(v1), "+r"(v2) : : "cc", "memory");
    asm volatile("" : "+m"(v3), "+r"(v4) : : "cc");
    
    /* More arithmetic creating intermediate values */
    v1 = t1 * t2 - t3;
    v2 = (t4 & t5) | (t6 ^ t7);
    v3 = t8 + t9 * t10;
    v4 = (t1 << 3) + (t2 >> 2) - t3;
    v5 = t4 | t5 & t6;
    
    /* Even more variables to increase pressure */
    int u1 = v1 + v2;
    int u2 = v3 * v4;
    int u3 = v5 ^ v6;
    int u4 = v7 & v8;
    int u5 = v9 | v10;
    int u6 = v11 + v12;
    int u7 = v13 * v14;
    int u8 = v15 ^ v16;
    int u9 = v17 & v18;
    int u10 = v19 | v20;
    
    /* Complex expression with many operands */
    int result = (u1 * u2) + (u3 << (u4 & 3)) - (u5 & u6) | 
                 (u7 >> 1) ^ (u8 * u9) + (u10 & 0xFF);
    
    /* Use all variables in final computation to prevent DCE */
    result += v21 + v22 + v23 + v24 + v25;
    result ^= v26 | v27 | v28 | v29;
    result &= (v30 & 0xFFFFFFFF);
    
    /* More arithmetic to keep values live */
    result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
    
    return result;
}

/* Another high-pressure function with different patterns */
__attribute__((noinline, noipa))
int secondary_pressure(int base) {
    /* Use different types and operations */
    unsigned int ua = base * 3;
    unsigned int ub = base + 0x12345678;
    unsigned short us1 = (ua >> 16) & 0xFFFF;
    unsigned short us2 = ub & 0xFFFF;
    unsigned char uc1 = ua & 0xFF;
    unsigned char uc2 = (ub >> 8) & 0xFF;
    
    /* Force memory operations */
    volatile unsigned int mem1 = ua;
    volatile unsigned short mem2 = us1;
    volatile unsigned char mem3 = uc1;
    
    /* Complex bit manipulation */
    unsigned int r1 = (ua << 5) | (ub >> 27);
    unsigned int r2 = (us1 * us2) ^ (uc1 << 8 | uc2);
    unsigned int r3 = mem1 + mem2 * mem3;
    
    /* More operations */
    r1 = r1 ^ r2 ^ r3;
    r2 = (r1 << 1) | (r1 >> 31);  /* Rotate left */
    r3 = r2 * 0x9E3779B9;  /* Golden ratio multiplier */
    
    /* Inline assembly with memory constraint */
    asm volatile("" : : "r"(r1), "m"(mem1), "r"(r2), "m"(mem2) : "cc");
    
    return (int)(r1 + r2 + r3);
}

int main(int argc, char *argv[]) {
    /* Use command line arguments to create variable inputs */
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_limit < 1) loop_limit = 1;
    if (loop_limit > 1000) loop_limit = 1000;
    
    volatile int seed1 = argc * 12345;
    volatile int seed2 = global_seed;
    volatile int seed3 = global_mask & 0xFF;
    
    int total_result = 0;
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* Mix both high-pressure functions */
        int r1 = create_reload_pressure(seed1 + i, seed2 * i, seed3 ^ i);
        int r2 = secondary_pressure(r1 + i);
        
        /* Combine results in non-trivial way */
        total_result ^= r1;
        total_result += r2;
        total_result = (total_result << 1) | (total_result >> 31);
        
        /* Modify seeds to create varying inputs */
        seed1 = (seed1 * 1103515245 + 12345) & 0x7FFFFFFF;
        seed2 ^= i;
        seed3 += 1;
    }
    
    /* Use the result to prevent optimization */
    printf("Result: 0x%08X\n", total_result & 0xFFFFFFFF);
    
    return (total_result & 0xFF);
}
