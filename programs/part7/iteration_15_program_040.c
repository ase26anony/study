/* reload_coverage.c
 * Program designed to trigger GCC's reload pass initialization code
 * Specifically targets lines 1381-1399 in reload.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 42;
volatile int global_mask = 0xFFFF;

/* Non-inlineable function with extreme register pressure */
__attribute__((noinline, noipa, optimize("O0")))
int create_reload_pressure(int input1, int input2, int input3) {
    /* Declare many local variables to exhaust registers */
    int v1 = input1 + 1;
    int v2 = input2 * 2;
    int v3 = input3 ^ 0xAA;
    short v4 = (short)(input1 & 0xFF);
    char v5 = (char)(input2 + 3);
    long v6 = (long)input1 * input2;
    int v7 = v1 + v2;
    int v8 = v3 - v1;
    short v9 = (short)(v4 * 2);
    char v10 = (char)(v5 ^ 0x55);
    long v11 = v6 + (long)v1;
    int v12 = v7 * v8;
    int v13 = v2 | v3;
    short v14 = (short)(v9 + v4);
    char v15 = (char)(v10 & 0xF);
    long v16 = v11 - v6;
    int v17 = v12 ^ v13;
    int v18 = v1 << 2;
    int v19 = v2 >> 1;
    short v20 = (short)(v14 * 3);
    char v21 = (char)(v15 | 0x10);
    long v22 = v16 * 2;
    int v23 = v17 + v18;
    int v24 = v19 - v7;
    short v25 = (short)(v20 & 0x7F);
    char v26 = (char)(v21 ^ v15);
    long v27 = v22 + v11;
    int v28 = v23 * v24;
    int v29 = v8 + v13;
    short v30 = (short)(v25 | 0x80);
    
    /* Complex expressions with multiple uses of variables */
    /* These create many intermediate values and addressing modes */
    int expr1 = (v1 * v2) + (v3 << v4) - (v5 * v6);
    int expr2 = (v7 & v8) | (v9 << v10) ^ (v11 >> 3);
    int expr3 = (v12 + v13) * (v14 - v15) / (v16 & 0xFF);
    int expr4 = (v17 ^ v18) | (v19 & v20) + (v21 << 2);
    int expr5 = (v22 % 100) * (v23 + v24) - (v25 ^ v26);
    int expr6 = (v27 >> 4) + (v28 & 0xF0F0) * (v29 | 0x0F0F);
    
    /* More complex expressions with mixed types */
    long expr7 = (long)expr1 * (long)expr2 + (long)expr3;
    int expr8 = (expr4 << 3) | (expr5 >> 2) ^ expr6;
    short expr9 = (short)((expr1 & 0xFFFF) + (expr2 & 0xFFFF));
    char expr10 = (char)((expr3 ^ expr4) & 0xFF);
    
    /* Inline assembly to force specific register constraints */
    /* This creates artificial register pressure and conflicts */
    asm volatile("" 
                 : "+r"(v1), "+r"(v2), "+r"(v3)
                 : 
                 : "cc", "memory");
    
    asm volatile("mov %0, %0" : "+r"(v4));
    asm volatile("add %1, %0" : "+r"(v5) : "r"(expr10));
    
    /* Volatile memory accesses to force address register usage */
    volatile int mem1 = expr1;
    volatile int mem2 = expr2;
    volatile short mem3 = expr9;
    volatile char mem4 = expr10;
    
    /* More operations using volatile memory */
    v1 = v1 + mem1;
    v2 = v2 * mem2;
    v3 = v3 ^ mem1;
    v4 = (short)(v4 + mem3);
    v5 = (char)(v5 | mem4);
    
    /* Additional complex expressions with spill potential */
    int complex1 = ((v1 * v2) + (v3 << (v4 & 0x7))) - 
                   ((v5 * v6) / (v7 + 1)) + 
                   ((v8 & v9) | (v10 << 2));
    
    int complex2 = (v11 % 17) * (v12 ^ v13) + 
                   (v14 << (v15 & 0x3)) - 
                   (v16 >> 4);
    
    /* Force many values to be live simultaneously */
    int sum1 = v1 + v2 + v3 + v4 + v5;
    int sum2 = v6 + v7 + v8 + v9 + v10;
    int sum3 = v11 + v12 + v13 + v14 + v15;
    int sum4 = v16 + v17 + v18 + v19 + v20;
    int sum5 = v21 + v22 + v23 + v24 + v25;
    int sum6 = v26 + v27 + v28 + v29 + v30;
    
    /* Final aggregation to prevent dead code elimination */
    int result = expr1 ^ expr2 ^ expr3 ^ expr4 ^ expr5 ^ expr6;
    result += complex1 + complex2;
    result += sum1 + sum2 + sum3 + sum4 + sum5 + sum6;
    result += mem1 + mem2 + mem3 + mem4;
    
    /* More inline assembly to prevent optimization */
    asm volatile("" : : "r"(result) : "memory");
    
    return result & 0x7FFFFFFF; /* Ensure positive result */
}

/* Another non-inlineable function with different patterns */
__attribute__((noinline, noipa))
int secondary_pressure(int base, int modifier) {
    /* Different variable types and patterns */
    unsigned int u1 = (unsigned int)base;
    unsigned short u2 = (unsigned short)modifier;
    unsigned char u3 = (unsigned char)(base ^ modifier);
    
    unsigned long ul1 = (unsigned long)u1 * u2;
    unsigned long ul2 = ul1 + (unsigned long)u3;
    
    /* Bit manipulation creating many intermediate values */
    u1 = (u1 << 3) | (u1 >> 29); /* rotate */
    u2 = (u2 ^ 0xAAAA) & 0x5555;
    u3 = ((u3 + 1) * 3) & 0xFF;
    
    /* More complex expressions */
    unsigned int expr = (u1 * u2) + (u3 << 2) - (u1 % u2);
    expr = expr ^ (ul1 & 0xFFFFFFFF) ^ (ul2 >> 16);
    
    /* Force spill/reload with addressing modes */
    volatile unsigned int vmem = expr;
    expr = expr + vmem * 2;
    
    return (int)expr;
}

int main(int argc, char *argv[]) {
    /* Use command line arguments for variability */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    if (iterations > 1000) iterations = 1000;
    
    volatile int seed1 = argc + global_seed;
    volatile int seed2 = argc * 3;
    volatile int seed3 = global_mask & argc;
    
    int total_result = 0;
    
    /* Loop to ensure the function is called multiple times */
    for (volatile int i = 0; i < iterations; i++) {
        /* Mix different function calls */
        int result1 = create_reload_pressure(seed1 + i, seed2 - i, seed3 ^ i);
        int result2 = secondary_pressure(result1, i);
        
        /* Complex accumulation to prevent optimization */
        total_result ^= result1;
        total_result += result2;
        total_result = (total_result << 1) | (total_result >> 31);
        
        /* Modify seeds to create different patterns */
        seed1 = (seed1 * 1103515245 + 12345) & 0x7FFFFFFF;
        seed2 = seed2 ^ (i * 0x5A5A5A5A);
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Final result: %d (0x%08x)\n", 
           total_result & 0xFF, 
           total_result & 0xFFFFFFFF);
    
    return (total_result & 0xFF) == 0 ? 0 : 1;
}
