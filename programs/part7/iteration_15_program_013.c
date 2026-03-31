/* reload_coverage.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_seed1 = 12345;
volatile int global_seed2 = 67890;
volatile int global_seed3 = 13579;
volatile int global_seed4 = 24680;

/* Non-inlineable function to create maximum register pressure */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3, int input4) {
    /* Declare many local variables with different types to exhaust registers */
    int v1 = input1 + 1;
    int v2 = input2 - 1;
    int v3 = input3 * 2;
    int v4 = input4 / 2;
    short v5 = (short)(input1 & 0xFFFF);
    short v6 = (short)(input2 & 0xFFFF);
    char v7 = (char)(input3 & 0xFF);
    char v8 = (char)(input4 & 0xFF);
    long v9 = (long)input1 * input2;
    long v10 = (long)input3 * input4;
    int v11 = v1 ^ v2;
    int v12 = v3 | v4;
    int v13 = v11 + v12;
    int v14 = v13 - v1;
    int v15 = v14 * v2;
    int v16 = v15 / (v3 ? v3 : 1);
    int v17 = v16 << 3;
    int v18 = v17 >> 1;
    int v19 = v18 & 0x7F;
    int v20 = v19 | 0x80;
    int v21 = v20 ^ v11;
    int v22 = v21 + v12;
    int v23 = v22 - v13;
    int v24 = v23 * v14;
    int v25 = v24 / (v15 ? v15 : 1);
    int v26 = v25 << 2;
    int v27 = v26 >> 2;
    int v28 = v27 & 0x3F;
    int v29 = v28 | 0xC0;
    int v30 = v29 ^ v21;
    
    /* Complex addressing mode forcing - use variables as array indices */
    volatile int dummy_array[256];
    int idx1 = v1 & 0xFF;
    int idx2 = v2 & 0xFF;
    int idx3 = v3 & 0xFF;
    int idx4 = v4 & 0xFF;
    
    /* Force memory accesses that compete for address registers */
    int mem1 = dummy_array[idx1];
    int mem2 = dummy_array[idx2];
    int mem3 = dummy_array[idx3];
    int mem4 = dummy_array[idx4];
    
    /* Multi-operand expressions to increase register pressure */
    v1 = (v2 & v3) | (v4 << v5) | (mem1 * mem2);
    v6 = (v7 * v8) - (v9 >> 4) + (mem3 ^ mem4);
    v11 = (v12 + v13) * (v14 - v15) / ((v16 ? v16 : 1) | 1);
    v17 = (v18 << v19) | (v20 >> v21) ^ (v22 & v23);
    
    /* Inline assembly to create specific register constraints */
    asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3) : : "cc", "memory");
    asm volatile("" : "+m"(dummy_array[idx1]), "+m"(dummy_array[idx2]) : : "cc");
    
    /* More complex expressions with many live values */
    int t1 = v1 + v2 + v3 + v4;
    int t2 = v5 * v6 * v7 * v8;
    int t3 = v9 ^ v10 ^ v11 ^ v12;
    int t4 = v13 | v14 | v15 | v16;
    int t5 = v17 & v18 & v19 & v20;
    int t6 = v21 - v22 - v23 - v24;
    int t7 = v25 << (v26 & 3);
    int t8 = v27 >> (v28 & 3);
    int t9 = v29 + v30 + t1 + t2;
    int t10 = t3 * t4 * t5 * t6;
    
    /* Use all variables in final computation to prevent elimination */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ 
                 (v9 & 0xFFFFFFFF) ^ (v10 & 0xFFFFFFFF) ^
                 v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^
                 v17 ^ v18 ^ v19 ^ v20 ^ v21 ^ v22 ^
                 v23 ^ v24 ^ v25 ^ v26 ^ v27 ^ v28 ^
                 v29 ^ v30 ^ t1 ^ t2 ^ t3 ^ t4 ^
                 t5 ^ t6 ^ t7 ^ t8 ^ t9 ^ t10 ^
                 mem1 ^ mem2 ^ mem3 ^ mem4;
    
    return result;
}

/* Another pressure function with different patterns */
__attribute__((noinline, noipa))
int secondary_pressure(int base) {
    /* Different type mixing */
    unsigned int u1 = (unsigned int)base;
    unsigned short u2 = (unsigned short)(base >> 8);
    unsigned char u3 = (unsigned char)(base >> 16);
    signed int s1 = -base;
    signed short s2 = -(short)base;
    
    /* Complex bit manipulation */
    u1 = (u1 << 5) | (u1 >> 27);
    u2 = (u2 ^ 0x55AA) + u3;
    u3 = ((u3 & 0xF0) >> 4) | ((u3 & 0x0F) << 4);
    s1 = (s1 * 7) / 3;
    s2 = s2 | 0x8000;
    
    /* Force spills with large expression */
    int r = (u1 * u2) + (u3 << 8) - (s1 / 2) + (s2 & 0x7FFF);
    r = r ^ (r << 16) ^ (r >> 16);
    
    /* Assembly to prevent optimization */
    asm volatile("" : "+r"(r) : : "cc");
    
    return r;
}

int main(int argc, char *argv[]) {
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_limit < 1) loop_limit = 1;
    if (loop_limit > 10000) loop_limit = 10000;
    
    int total_result = 0;
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* Use volatile globals and argc to create unknown values */
        int input1 = global_seed1 + argc + i;
        int input2 = global_seed2 - argc + i * 2;
        int input3 = global_seed3 ^ argc ^ i;
        int input4 = global_seed4 | argc | i;
        
        /* Call the high-pressure function */
        int result1 = create_reload_pressure(input1, input2, input3, input4);
        
        /* Call secondary function */
        int result2 = secondary_pressure(result1 + i);
        
        /* Mix results to prevent dead code elimination */
        total_result ^= result1;
        total_result += result2;
        total_result = (total_result << 1) | (total_result >> 31);
    }
    
    /* Use the result to prevent optimization */
    printf("Result: 0x%08x\n", total_result & 0xFFFFFFFF);
    
    return (total_result & 0xFF);
}
