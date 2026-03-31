/* reload_coverage.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_seed = 42;
volatile int global_mask = 0xFFFF;

/* Force register pressure function to not be optimized away */
__attribute__((noinline, noipa, noicf))
int create_reload_pressure(int input1, int input2, int input3, int input4) {
    /* Declare many local variables with different types to exhaust registers */
    int v1 = input1 + 1;
    int v2 = input2 * 2;
    int v3 = input3 | 0xAA;
    int v4 = input4 ^ 0x55;
    short v5 = (short)(v1 + v2);
    short v6 = (short)(v3 - v4);
    char v7 = (char)(v1 & 0xFF);
    char v8 = (char)(v2 | 0x7F);
    long v9 = (long)v1 * (long)v2;
    long v10 = (long)v3 + (long)v4;
    int v11 = v1 * v3;
    int v12 = v2 / (v4 ? v4 : 1);
    int v13 = v5 + v6;
    int v14 = v7 * v8;
    int v15 = (int)(v9 & 0xFFFFFFFF);
    int v16 = (int)(v10 >> 16);
    int v17 = v11 ^ v12;
    int v18 = v13 | v14;
    int v19 = v15 + v16;
    int v20 = v17 - v18;
    int v21 = v19 * v20;
    int v22 = v21 & global_mask;
    int v23 = v22 << 3;
    int v24 = v23 >> 1;
    int v25 = v24 | 0x1234;
    int v26 = v25 ^ 0xABCD;
    int v27 = v26 + global_seed;
    int v28 = v27 - input1;
    int v29 = v28 * input2;
    int v30 = v29 / (input3 ? input3 : 1);
    
    /* Complex expressions with multiple operands to force reloads */
    v1 = (v2 & v3) | (v4 << (v5 & 3)) - v6;
    v2 = v7 * v8 + (v9 >> (v10 & 7)) ^ v11;
    v3 = (v12 | v13) & (v14 ^ v15) + (v16 << 2);
    v4 = v17 - v18 * (v19 + v20) / (v21 ? v21 : 1);
    v5 = (v22 | v23) ^ (v24 & v25) + (v26 >> 3);
    
    /* More complex expressions creating many intermediate values */
    int t1 = v1 * v2 + v3 * v4;
    int t2 = v5 * v6 - v7 * v8;
    int t3 = v9 & v10 | v11 ^ v12;
    int t4 = v13 << (v14 & 3) >> (v15 & 1);
    int t5 = v16 + v17 - v18 + v19;
    int t6 = v20 * v21 / (v22 ? v22 : 1);
    int t7 = v23 | v24 & v25 ^ v26;
    int t8 = v27 << 1 + v28 >> 2;
    int t9 = v29 & 0xFF + v30 & 0xFF00;
    
    /* Use inline assembly to create specific register constraints */
    asm volatile("" : "+r"(v1), "+r"(v2) : : "cc", "memory");
    asm volatile("" : "+m"(v3), "+r"(v4) : : "cc");
    asm volatile("" : "+r"(v5), "+r"(v6), "+r"(v7) : : "cc");
    
    /* Even more operations to keep values live */
    v1 = v1 + t1 - t2;
    v2 = v2 * t3 / (t4 ? t4 : 1);
    v3 = v3 | t5 & t6;
    v4 = v4 ^ t7 + t8;
    v5 = v5 - t9 * v1;
    
    /* Create addressing mode pressure with array accesses */
    int arr[8];
    for (int i = 0; i < 8; i++) {
        arr[i] = (v1 + i) * (v2 - i) + (v3 << i);
    }
    
    /* Use array elements in complex expressions */
    int sum = 0;
    sum += arr[0] * arr[1];
    sum -= arr[2] | arr[3];
    sum ^= arr[4] & arr[5];
    sum += arr[6] << arr[7];
    
    /* Final complex expression using all variables */
    int result = (v1 ^ v2) + (v3 | v4) - (v5 & v6) * 
                 (v7 + v8) / (v9 ? (v9 & 0xFF) : 1) +
                 (v10 << 2) - (v11 >> 3) +
                 (v12 * v13) ^ (v14 + v15) |
                 (v16 & v17) + (v18 - v19) *
                 (v20 / (v21 ? v21 : 1)) +
                 (v22 | v23) - (v24 ^ v25) +
                 (v26 << (v27 & 3)) >> (v28 & 7) +
                 (v29 * v30) + sum + t1 - t2 + t3 * t4;
    
    /* Force memory barrier */
    asm volatile("" ::: "memory");
    
    return result;
}

/* Another high-pressure function with different patterns */
__attribute__((noinline, noipa))
int secondary_pressure(int base) {
    /* Different variable naming and patterns */
    int a = base * 3;
    int b = base + 7;
    int c = base ^ 0x99;
    int d = base | 0x66;
    int e = a & b;
    int f = c | d;
    int g = e ^ f;
    int h = g << 2;
    int i = h >> 1;
    int j = i * a;
    int k = j + b;
    int l = k - c;
    int m = l & d;
    int n = m | e;
    int o = n ^ f;
    int p = o * g;
    int q = p / (h ? h : 1);
    int r = q + i;
    int s = r - j;
    int t = s & k;
    int u = t | l;
    int v = u ^ m;
    int w = v * n;
    int x = w + o;
    int y = x - p;
    int z = y & q;
    
    /* Complex nested expressions */
    int r1 = (a * b) + (c << d) - (e & f) | (g ^ h);
    int r2 = (i * j) - (k << l) + (m & n) ^ (o | p);
    int r3 = (q * r) | (s << t) - (u & v) + (w ^ x);
    int r4 = (y * z) ^ (a << b) | (c & d) - (e | f);
    
    /* Mix with inline assembly constraints */
    asm volatile("" : "+r"(a), "+r"(b), "+r"(c) : : "cc");
    asm volatile("" : "+m"(d), "+r"(e) : : "cc");
    
    return r1 + r2 - r3 * r4 + a - b + c - d + e;
}

int main(int argc, char *argv[]) {
    /* Use volatile to prevent loop optimization */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 1;
    if (iterations > 1000) iterations = 1000;
    
    volatile int seed1 = argc * 12345;
    volatile int seed2 = argc + 67890;
    volatile int seed3 = argc ^ 0x13579;
    volatile int seed4 = argc | 0x24680;
    
    int total = 0;
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < iterations; i++) {
        /* Call both high-pressure functions */
        int result1 = create_reload_pressure(seed1 + i, seed2 - i, 
                                           seed3 ^ i, seed4 | i);
        int result2 = secondary_pressure(seed1 ^ result1);
        
        /* Mix results to prevent dead code elimination */
        total += result1 - result2;
        total ^= (result1 * result2) & 0xFF;
        
        /* Modify seeds to create varying inputs */
        seed1 = (seed1 * 1103515245 + 12345) & 0x7FFFFFFF;
        seed2 = (seed2 * 1664525 + 1013904223) & 0x7FFFFFFF;
    }
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", total & 0xFF);
    
    return (total & 0xFF) == 0 ? 0 : 1;
}
