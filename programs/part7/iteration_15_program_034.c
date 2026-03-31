/* test_reload.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 42;
volatile int global_mask = 0xFFFFFFFF;

/* Non-inlineable function that creates massive register pressure */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3, int input4) {
    /* Declare many local variables with different types to exhaust registers */
    int v1 = input1 + 1;
    int v2 = input2 * 2;
    int v3 = input3 | 0xFF;
    int v4 = input4 ^ 0xAA;
    short v5 = (short)(v1 + v2);
    short v6 = (short)(v3 - v4);
    char v7 = (char)(v1 & 0x7F);
    char v8 = (char)(v2 | 0x80);
    long v9 = (long)v1 * (long)v2;
    long v10 = (long)v3 + (long)v4;
    int v11 = v5 * v6;
    int v12 = v7 + v8;
    int v13 = (int)(v9 >> 3);
    int v14 = (int)(v10 << 2);
    int v15 = v11 ^ v12;
    int v16 = v13 & v14;
    int v17 = v15 | v16;
    int v18 = v1 + v2 + v3;
    int v19 = v4 * v5 * v6;
    int v20 = v7 - v8 - v9;
    int v21 = v10 ^ v11 ^ v12;
    int v22 = v13 | v14 | v15;
    int v23 = v16 & v17 & v18;
    int v24 = v19 + v20 + v21;
    int v25 = v22 * v23 * v24;
    int v26 = v1 << v2;
    int v27 = v3 >> v4;
    int v28 = v5 & v6;
    int v29 = v7 | v8;
    int v30 = v9 ^ v10;
    
    /* Complex expressions that create many intermediate values */
    v1 = (v2 & v3) | (v4 << v5);
    v6 = (v7 * v8) - (v9 >> 2);
    v11 = (v12 + v13) ^ (v14 - v15);
    v16 = (v17 | v18) & (v19 ^ v20);
    v21 = (v22 << v23) + (v24 >> v25);
    v26 = (v27 * v28) - (v29 / (v30 ? v30 : 1));
    
    /* More complex expressions with multiple uses */
    v1 = v1 + ((v2 * v3) + (v4 << v5) - v6);
    v7 = v7 ^ ((v8 & v9) | (v10 >> v11) + v12);
    v13 = v13 - ((v14 + v15) ^ (v16 << v17) & v18);
    v19 = v19 | ((v20 - v21) + (v22 >> v23) * v24);
    v25 = v25 & ((v26 | v27) ^ (v28 + v29) - v30);
    
    /* Even more complex expressions to ensure values stay live */
    int t1 = (v1 * v2) + (v3 << v4) - (v5 & v6);
    int t2 = (v7 | v8) ^ (v9 >> v10) + (v11 - v12);
    int t3 = (v13 & v14) | (v15 << v16) - (v17 * v18);
    int t4 = (v19 ^ v20) + (v21 >> v22) & (v23 | v24);
    int t5 = (v25 - v26) << (v27 & v28) | (v29 ^ v30);
    
    /* Use inline assembly to create register constraints */
    asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3) : : "cc", "memory");
    asm volatile("" : "+r"(v4), "+r"(v5), "+r"(v6) : : "cc", "memory");
    asm volatile("" : "+r"(v7), "+r"(v8), "+r"(v9) : : "cc", "memory");
    
    /* Complex addressing mode simulation */
    int* ptr1 = &v1;
    int* ptr2 = &v2;
    int* ptr3 = &v3;
    
    /* Force memory accesses that compete for address registers */
    v1 = *ptr1 + *ptr2;
    v2 = *ptr3 - v1;
    v3 = *ptr1 * *ptr3;
    
    /* More pointer arithmetic */
    ptr1 = &v4;
    ptr2 = &v5;
    ptr3 = &v6;
    
    v4 = *ptr1 | *ptr2;
    v5 = *ptr3 ^ v4;
    v6 = *ptr1 & *ptr3;
    
    /* Final complex computation using all variables */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10;
    result += v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18 ^ v19 ^ v20;
    result += v21 ^ v22 ^ v23 ^ v24 ^ v25 ^ v26 ^ v27 ^ v28 ^ v29 ^ v30;
    result += t1 ^ t2 ^ t3 ^ t4 ^ t5;
    
    /* Use global volatile to prevent optimization */
    result &= global_mask;
    
    return result;
}

/* Another high-pressure function with different patterns */
__attribute__((noinline, noipa))
int secondary_pressure(int base) {
    /* Different variable naming and patterns */
    int a = base + 100;
    int b = base * 2;
    int c = base | 0x55;
    int d = base ^ 0xAA;
    int e = a + b;
    int f = c - d;
    int g = a * b;
    int h = c | d;
    int i = e ^ f;
    int j = g & h;
    int k = i << 2;
    int l = j >> 1;
    int m = k | l;
    int n = a ^ b ^ c;
    int o = d + e + f;
    int p = g * h * i;
    int q = j - k - l;
    int r = m | n | o;
    int s = p & q & r;
    int t = a << b;
    int u = c >> d;
    
    /* Nested complex expressions */
    a = (b + ((c * d) << (e & 0xF))) - (f | (g ^ h));
    i = (j & ((k | l) >> (m + 1))) + (n ^ (p * q));
    r = (s - ((t & u) << (a % 16))) | (b ^ (c + d));
    
    /* Force spilling with large expression */
    int complex = (a * b) + (c << d) - (e & f) | (g ^ h) + (i >> j) * (k | l) - (m & n) ^ (o << p) + (q * r) - (s | t) ^ u;
    
    /* Mix with global volatile */
    complex ^= global_seed;
    
    return complex;
}

int main(int argc, char** argv) {
    /* Use volatile loop counter to prevent optimization */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 1;
    if (iterations > 1000) iterations = 1000;
    
    int total_result = 0;
    
    /* Loop to ensure the function is called multiple times */
    for (volatile int i = 0; i < iterations; i++) {
        /* Create varying inputs to prevent constant folding */
        int input1 = argc + i;
        int input2 = global_seed * i;
        int input3 = (argc << 3) | i;
        int input4 = global_mask ^ i;
        
        /* Call the high-pressure functions */
        int result1 = create_reload_pressure(input1, input2, input3, input4);
        int result2 = secondary_pressure(input1 + input2);
        
        /* Combine results in a non-trivial way */
        total_result ^= result1;
        total_result += result2;
        total_result = (total_result << 3) | (total_result >> 29); /* rotate */
        
        /* Use global volatile to prevent dead code elimination */
        global_seed ^= result1;
        global_mask &= result2;
    }
    
    /* Print result to prevent entire program from being optimized away */
    printf("Final result: 0x%08X\n", total_result & 0xFFFFFFFF);
    
    return (total_result & 0xFF);
}
