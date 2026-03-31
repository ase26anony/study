/* reload_test.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_seed1 = 12345;
volatile int global_seed2 = 67890;
volatile int global_seed3 = 54321;

/* Prevent inlining and IPA optimizations */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3) {
    /* Declare many local variables to exhaust registers */
    int v1 = input1 + 1;
    int v2 = input2 * 2;
    int v3 = input3 - 3;
    short v4 = (short)(input1 & 0xFFFF);
    short v5 = (short)(input2 | 0x1234);
    char v6 = (char)(input3 ^ 0x55);
    long v7 = (long)input1 * input2;
    long v8 = (long)input2 + input3;
    int v9 = v1 * v2;
    int v10 = v2 + v3;
    int v11 = v3 - v1;
    int v12 = v4 * v5;
    int v13 = v6 + v1;
    int v14 = v7 & 0xFFFFFFFF;
    int v15 = v8 | 0xAAAAAAAA;
    int v16 = v9 ^ v10;
    int v17 = v11 << 2;
    int v18 = v12 >> 1;
    int v19 = v13 + v14;
    int v20 = v15 - v16;
    int v21 = v17 * v18;
    int v22 = v19 & v20;
    int v23 = v21 | v22;
    int v24 = v23 ^ v1;
    int v25 = v24 + v2;
    int v26 = v25 - v3;
    int v27 = v26 * v4;
    int v28 = v27 / (v5 + 1);
    int v29 = v28 << v6;
    int v30 = v29 >> 1;
    
    /* Complex expressions requiring multiple registers */
    int expr1 = (v1 * v2) + (v3 << v4) - (v5 & v6);
    int expr2 = (v7 % 100) * (v8 / 3) + (v9 ^ v10);
    int expr3 = (v11 | v12) & (v13 ^ v14) - (v15 << 1);
    
    /* Force register usage with inline asm */
    asm volatile("" : "+r"(v1), "+r"(v2) : : "cc", "memory");
    asm volatile("" : "+r"(v3), "+r"(v4) : : "cc", "memory");
    
    /* More complex operations keeping values live */
    v16 = (expr1 & expr2) | (expr3 << 2);
    v17 = (v16 * v1) - (v2 / (v3 + 1));
    v18 = (v4 << v5) | (v6 & v7);
    v19 = (v8 ^ v9) + (v10 * v11);
    v20 = (v12 - v13) & (v14 | v15);
    
    /* Additional variables to increase pressure */
    int v31 = v17 + v18;
    int v32 = v19 - v20;
    int v33 = v21 * v22;
    int v34 = v23 / (v24 + 1);
    int v35 = v25 ^ v26;
    int v36 = v27 & v28;
    int v37 = v29 | v30;
    int v38 = v31 << 2;
    int v39 = v32 >> 1;
    int v40 = v33 + v34;
    
    /* Memory operations to force address register usage */
    volatile int mem1 = v35;
    volatile int mem2 = v36;
    volatile int mem3 = v37;
    
    /* Use all variables in final computation */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ 
                 v7 ^ v8 ^ v9 ^ v10 ^ v11 ^ v12 ^ 
                 v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18 ^ 
                 v19 ^ v20 ^ v21 ^ v22 ^ v23 ^ v24 ^ 
                 v25 ^ v26 ^ v27 ^ v28 ^ v29 ^ v30 ^
                 v31 ^ v32 ^ v33 ^ v34 ^ v35 ^ v36 ^
                 v37 ^ v38 ^ v39 ^ v40 ^
                 expr1 ^ expr2 ^ expr3 ^
                 mem1 ^ mem2 ^ mem3;
    
    return result & 0xFF; /* Return small value */
}

int main(int argc, char **argv) {
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_limit < 1) loop_limit = 1;
    if (loop_limit > 1000) loop_limit = 1000;
    
    int total = 0;
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* Use volatile inputs to prevent constant propagation */
        int input1 = global_seed1 + i;
        int input2 = global_seed2 - i;
        int input3 = global_seed3 * (i + 1);
        
        total += create_reload_pressure(input1, input2, input3);
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %d\n", total & 0xFF);
    return total & 0xFF;
}
