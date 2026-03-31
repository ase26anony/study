/* reload_coverage.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_seed = 42;
volatile int global_mask = 0xFFFFFFFF;

/* Non-inlineable function with extreme register pressure */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3) {
    /* Declare many local variables with different types */
    int v1 = input1 + 1;
    int v2 = input2 - 1;
    int v3 = v1 * v2;
    short v4 = (short)(input3 & 0xFFFF);
    char v5 = (char)(input1 ^ input2);
    long v6 = (long)v1 * (long)v2;
    int v7 = v3 ^ v2;
    int v8 = v1 | v2;
    short v9 = (short)(v4 + v5);
    char v10 = (char)(v5 * 3);
    int v11 = v7 << 2;
    int v12 = v8 >> 1;
    long v13 = v6 + v11;
    int v14 = v12 & v11;
    short v15 = v9 | v4;
    char v16 = v10 ^ v5;
    int v17 = v14 * v11;
    int v18 = v17 - v12;
    long v19 = v13 | v6;
    int v20 = v18 ^ v17;
    short v21 = (short)(v15 + v9);
    char v22 = (char)(v16 * 2);
    int v23 = v20 << 3;
    int v24 = v23 >> 2;
    long v25 = v19 & v13;
    int v26 = v24 | v23;
    short v27 = v21 ^ v15;
    char v28 = v22 + v16;
    int v29 = v26 - v24;
    int v30 = v29 * v26;
    
    /* Complex expressions creating many intermediate values */
    int expr1 = (v1 & v2) | (v3 << (v4 & 3));
    int expr2 = (v5 * v6) + (v7 >> (v8 & 7));
    int expr3 = (v9 | v10) ^ (v11 & v12);
    int expr4 = (v13 + v14) - (v15 * v16);
    int expr5 = (v17 << 2) | (v18 >> 1);
    int expr6 = (v19 & v20) + (v21 ^ v22);
    int expr7 = (v23 * v24) - (v25 | v26);
    int expr8 = (v27 << 3) & (v28 >> 2);
    int expr9 = (v29 ^ v30) | (expr1 & expr2);
    int expr10 = (expr3 + expr4) - (expr5 * expr6);
    
    /* Inline assembly to force specific register usage */
    asm volatile("" : "+r"(v1), "+r"(v2) : : "cc", "memory");
    asm volatile("" : "+r"(v3), "+r"(v4) : : "cc", "memory");
    asm volatile("" : "+r"(expr1), "+r"(expr2) : : "cc", "memory");
    
    /* More complex operations mixing all variables */
    v1 = (v2 & v3) | (v4 << v5);
    v6 = (v7 * v8) - (v9 ^ v10);
    v11 = (v12 | v13) & (v14 >> v15);
    v16 = (v17 + v18) * (v19 - v20);
    v21 = (v22 & v23) | (v24 ^ v25);
    v26 = (v27 << 2) + (v28 >> 1);
    v29 = (v30 * expr1) - (expr2 & expr3);
    
    /* Even more operations to keep values live */
    expr1 = expr1 + expr4 - expr7;
    expr2 = expr2 * expr5 / (expr8 + 1);
    expr3 = expr3 | expr6 & expr9;
    expr4 = expr4 ^ expr10 ^ expr1;
    expr5 = (expr5 << 2) | (expr6 >> 3);
    expr6 = expr7 * expr8 + expr9;
    expr7 = expr10 & expr1 | expr2;
    expr8 = expr3 - expr4 * expr5;
    expr9 = expr6 ^ expr7 ^ expr8;
    expr10 = expr9 + expr10 - expr1;
    
    /* Use volatile memory access to force address register pressure */
    volatile int mem1 = global_seed;
    volatile int mem2 = global_mask;
    
    v1 = v1 + mem1;
    v2 = v2 & mem2;
    v3 = v3 | mem1;
    v4 = v4 ^ (mem2 & 0xFFFF);
    
    /* Final aggregation to prevent dead code elimination */
    int result = v1 ^ v2 ^ v3 ^ (v4 << 16);
    result ^= v5 ^ v6 ^ v7 ^ v8;
    result ^= v9 ^ v10 ^ v11 ^ v12;
    result ^= v13 ^ v14 ^ v15 ^ v16;
    result ^= v17 ^ v18 ^ v19 ^ v20;
    result ^= v21 ^ v22 ^ v23 ^ v24;
    result ^= v25 ^ v26 ^ v27 ^ v28;
    result ^= v29 ^ v30 ^ expr1 ^ expr2;
    result ^= expr3 ^ expr4 ^ expr5 ^ expr6;
    result ^= expr7 ^ expr8 ^ expr9 ^ expr10;
    
    return result & 0xFF; /* Return small value */
}

int main(int argc, char **argv) {
    int total = 0;
    volatile int iterations;
    
    /* Use argc to determine iterations, preventing optimization */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 10;
    } else {
        iterations = 5;
    }
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < iterations; i++) {
        /* Use different inputs each iteration */
        int input1 = global_seed + i;
        int input2 = global_mask - i;
        int input3 = argc * i;
        
        /* Call the high-pressure function */
        int result = create_reload_pressure(input1, input2, input3);
        total += result;
        
        /* Modify globals slightly */
        global_seed ^= result;
        global_mask &= ~result;
    }
    
    /* Print result to prevent optimization */
    printf("Total checksum: %d\n", total & 0xFF);
    
    return total & 0xFF;
}
