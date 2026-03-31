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
    int v1 = input1;
    int v2 = input2 + 1;
    int v3 = input3 * 2;
    short v4 = (short)(input1 & 0xFF);
    short v5 = (short)(input2 & 0xFF);
    char v6 = (char)(input3 & 0x7F);
    long v7 = (long)input1 * input2;
    long v8 = (long)input2 * input3;
    int v9 = v1 ^ v2;
    int v10 = v3 | v4;
    int v11 = v5 << 2;
    int v12 = v6 * 3;
    long v13 = v7 + v8;
    int v14 = v9 - v10;
    short v15 = (short)(v11 + v12);
    char v16 = (char)(v13 & 0xFF);
    int v17 = v14 * v15;
    int v18 = v16 + v1;
    long v19 = v13 >> 4;
    int v20 = v17 & v18;
    short v21 = (short)(v19 & 0xFFFF);
    char v22 = (char)(v20 & 0xFF);
    int v23 = v21 * v22;
    long v24 = v19 + v23;
    int v25 = v24 & 0xFFFFFF;
    short v26 = (short)v25;
    char v27 = (char)(v25 >> 8);
    int v28 = v26 | v27;
    long v29 = v24 ^ v28;
    int v30 = v29 & 0xFFFF;
    
    /* Complex expressions requiring multiple registers */
    v1 = (v2 & v3) | (v4 << (v5 & 3));
    v6 = (v7 * v8 - v9) & 0xFF;
    v10 = ((v11 << 2) + (v12 >> 1)) ^ v13;
    v14 = (v15 * v16) + (v17 / (v18 + 1));
    v19 = (v20 | v21) & (v22 ^ v23);
    v24 = (v25 << 4) - (v26 * v27);
    v28 = (v29 & v30) | (v1 ^ v2);
    
    /* More operations to keep values live */
    v3 = v4 + v5 * v6 - v7 / (v8 + 1);
    v9 = (v10 << 1) | (v11 >> 2);
    v12 = v13 ^ v14 & v15 | v16;
    v17 = v18 * v19 - v20 + v21;
    v22 = (v23 & 0xF0) | (v24 & 0x0F);
    v25 = v26 + v27 - v28 * v29;
    v30 = (v1 << 3) ^ (v2 >> 1) & v3;
    
    /* Use inline assembly to create register constraints */
    asm volatile ("" : "+r" (v1), "+r" (v2), "+r" (v3) : : "cc", "memory");
    asm volatile ("" : "+m" (v4), "+r" (v5) : : "cc");
    
    /* Complex addressing mode simulation */
    int* ptr1 = &v1;
    int* ptr2 = &v2;
    short* ptr3 = (short*)&v3;
    char* ptr4 = (char*)&v4;
    
    /* Force memory accesses */
    *ptr1 = (*ptr2 + *ptr3) | (*ptr4 << 2);
    v5 = *(short*)((char*)ptr1 + 2);
    v6 = *(char*)((int*)ptr2 + 1);
    
    /* More arithmetic with all variables live */
    v7 = v8 + v9 * v10 - v11 / (v12 | 1);
    v13 = v14 ^ v15 & v16 | v17;
    v18 = v19 * v20 - v21 + v22;
    v23 = (v24 & 0xAA) | (v25 & 0x55);
    v26 = v27 + v28 - v29 * v30;
    
    /* Final computation using all variables */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6;
    result += v7 ^ v8 ^ v9 ^ v10 ^ v11 ^ v12;
    result += v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18;
    result += v19 ^ v20 ^ v21 ^ v22 ^ v23 ^ v24;
    result += v25 ^ v26 ^ v27 ^ v28 ^ v29 ^ v30;
    
    return result & global_mask;
}

int main(int argc, char** argv) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 100;
    
    int total = 0;
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < iterations; i++) {
        /* Use volatile inputs to prevent constant propagation */
        volatile int input1 = global_seed + i;
        volatile int input2 = argc * 3 + i;
        volatile int input3 = (int)((long)argv & 0xFFFF) + i;
        
        total ^= create_reload_pressure(input1, input2, input3);
        
        /* Modify global to prevent loop optimization */
        global_seed ^= total;
    }
    
    printf("Result: 0x%08x\n", total & 0xFFFFFFFF);
    return total & 0xFF;
}
