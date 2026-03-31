/* test_reload.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_seed1 = 12345;
volatile int global_seed2 = 67890;
volatile int global_seed3 = 24680;
volatile int global_seed4 = 13579;

/* Prevent inlining and IPA optimizations */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3, int input4) {
    /* Declare many local variables of different types to exhaust registers */
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
    int v11 = v1 + v2;
    int v12 = v3 - v4;
    int v13 = v5 * v6;
    int v14 = v7 | v8;
    int v15 = (int)(v9 & 0xFFFFFFFF);
    int v16 = (int)(v10 >> 16);
    int v17 = v11 ^ v12;
    int v18 = v13 & v14;
    int v19 = v15 | v16;
    int v20 = v17 + v18;
    int v21 = v19 - v20;
    int v22 = v1 * v3;
    int v23 = v2 / v4;
    int v24 = v5 << 3;
    int v25 = v6 >> 2;
    int v26 = v7 & 0x7F;
    int v27 = v8 | 0x80;
    int v28 = (int)v9 + (int)v10;
    int v29 = v11 * v12;
    int v30 = v13 ^ v14;
    
    /* Complex expressions requiring multiple registers */
    v1 = (v2 & v3) | (v4 << (v5 & 3));
    v6 = (v7 * v8) - (v9 & 0xFFF);
    v10 = (v11 << (v12 % 8)) + (v13 >> (v14 & 7));
    v15 = (v16 | v17) ^ (v18 & v19);
    v20 = (v21 * v22) / (v23 ? v23 : 1);
    v24 = (v25 + v26) - (v27 * v28);
    v29 = (v30 ^ v1) & (v2 | v3);
    
    /* More intermediate calculations */
    int t1 = v1 + v2 + v3 + v4;
    int t2 = v5 * v6 * v7 * v8;
    int t3 = v9 ^ v10 ^ v11 ^ v12;
    int t4 = v13 | v14 | v15 | v16;
    int t5 = v17 & v18 & v19 & v20;
    int t6 = v21 - v22 - v23 - v24;
    int t7 = v25 << 2 | v26 >> 2;
    int t8 = v27 & 0xAA | v28 & 0x55;
    int t9 = v29 * 3 + v30 * 7;
    int t10 = t1 ^ t2 ^ t3;
    
    /* Use inline assembly to force specific register usage */
    asm volatile ("" : "+r" (v1), "+r" (v2), "+r" (v3) : : "cc", "memory");
    asm volatile ("" : "+r" (v4), "+r" (v5), "+r" (v6) : : "cc", "memory");
    
    /* More complex operations with addressing modes */
    int* ptr1 = &v1;
    int* ptr2 = &v2;
    int* ptr3 = &v3;
    
    *ptr1 = *ptr2 + *ptr3;
    *ptr2 = *ptr1 - *ptr3;
    *ptr3 = *ptr1 * *ptr2;
    
    /* Force memory accesses */
    volatile int mem1 = v4;
    volatile int mem2 = v5;
    volatile int mem3 = v6;
    
    v4 = mem1 + mem2;
    v5 = mem2 - mem3;
    v6 = mem3 * mem1;
    
    /* Even more variables to increase pressure */
    int u1 = t1 + t2;
    int u2 = t3 - t4;
    int u3 = t5 & t6;
    int u4 = t7 | t8;
    int u5 = t9 ^ t10;
    int u6 = u1 * u2;
    int u7 = u3 / (u4 ? u4 : 1);
    int u8 = u5 << (u6 & 3);
    int u9 = u7 >> (u8 & 7);
    int u10 = u9 + u1 - u2;
    
    /* Final aggregation to prevent dead code elimination */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6;
    result ^= v7 ^ v8 ^ v9 ^ v10 ^ v11 ^ v12;
    result ^= v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18;
    result ^= v19 ^ v20 ^ v21 ^ v22 ^ v23 ^ v24;
    result ^= v25 ^ v26 ^ v27 ^ v28 ^ v29 ^ v30;
    result ^= t1 ^ t2 ^ t3 ^ t4 ^ t5 ^ t6;
    result ^= t7 ^ t8 ^ t9 ^ t10;
    result ^= u1 ^ u2 ^ u3 ^ u4 ^ u5;
    result ^= u6 ^ u7 ^ u8 ^ u9 ^ u10;
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_limit < 1) loop_limit = 1;
    if (loop_limit > 1000) loop_limit = 1000;
    
    int total = 0;
    
    /* Loop to ensure function is executed multiple times */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* Use volatile globals and argc to prevent constant propagation */
        int input1 = global_seed1 + argc + i;
        int input2 = global_seed2 - argc + i * 2;
        int input3 = global_seed3 ^ argc ^ i;
        int input4 = global_seed4 | argc | i;
        
        total += create_reload_pressure(input1, input2, input3, input4);
        
        /* Modify globals slightly to change inputs */
        global_seed1 ^= total;
        global_seed2 += i;
        global_seed3 -= total & 0xFF;
        global_seed4 |= i & 0xFF;
    }
    
    /* Use result to prevent optimization */
    printf("Result: 0x%08x\n", total & 0xFFFFFFFF);
    
    return (total & 0xFF);
}
