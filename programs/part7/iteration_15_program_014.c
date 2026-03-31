/* test_reload.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 42;
volatile int global_mask = 0xFFFF;
volatile int global_mod = 10007;

/* Prevent inlining and IPA optimizations */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3) {
    /* Declare many local variables to exhaust registers */
    int v1 = input1 + 1;
    int v2 = input2 * 2;
    int v3 = input3 & 0xFF;
    short v4 = (short)(input1 >> 8);
    char v5 = (char)(input2 + 3);
    long v6 = (long)input1 * input2;
    int v7 = v1 ^ v2;
    int v8 = v3 | v4;
    int v9 = v5 * 2;
    int v10 = (int)(v6 & 0xFFFFFFFF);
    int v11 = v7 + v8;
    int v12 = v9 - v10;
    unsigned int v13 = (unsigned int)v11 * 3;
    unsigned short v14 = (unsigned short)v12;
    signed char v15 = (signed char)v13;
    int v16 = v14 + v15;
    long v17 = v6 + v16;
    int v18 = v1 & v2;
    int v19 = v3 | v4;
    int v20 = v5 ^ v6;
    int v21 = v7 - v8;
    int v22 = v9 * v10;
    int v23 = v11 / (v12 ? v12 : 1);
    int v24 = v13 % (v14 ? v14 : 1);
    int v25 = v15 << 2;
    int v26 = v16 >> 1;
    int v27 = v17 & 0xFF;
    int v28 = ~v18;
    int v29 = v19 | v20;
    int v30 = v21 ^ v22;
    
    /* Complex expressions with multiple uses of variables */
    /* This creates many intermediate values that need registers */
    v1 = (v2 & v3) | (v4 << (v5 & 3));
    v6 = (long)(v7 * v8) - (v9 << 2) + (v10 >> 1);
    v11 = ((v12 + v13) * (v14 - v15)) / ((v16 & 0xFF) + 1);
    v17 = v18 * v19 + v20 * v21 - v22 * v23;
    v24 = (v25 | v26) & (v27 ^ v28);
    v29 = ((v30 << v1) >> v2) & ((v3 | v4) ^ v5);
    
    /* More complex expressions with addressing mode constraints */
    v6 = v7 + (v8 * 3) - (v9 / 2) + (v10 & v11) | (v12 ^ v13);
    v14 = (v15 << (v16 & 3)) + (v17 >> (v18 % 4)) - (v19 & v20);
    v21 = ((v22 + global_seed) * (v23 - global_mask)) % global_mod;
    v24 = (v25 | global_seed) & (v26 ^ global_mask);
    
    /* Inline assembly to force specific register constraints */
    /* This creates artificial register pressure and conflicts */
    asm volatile("" : "+r"(v1), "+r"(v2) : : "cc", "memory");
    asm volatile("" : "+m"(v3), "+r"(v4) : : "cc");
    asm volatile("" : "+r"(v5), "+r"(v6), "+r"(v7) : : "cc");
    
    /* More arithmetic to keep values live */
    v8 = v1 * v2 + v3 * v4 - v5 * v6 + v7;
    v9 = (v8 << 3) | (v1 >> 2) & (v2 ^ v3);
    v10 = v4 + v5 * v6 - v7 / (v8 ? v8 : 1) + v9;
    
    /* Additional variables to increase pressure */
    int v31 = v1 + v2;
    int v32 = v3 * v4;
    int v33 = v5 ^ v6;
    int v34 = v7 | v8;
    int v35 = v9 - v10;
    int v36 = v11 & v12;
    int v37 = v13 | v14;
    int v38 = v15 ^ v16;
    int v39 = v17 - v18;
    int v40 = v19 * v20;
    
    /* Complex expression using all variables */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10 ^
                 v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18 ^ v19 ^ v20 ^
                 v21 ^ v22 ^ v23 ^ v24 ^ v25 ^ v26 ^ v27 ^ v28 ^ v29 ^ v30 ^
                 v31 ^ v32 ^ v33 ^ v34 ^ v35 ^ v36 ^ v37 ^ v38 ^ v39 ^ v40;
    
    /* Final inline assembly to prevent optimization */
    asm volatile("" : "+r"(result) : : "cc");
    
    return result;
}

/* Another high-pressure function with different patterns */
__attribute__((noinline, noipa))
int secondary_pressure(int base) {
    /* Use different types and operations */
    unsigned int u1 = base * 3;
    unsigned short u2 = base + 1;
    unsigned char u3 = base & 0xFF;
    signed int s1 = -base;
    signed short s2 = base >> 1;
    signed char s3 = base + 5;
    
    /* Force many intermediate calculations */
    u1 = (u1 * u2) + (u3 << 2);
    s1 = (s1 - s2) * (s3 + 1);
    u2 = (u2 | u1) & (s1 & 0xFFFF);
    s2 = (s2 ^ s3) + (u1 >> 4);
    u3 = (u3 * 2) - (s2 & 0xFF);
    
    /* More variables */
    int t1 = u1 + s1;
    int t2 = u2 * s2;
    int t3 = u3 ^ s3;
    int t4 = t1 | t2;
    int t5 = t3 & t4;
    int t6 = t1 - t2;
    int t7 = t3 + t4;
    int t8 = t5 * t6;
    int t9 = t7 / (t8 ? t8 : 1);
    int t10 = t9 % (global_mod ? global_mod : 1);
    
    /* Complex addressing mode simulation */
    t1 = t2 + (t3 * global_seed) - (t4 & global_mask);
    t5 = (t6 << (t7 & 3)) | (t8 >> (t9 % 4));
    t10 = t1 ^ t5 ^ t2 ^ t3 ^ t4 ^ t6 ^ t7 ^ t8 ^ t9;
    
    return t10;
}

int main(int argc, char **argv) {
    /* Use volatile to prevent loop optimization */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 1;
    if (iterations > 1000) iterations = 1000;
    
    int total_result = 0;
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < iterations; i++) {
        /* Use function arguments and globals as inputs */
        int input1 = argc + i;
        int input2 = global_seed * (i + 1);
        int input3 = global_mask & (i * 3);
        
        /* Call high-pressure functions */
        int result1 = create_reload_pressure(input1, input2, input3);
        int result2 = secondary_pressure(input1 + input2);
        
        /* Combine results to prevent elimination */
        total_result ^= result1;
        total_result += result2;
        total_result &= 0xFFFFFF; /* Keep it bounded */
        
        /* Use inline assembly as a compiler barrier */
        asm volatile("" : : : "memory");
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total_result & 0xFF);
    
    return total_result & 0xFF;
}
