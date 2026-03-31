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
    long v7 = (long)v1 * v2;
    int v8 = v3 << 2;
    int v9 = v4 >> 1;
    int v10 = v5 * 3;
    int v11 = v6 + 100;
    int v12 = (int)(v7 & 0xFFFFFFFF);
    int v13 = v8 | v9;
    int v14 = v10 ^ v11;
    int v15 = v12 + v13;
    int v16 = v14 * v15;
    int v17 = v1 & v2;
    int v18 = v3 | v4;
    int v19 = v5 ^ v6;
    int v20 = (int)v7 + v8;
    int v21 = v9 * v10;
    int v22 = v11 | v12;
    int v23 = v13 ^ v14;
    int v24 = v15 & v16;
    int v25 = v17 + v18;
    int v26 = v19 * v20;
    int v27 = v21 | v22;
    int v28 = v23 ^ v24;
    int v29 = v25 + v26;
    int v30 = v27 * v28;
    
    /* Complex expressions with multiple uses of variables */
    /* These create many intermediate values and addressing modes */
    v1 = (v2 & v3) | (v4 << (v5 & 3));
    v6 = (v7 * v8) - (v9 >> (v10 % 4));
    v11 = ((v12 + v13) * (v14 - v15)) & global_mask;
    v16 = (v17 | v18) ^ (v19 & v20);
    v21 = ((v22 << 2) + (v23 >> 1)) * v24;
    v25 = (v26 & v27) | (v28 ^ v29);
    
    /* Inline assembly to force specific register constraints */
    /* This creates artificial register pressure and conflicts */
    asm volatile("" : "+r"(v1), "+r"(v2) : : "cc", "memory");
    asm volatile("" : "+m"(v3), "+r"(v4) : : "cc");
    asm volatile("" : "+r"(v5), "+m"(v6) : : "cc", "memory");
    
    /* More complex expressions with volatile memory access */
    v7 = (v8 * v9) + (global_seed & 0xFF);
    v10 = (v11 << 3) | (v12 >> 2);
    v13 = (v14 & v15) ^ (v16 | v17);
    v18 = (v19 + v20) - (v21 * v22);
    v23 = ((v24 % 17) + v25) & (v26 ^ 0xDEADBEEF);
    v27 = (v28 << (v29 & 3)) | (v30 >> (v1 & 3));
    
    /* Additional inline assembly with mixed constraints */
    asm volatile("addl %1, %0" : "+r"(v2) : "rm"(v3) : "cc");
    asm volatile("orl %1, %0" : "+r"(v4) : "rm"(v5) : "cc");
    
    /* Create addressing mode pressure with array-like calculations */
    int temp1 = v1 + v2 + v3 + v4 + v5;
    int temp2 = v6 * v7 * v8 * v9 * v10;
    int temp3 = v11 | v12 | v13 | v14 | v15;
    int temp4 = v16 ^ v17 ^ v18 ^ v19 ^ v20;
    int temp5 = v21 & v22 & v23 & v24 & v25;
    int temp6 = v26 + v27 + v28 + v29 + v30;
    
    /* Final complex expression that uses all variables */
    int result = (temp1 & temp2) | (temp3 ^ temp4) + (temp5 * temp6);
    result = result + (v1 << 2) - (v2 >> 1) + (v3 & v4) | (v5 ^ v6);
    result = result * (v7 % 31) + (v8 & 0xFF) - (v9 | 0xAA) ^ (v10 << 3);
    result = result + (v11 >> 2) * (v12 & 0xF) - (v13 | v14) + (v15 ^ v16);
    result = result & (v17 << 4) | (v18 >> 2) ^ (v19 & v20) + (v21 * v22);
    result = result - (v23 % 7) + (v24 & v25) | (v26 ^ v27) << (v28 & 3);
    result = result + (v29 * 3) - (v30 & 0x7F);
    
    /* Force memory spill/reload with volatile store/load */
    volatile int spill_slot;
    spill_slot = result;
    result = spill_slot + global_seed;
    
    return result;
}

int main(int argc, char **argv) {
    int i;
    int total = 0;
    
    /* Use argc to control loop iterations (prevents optimization) */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    if (iterations > 1000) iterations = 1000;
    
    /* Initialize volatile inputs */
    volatile int input1 = argc;
    volatile int input2 = argc * 2;
    volatile int input3 = argc | 0x55;
    volatile int input4 = argc ^ 0xAA;
    
    printf("Starting reload pressure test with %d iterations...\n", iterations);
    
    /* Loop to ensure function is called multiple times */
    for (i = 0; i < iterations; i++) {
        /* Modify inputs slightly each iteration */
        input1 = input1 + i;
        input2 = input2 ^ i;
        input3 = input3 | (i & 0xFF);
        input4 = input4 - i;
        
        /* Call the high-pressure function */
        int result = create_reload_pressure(input1, input2, input3, input4);
        
        /* Accumulate results (prevents dead code elimination) */
        total = total ^ result;
        
        /* Modify global to prevent optimization */
        global_seed = global_seed + result;
    }
    
    /* Use the result to prevent optimization */
    printf("Result: 0x%08x\n", total & 0xFFFFFFFF);
    
    return (total & 0xFF);
}
