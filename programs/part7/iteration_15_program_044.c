/* test_reload.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_seed = 42;
volatile int global_mask = 0xFFFFFFFF;

/* Force register pressure function to not be inlined or optimized */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3) {
    /* Declare many local variables with different types to exhaust registers */
    int v1 = input1 + 1;
    int v2 = input2 * 2;
    int v3 = input3 & 0xFF;
    short v4 = (short)(input1 >> 8);
    char v5 = (char)(input2 + 3);
    long v6 = (long)input1 * input2;
    int v7 = v1 ^ v2;
    int v8 = v3 | v4;
    int v9 = v5 * 2;
    long v10 = v6 + 100;
    int v11 = v7 - v8;
    short v12 = (short)(v9 + v4);
    char v13 = (char)(v5 ^ 0x55);
    int v14 = v11 * v12;
    long v15 = v10 - v6;
    int v16 = v14 & 0xFFFF;
    int v17 = v13 + v16;
    short v18 = (short)(v12 | 0xAA);
    char v19 = (char)(v13 & 0xF0);
    int v20 = v17 << 2;
    long v21 = v15 * 3;
    int v22 = v18 * v19;
    int v23 = v20 ^ v21;
    short v24 = (short)(v22 + v18);
    char v25 = (char)(v19 | 0x0F);
    int v26 = v23 - v24;
    long v27 = v21 / 2;
    int v28 = v25 * v26;
    int v29 = v27 ^ v28;
    
    /* Complex expressions with multiple operands to force reloads */
    v1 = (v2 & v3) | (v4 << (v5 & 3));
    v6 = (v7 * v8) - (v9 + v10);
    v11 = ((v12 | v13) ^ (v14 & v15)) + v16;
    v17 = (v18 * v19) >> (v20 % 4);
    v21 = (v22 + v23) * (v24 - v25);
    v26 = (v27 | v28) & (v29 ^ v1);
    
    /* More complex expressions with mixed operations */
    v2 = v3 + ((v4 * v5) >> 2);
    v7 = (v8 & v9) | (v10 << 1);
    v12 = v13 ^ ((v14 + v15) & 0xFF);
    v17 = (v18 * v19) - (v20 | v21);
    v22 = ((v23 << 2) + v24) ^ v25;
    v27 = (v28 & v29) | (v1 >> 3);
    
    /* Even more expressions to keep values live */
    v3 = v4 * v5 + v6 - v7;
    v8 = (v9 & v10) | (v11 ^ v12);
    v13 = v14 << (v15 & 3);
    v16 = v17 + v18 * v19;
    v20 = (v21 | v22) & (v23 ^ v24);
    v25 = v26 * v27 - v28;
    v29 = v1 ^ v2 ^ v3 ^ v4;
    
    /* Use inline assembly to force specific register constraints */
    asm volatile ("" : "+r" (v1), "+r" (v2), "+r" (v3) : : "cc", "memory");
    asm volatile ("" : "+r" (v4), "+r" (v5), "+r" (v6) : : "cc", "memory");
    asm volatile ("" : "+r" (v7), "+r" (v8), "+r" (v9) : : "cc", "memory");
    
    /* Complex addressing mode simulation */
    int* ptr1 = &v10;
    int* ptr2 = &v11;
    int* ptr3 = &v12;
    
    /* Force memory accesses that compete for address registers */
    v13 = *ptr1 + *ptr2;
    v14 = *ptr3 - *ptr1;
    v15 = *ptr2 * *ptr3;
    
    /* More pointer arithmetic */
    short* sptr1 = (short*)&v16;
    char* cptr1 = (char*)&v17;
    v18 = *sptr1 + *(cptr1 + 1);
    
    /* Additional complex expressions */
    v19 = (v20 * v21) + (v22 << (v23 & 3)) - v24;
    v25 = ((v26 & v27) | (v28 ^ v29)) + v1;
    v2 = v3 * v4 - v5 / (v6 + 1);
    v7 = (v8 << 2) | (v9 >> 1);
    v10 = v11 ^ v12 ^ v13;
    
    /* Final aggregation to prevent dead code elimination */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10;
    result ^= v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18 ^ v19;
    result ^= v20 ^ v21 ^ v22 ^ v23 ^ v24 ^ v25 ^ v26 ^ v27 ^ v28 ^ v29;
    
    return result & 0xFF;  /* Return meaningful result */
}

int main(int argc, char** argv) {
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    volatile int seed = argc;
    int total = 0;
    
    /* Call the high-pressure function multiple times */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* Use volatile globals and arguments to create unknown values */
        int input1 = seed + i;
        int input2 = global_seed * i;
        int input3 = global_mask & (i * 3);
        
        /* Call the register pressure function */
        int result = create_reload_pressure(input1, input2, input3);
        total += result;
        
        /* Modify globals to prevent optimization */
        global_seed ^= result;
        global_mask |= i;
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return total & 1;  /* Return non-constant value */
}
