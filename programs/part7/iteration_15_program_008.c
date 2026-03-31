/* test_reload.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 42;
volatile int global_mask = 0xFFFF;

/* Prevent inlining and IPA optimizations */
__attribute__((noinline, noipa, optimize("O0")))
int create_reload_pressure(int input1, int input2, int input3) {
    /* Declare many local variables to exhaust registers */
    int v1 = input1 + 1;
    int v2 = input2 - 1;
    int v3 = v1 * v2;
    short v4 = (short)(input3 & 0xFF);
    char v5 = (char)(input1 ^ input2);
    long v6 = (long)v1 * (long)v2;
    int v7 = v3 >> 2;
    int v8 = v1 | v2;
    int v9 = v3 & v7;
    short v10 = (short)(v4 + v5);
    char v11 = (char)(v5 * 3);
    long v12 = v6 + (long)v7;
    int v13 = v8 ^ v9;
    int v14 = v7 + v8;
    int v15 = v9 - v13;
    short v16 = (short)(v10 | (short)v11);
    char v17 = (char)(v11 + v5);
    long v18 = v12 * 2;
    int v19 = v13 & v14;
    int v20 = v15 | v19;
    int v21 = v14 ^ v20;
    short v22 = (short)(v16 + v10);
    char v23 = (char)(v17 ^ v11);
    long v24 = v18 + v12;
    int v25 = v19 * v20;
    int v26 = v21 - v25;
    int v27 = v20 + v26;
    short v28 = (short)(v22 & v16);
    char v29 = (char)(v23 | v17);
    long v30 = v24 ^ v18;
    
    /* Complex expressions with many live values */
    v1 = (v2 & v3) | (v4 << (v5 & 3));
    v6 = (long)v7 * (long)v8 - (long)v9;
    v10 = (short)((v11 * v12) + (v13 & v14));
    v15 = ((v16 << 2) | (v17 & 0xF)) ^ v18;
    v19 = v20 * v21 - v22 + v23;
    v24 = (v25 & v26) | (v27 << 3);
    v28 = (short)(v29 * v30 + v1 - v2);
    
    /* More operations keeping values live */
    v3 = v4 * v5 + v6 / 2;
    v7 = (v8 << v9) | (v10 & v11);
    v12 = v13 ^ v14 ^ v15;
    v16 = (short)(v17 + v18 - v19);
    v20 = v21 * v22 / (v23 + 1);
    v25 = v26 | v27 & v28;
    v29 = (char)(v30 % 31 + v1);
    
    /* Inline assembly to force specific register usage */
    asm volatile ("" : "+r" (v1), "+r" (v2), "+r" (v3) : : "cc", "memory");
    asm volatile ("" : "+r" (v4), "+r" (v5), "+r" (v6) : : "cc", "memory");
    
    /* More complex expressions with addressing modes */
    int* ptr1 = &v7;
    int* ptr2 = &v8;
    int* ptr3 = &v9;
    
    *ptr1 = (*ptr2 + *ptr3) * v10;
    *ptr2 = (*ptr1 - *ptr3) | v11;
    *ptr3 = (*ptr1 ^ *ptr2) & v12;
    
    /* Force memory accesses */
    volatile int mem1 = v13;
    volatile int mem2 = v14;
    volatile int mem3 = v15;
    
    v16 = (short)(mem1 + mem2 - mem3);
    v17 = (char)(mem1 ^ mem2 ^ mem3);
    
    /* More arithmetic creating intermediate values */
    int t1 = v18 + v19;
    int t2 = v20 * v21;
    int t3 = v22 | v23;
    int t4 = v24 ^ v25;
    int t5 = v26 - v27;
    int t6 = v28 & v29;
    
    v30 = t1 * t2 - t3 + t4 ^ t5 | t6;
    
    /* Even more operations */
    for (int i = 0; i < 3; i++) {
        v1 = v1 + v2 + v3;
        v4 = (short)(v4 + v5 + v6);
        v7 = v7 * v8 - v9;
        v10 = (short)(v10 | v11 | v12);
    }
    
    /* Final aggregation to prevent dead code elimination */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ (int)v6 ^ v7 ^ v8 ^ v9 ^ v10 ^ 
                 v11 ^ (int)v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ (int)v18 ^ 
                 v19 ^ v20 ^ v21 ^ v22 ^ v23 ^ (int)v24 ^ v25 ^ v26 ^ 
                 v27 ^ v28 ^ v29 ^ (int)v30;
    
    return result & 0xFF; /* Keep result small */
}

int main(int argc, char *argv[]) {
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_limit < 1) loop_limit = 1;
    if (loop_limit > 1000) loop_limit = 1000;
    
    int total = 0;
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* Use volatile inputs to prevent constant propagation */
        volatile int input1 = global_seed + i;
        volatile int input2 = global_mask - i;
        volatile int input3 = argc * i;
        
        total += create_reload_pressure(input1, input2, input3);
        
        /* Modify globals to create side effects */
        global_seed ^= total;
        global_mask += i;
    }
    
    printf("Result: %d\n", total & 0xFF);
    return total & 0xFF;
}
