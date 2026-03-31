/* reload_test.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_seed = 42;
volatile int global_mask = 0xFFFF;

/* Non-inlineable function with extreme register pressure */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3, int input4) {
    /* Declare many local variables with different types */
    int v1 = input1 + 1;
    int v2 = input2 * 2;
    int v3 = input3 & 0xFF;
    int v4 = input4 | 0x80;
    short s1 = (short)(v1 + v2);
    short s2 = (short)(v3 - v4);
    char c1 = (char)(v1 & 0xFF);
    char c2 = (char)(v2 & 0xFF);
    long l1 = (long)v1 * v2;
    long l2 = (long)v3 * v4;
    int v5 = v1 ^ v2;
    int v6 = v3 | v4;
    int v7 = v5 << 2;
    int v8 = v6 >> 1;
    int v9 = v7 + v8;
    int v10 = v9 * 3;
    int v11 = v10 & global_mask;
    int v12 = v11 | 0x7F;
    int v13 = v12 ^ v1;
    int v14 = v13 + v2;
    int v15 = v14 - v3;
    int v16 = v15 * v4;
    int v17 = v16 >> 3;
    int v18 = v17 << 1;
    int v19 = v18 & 0x3FF;
    int v20 = v19 | 0x1FF;
    int v21 = v20 ^ v5;
    int v22 = v21 + v6;
    int v23 = v22 - v7;
    int v24 = v23 * v8;
    int v25 = v24 / 2;
    int v26 = v25 % 17;
    int v27 = v26 | v9;
    int v28 = v27 & v10;
    int v29 = v28 ^ v11;
    int v30 = v29 + v12;
    
    /* Complex expressions with multiple operands */
    v1 = (v2 & v3) | (v4 << (c1 & 3)) - v5;
    v6 = v7 * v8 - v9 + (v10 >> (c2 & 3));
    v11 = (v12 ^ v13) + (v14 & v15) * (v16 | v17);
    v18 = ((v19 << 2) + (v20 >> 1)) ^ (v21 & v22);
    v23 = (v24 * v25) - (v26 << 3) + (v27 & 0xFFF);
    v28 = (v29 | v30) ^ (v1 << 1) - (v2 & 0x7F);
    
    /* More intermediate calculations */
    int t1 = v1 + v2 + v3 + v4;
    int t2 = v5 * v6 - v7 + v8;
    int t3 = v9 & v10 | v11 ^ v12;
    int t4 = v13 << 2 | v14 >> 1;
    int t5 = v15 + v16 - v17 * v18;
    int t6 = v19 & v20 ^ v21 | v22;
    int t7 = v23 + v24 - v25 * v26;
    int t8 = v27 & v28 | v29 ^ v30;
    
    /* Use inline assembly to create register constraints */
    asm volatile("" : "+r"(v1), "+r"(v2) : : "cc", "memory");
    asm volatile("" : "+r"(v3), "+r"(v4) : : "cc", "memory");
    asm volatile("" : "+m"(v5), "+m"(v6) : : "cc");
    
    /* More complex operations mixing all variables */
    l1 = (long)v1 * v2 + (long)v3 * v4 - (long)v5 * v6;
    l2 = (long)v7 * v8 + (long)v9 * v10 - (long)v11 * v12;
    
    s1 = (short)((v13 & 0xFFFF) + (v14 & 0xFFFF));
    s2 = (short)((v15 & 0xFFFF) - (v16 & 0xFFFF));
    
    c1 = (char)((v17 ^ v18) & 0xFF);
    c2 = (char)((v19 | v20) & 0xFF);
    
    /* Even more operations to keep values live */
    v1 = v2 + v3 * v4 - v5 / (v6 + 1);
    v7 = v8 & v9 | v10 ^ v11 << (v12 & 3);
    v13 = v14 - v15 + v16 * v17 >> (v18 & 3);
    v19 = v20 | v21 & v22 ^ v23 + v24;
    v25 = v26 * v27 - v28 + v29 & v30;
    
    /* Final aggregation to prevent dead code elimination */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10;
    result ^= v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18 ^ v19 ^ v20;
    result ^= v21 ^ v22 ^ v23 ^ v24 ^ v25 ^ v26 ^ v27 ^ v28 ^ v29 ^ v30;
    result ^= (int)l1 ^ (int)l2 ^ (int)s1 ^ (int)s2 ^ (int)c1 ^ (int)c2;
    result ^= t1 ^ t2 ^ t3 ^ t4 ^ t5 ^ t6 ^ t7 ^ t8;
    
    return result & 0xFF; /* Return small value */
}

int main(int argc, char **argv) {
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    volatile int seed = argc;
    int total = 0;
    
    /* Create register pressure in main as well */
    volatile int m1 = seed + 1;
    volatile int m2 = seed * 2;
    volatile int m3 = seed & 0xFF;
    volatile int m4 = seed | 0x80;
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* Mix global and local values */
        int input1 = global_seed + i + m1;
        int input2 = global_seed * i + m2;
        int input3 = global_seed ^ i ^ m3;
        int input4 = global_seed & i & m4;
        
        /* Call the high-pressure function */
        total += create_reload_pressure(input1, input2, input3, input4);
        
        /* Modify globals to prevent optimization */
        global_seed ^= total;
        global_mask = (global_mask + 1) & 0xFFFF;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return total & 1;
}
