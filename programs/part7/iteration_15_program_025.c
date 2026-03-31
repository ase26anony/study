/* test_reload.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_seed = 42;
volatile int global_mask = 0xFFFFFFFF;

/* Non-inlineable function with extreme register pressure */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3, int input4) {
    /* Declare many local variables with different types to exhaust registers */
    int v1 = input1 + 1;
    int v2 = input2 * 2;
    int v3 = input3 | 0xFF;
    int v4 = input4 ^ 0xAA;
    short v5 = (short)(v1 + v2);
    short v6 = (short)(v3 - v4);
    char v7 = (char)(v1 & 0x7F);
    char v8 = (char)(v2 | 0x3F);
    long v9 = (long)v1 * (long)v2;
    long v10 = (long)v3 + (long)v4;
    int v11 = v1 * v3;
    int v12 = v2 ^ v4;
    int v13 = v5 + v6;
    int v14 = v7 * v8;
    int v15 = v9 & 0xFFFF;
    int v16 = v10 >> 4;
    int v17 = v11 | v12;
    int v18 = v13 & v14;
    int v19 = v15 ^ v16;
    int v20 = v17 + v18;
    int v21 = v19 * v20;
    int v22 = v1 << 2;
    int v23 = v2 >> 1;
    int v24 = v3 & v4;
    int v25 = v5 | v6;
    int v26 = v7 ^ v8;
    int v27 = v9 % 256;
    int v28 = v10 & 0xFF;
    int v29 = v11 + v12 + v13;
    int v30 = v14 - v15 - v16;
    
    /* Complex expressions with many live values simultaneously */
    v1 = (v2 & v3) | (v4 << (v5 & 3));
    v6 = (v7 * v8) - (v9 >> 4);
    v10 = (v11 ^ v12) + (v13 & v14);
    v15 = (v16 | v17) * (v18 ^ v19);
    v20 = (v21 + v22) - (v23 * v24);
    v25 = (v26 << 2) | (v27 >> 1);
    v28 = (v29 & v30) ^ (v1 | v2);
    
    /* More complex expressions creating intermediate values */
    int t1 = v3 * v4 + v5;
    int t2 = v6 ^ v7 | v8;
    int t3 = v9 << (v10 & 3);
    int t4 = v11 >> (v12 % 4);
    int t5 = v13 + v14 * v15;
    int t6 = v16 & v17 ^ v18;
    int t7 = v19 | v20 + v21;
    int t8 = v22 * v23 - v24;
    int t9 = v25 ^ v26 & v27;
    int t10 = v28 << 1 | v29;
    
    /* Use inline assembly to create register constraints */
    asm volatile("" : "+r"(v1), "+r"(v2) : : "cc", "memory");
    asm volatile("" : "+r"(v3), "+r"(v4) : : "cc", "memory");
    asm volatile("" : "+m"(v5), "+m"(v6) : : "cc");
    
    /* More operations keeping values live */
    v1 = v1 + t1 * t2;
    v2 = v2 ^ t3 | t4;
    v3 = v3 - t5 + t6;
    v4 = v4 & t7 * t8;
    v5 = v5 | t9 ^ t10;
    v6 = v6 + t1 - t2;
    v7 = v7 * t3 & t4;
    v8 = v8 ^ t5 | t6;
    v9 = v9 - t7 + t8;
    v10 = v10 & t9 * t10;
    
    /* Even more complex nested expressions */
    v11 = ((v12 & v13) << (v14 % 4)) | ((v15 ^ v16) >> 1);
    v12 = (v17 * v18) + (v19 - v20) * (v21 | v22);
    v13 = ((v23 ^ v24) & (v25 | v26)) + ((v27 & v28) << 2);
    v14 = (v29 * v30) - (t1 & t2) + (t3 | t4) - (t5 ^ t6);
    v15 = ((t7 << 3) & (t8 >> 2)) | ((t9 + t10) ^ (v1 & v2));
    
    /* Use volatile memory accesses to force address register pressure */
    volatile int mem1 = global_seed;
    volatile int mem2 = global_mask;
    
    v16 = v16 + mem1 * mem2;
    v17 = v17 ^ mem1 | mem2;
    v18 = v18 - mem1 + mem2;
    v19 = v19 & mem1 * mem2;
    v20 = v20 | mem1 ^ mem2;
    
    /* Final aggregation to prevent dead code elimination */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10;
    result ^= v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18 ^ v19 ^ v20;
    result ^= v21 ^ v22 ^ v23 ^ v24 ^ v25 ^ v26 ^ v27 ^ v28 ^ v29 ^ v30;
    result ^= t1 ^ t2 ^ t3 ^ t4 ^ t5 ^ t6 ^ t7 ^ t8 ^ t9 ^ t10;
    
    return result & 0xFF;  /* Return small value to avoid overflow issues */
}

int main(int argc, char *argv[]) {
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    volatile int seed = argc;
    int total = 0;
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* Use different inputs each iteration to prevent constant propagation */
        int input1 = seed + i;
        int input2 = global_seed * i;
        int input3 = global_mask & i;
        int input4 = ~i;
        
        total += create_reload_pressure(input1, input2, input3, input4);
        
        /* Modify globals slightly to create varying conditions */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7FFFFFFF;
        global_mask = global_mask ^ (i << 3);
    }
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", total & 0xFF);
    
    return (total & 0xFF) == 0 ? 0 : 1;
}
