/* test_reload.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent constant propagation */
volatile int global_seed1 = 12345;
volatile int global_seed2 = 67890;
volatile int global_seed3 = 13579;

/* Non-inlineable function with massive register pressure */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3) {
    /* Declare many local variables with different types to exhaust registers */
    int v1 = input1 + 1;
    int v2 = input2 - 1;
    int v3 = input3 * 2;
    short v4 = (short)(input1 & 0xFFFF);
    short v5 = (short)(input2 >> 8);
    char v6 = (char)(input3 & 0xFF);
    char v7 = (char)((input1 >> 8) & 0xFF);
    long v8 = (long)input1 * input2;
    long v9 = (long)input2 * input3;
    long v10 = (long)input3 * input1;
    
    int v11 = v1 + v2;
    int v12 = v2 - v3;
    int v13 = v3 * v1;
    int v14 = v1 | v2;
    int v15 = v2 & v3;
    int v16 = v3 ^ v1;
    int v17 = v11 << 2;
    int v18 = v12 >> 1;
    int v19 = v13 + v14;
    int v20 = v15 - v16;
    
    short v21 = (short)(v4 + v5);
    short v22 = (short)(v5 - v6);
    short v23 = (short)(v6 * v7);
    short v24 = (short)(v7 | v4);
    short v25 = (short)(v5 & v6);
    
    char v26 = (char)(v6 + v7);
    char v27 = (char)(v7 - v6);
    char v28 = (char)(v6 * v7);
    char v29 = (char)(v7 | v6);
    char v30 = (char)(v6 & v7);
    
    /* Complex expressions requiring multiple registers */
    v1 = (v11 & v12) | (v13 << v14);
    v2 = (v15 * v16) - (v17 >> v18);
    v3 = (v19 ^ v20) + (v21 * v22);
    
    /* Force address calculations with volatile memory accesses */
    volatile int mem1 = v1;
    volatile int mem2 = v2;
    volatile int mem3 = v3;
    
    /* More complex expressions with mixed types */
    v8 = (v8 * v9) + (v10 << 3);
    v9 = (v9 - v10) | (v8 >> 2);
    v10 = (v10 ^ v8) & (v9 << 1);
    
    /* Inline assembly to create register constraints */
    asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3) : : "cc", "memory");
    
    /* More operations keeping many values live */
    v11 = v1 + v2 + v3;
    v12 = v2 - v3 + v1;
    v13 = v3 * v1 - v2;
    v14 = (v1 | v2) & v3;
    v15 = (v2 & v3) | v1;
    v16 = (v3 ^ v1) + v2;
    v17 = v11 << v12;
    v18 = v12 >> v13;
    v19 = v13 + v14 + v15;
    v20 = v15 - v16 + v17;
    
    /* Use all variables in final computation to prevent elimination */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7;
    result += (int)(v8 ^ v9 ^ v10);
    result += v11 + v12 + v13 + v14 + v15;
    result += v16 - v17 + v18 - v19 + v20;
    result += v21 * v22 - v23 + v24 | v25;
    result += v26 + v27 - v28 ^ v29 & v30;
    
    return result & 0x7FFFFFFF; /* Keep result positive */
}

/* Another high-pressure function with different patterns */
__attribute__((noinline, noipa))
int secondary_pressure(int base, int modifier) {
    /* Create many intermediate values */
    int a = base + modifier;
    int b = base - modifier;
    int c = base * modifier;
    int d = base | modifier;
    int e = base & modifier;
    int f = base ^ modifier;
    
    int g = (a << 3) | (b >> 2);
    int h = (c * d) + (e - f);
    int i = (a & b) | (c ^ d);
    int j = (e << f) + (g >> h);
    int k = (i * j) - (a + b);
    int l = (c | d) & (e ^ f);
    int m = (g + h) * (i - j);
    int n = (k & l) | (m >> 2);
    int o = (a * b) + (c * d) - (e * f);
    int p = (g ^ h) + (i ^ j) - (k ^ l);
    
    /* Force spills with inline assembly constraints */
    asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d), 
                       "+r"(e), "+r"(f), "+r"(g), "+r"(h) : : "cc");
    
    /* Complex addressing mode simulation */
    volatile int* ptr = &a;
    int val1 = *ptr + *(ptr + 1) + *(ptr + 2);
    int val2 = *(ptr + 3) - *(ptr + 4) + *(ptr + 5);
    
    /* More operations */
    int q = (val1 << 1) | (val2 >> 1);
    int r = (m * n) + (o - p);
    int s = (q & r) | (val1 ^ val2);
    
    return a + b + c + d + e + f + g + h + i + j + 
           k + l + m + n + o + p + q + r + s;
}

int main(int argc, char *argv[]) {
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_limit < 10) loop_limit = 10;
    if (loop_limit > 1000) loop_limit = 1000;
    
    int total_result = 0;
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* Use volatile globals and argc to create varying inputs */
        int input1 = global_seed1 + argc + i;
        int input2 = global_seed2 - argc + i * 2;
        int input3 = global_seed3 ^ argc ^ i;
        
        /* Call high-pressure functions */
        int result1 = create_reload_pressure(input1, input2, input3);
        int result2 = secondary_pressure(input1, input3);
        
        total_result ^= result1;
        total_result += result2;
        
        /* Modify globals slightly to change patterns */
        global_seed1 = (global_seed1 * 1103515245 + 12345) & 0x7FFFFFFF;
        global_seed2 = (global_seed2 * 1664525 + 1013904223) & 0x7FFFFFFF;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Final result: %d\n", total_result & 0xFF);
    
    return (total_result & 0xFF) == 0 ? 0 : 1;
}
