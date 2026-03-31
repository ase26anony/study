/* test_reload.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_seed = 42;
volatile int global_mask = 0xFFFFFFFF;

/* NOINLINE function to create register pressure */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3, int input4) {
    /* Declare many local variables to exhaust registers */
    int v1 = input1 + 1;
    int v2 = input2 * 2;
    int v3 = input3 | 0xAA;
    int v4 = input4 ^ 0x55;
    short v5 = (short)(v1 + v2);
    char v6 = (char)(v3 & 0xFF);
    long v7 = (long)v4 * 3;
    int v8 = v1 ^ v2;
    int v9 = v3 | v4;
    short v10 = (short)(v5 + v6);
    char v11 = (char)(v6 ^ 0x7F);
    long v12 = v7 + 100;
    int v13 = v8 << 2;
    int v14 = v9 >> 1;
    int v15 = v10 * v11;
    int v16 = v12 & 0xFFFF;
    int v17 = v13 | v14;
    int v18 = v15 ^ v16;
    int v19 = v17 + v18;
    int v20 = v19 * 3;
    int v21 = v20 / 2;
    int v22 = v21 | 0x1234;
    int v23 = v22 ^ 0xABCD;
    int v24 = v23 << 3;
    int v25 = v24 >> 1;
    int v26 = v25 + 0x1000;
    int v27 = v26 & 0xFF00;
    int v28 = v27 | 0x00FF;
    int v29 = v28 ^ 0x5555;
    int v30 = v29 * 7;
    
    /* Complex expressions with multiple uses of variables */
    /* These create addressing mode constraints */
    v1 = (v2 & v3) | (v4 << (v5 & 7));
    v6 = (v7 * v8) - (v9 ^ v10);
    v11 = ((v12 + v13) >> (v14 & 3)) & v15;
    v16 = (v17 | v18) ^ (v19 & v20);
    v21 = (v22 * v23) + (v24 - v25);
    v26 = (v27 << 2) | (v28 >> 3);
    v29 = (v30 ^ v1) + (v2 & v3);
    
    /* More complex expressions with mixed types */
    v4 = (int)((long)v5 * (long)v6 + (long)v7);
    v8 = (v9 << 4) | (v10 >> 4);
    v11 = (v12 & v13) ^ (v14 | v15);
    v16 = (v17 + v18) * (v19 - v20);
    v21 = (v22 ^ v23) & (v24 | v25);
    v26 = (v27 * 3) + (v28 * 5) - (v29 * 7);
    v30 = (v1 << 1) | (v2 << 2) | (v3 << 3);
    
    /* Inline assembly to create register constraints */
    /* These force specific register allocation decisions */
    asm volatile("" : "+r"(v1), "+r"(v2) : : "cc", "memory");
    asm volatile("" : "+m"(v3), "+r"(v4) : : "cc");
    asm volatile("" : "+r"(v5), "+r"(v6), "+r"(v7) : : "cc");
    
    /* Even more operations to keep values live */
    v8 = v1 + v2 + v3 + v4;
    v9 = v5 * v6 * v7;
    v10 = (v8 & v9) | (v1 ^ v2);
    v11 = (v3 << v4) + (v5 >> v6);
    v12 = v7 * v8 - v9 * v10;
    v13 = v11 | v12 | v1 | v2;
    v14 = v3 & v4 & v5 & v6;
    v15 = (v7 ^ v8) + (v9 ^ v10);
    v16 = v11 * v12 / (v13 + 1);
    v17 = v14 << (v15 & 15);
    v18 = v16 >> (v17 & 7);
    v19 = v18 + v19 + v20;
    v20 = v21 * v22 - v23 * v24;
    v21 = v25 | v26 | v27 | v28;
    v22 = v29 & v30 & v1 & v2;
    v23 = (v3 ^ v4) * (v5 ^ v6);
    v24 = v7 + v8 + v9 + v10;
    v25 = v11 * v12 * v13;
    v26 = (v14 & v15) | (v16 ^ v17);
    v27 = (v18 << v19) - (v20 >> v21);
    v28 = v22 * v23 + v24 * v25;
    v29 = v26 | v27 | v28;
    v30 = v29 ^ v1 ^ v2 ^ v3;
    
    /* Use volatile memory access to force spills */
    volatile int mem1 = v4;
    volatile int mem2 = v8;
    volatile int mem3 = v12;
    
    /* More operations using the memory values */
    v1 = v1 + mem1;
    v2 = v2 * mem2;
    v3 = v3 ^ mem3;
    
    /* Final aggregation to prevent dead code elimination */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10;
    result ^= v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18 ^ v19 ^ v20;
    result ^= v21 ^ v22 ^ v23 ^ v24 ^ v25 ^ v26 ^ v27 ^ v28 ^ v29 ^ v30;
    result ^= mem1 ^ mem2 ^ mem3;
    
    return result & 0xFF;  /* Return small value */
}

int main(int argc, char *argv[]) {
    /* Use command line arguments to create variable inputs */
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_limit < 1) loop_limit = 1;
    if (loop_limit > 1000) loop_limit = 1000;
    
    int total = 0;
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* Create varying inputs to prevent constant propagation */
        int input1 = global_seed + i;
        int input2 = global_mask - i;
        int input3 = argc * i;
        int input4 = (int)((long)argv[0] & 0xFF) + i;
        
        /* Call the high-pressure function */
        int result = create_reload_pressure(input1, input2, input3, input4);
        total += result;
        
        /* Modify globals to prevent optimization */
        global_seed ^= result;
        global_mask |= i;
    }
    
    /* Print result to create side effect */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
