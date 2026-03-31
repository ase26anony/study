/* test_reload.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_seed = 42;
volatile int global_mask = 0xFFFF;

/* Non-inlineable function with extreme register pressure */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3) {
    /* Declare many local variables with different types */
    int v1 = input1 + 1;
    int v2 = input2 * 2;
    int v3 = input3 & 0xFF;
    short v4 = (short)(input1 >> 4);
    char v5 = (char)(input2 + 3);
    long v6 = (long)input1 * input2;
    int v7 = v1 ^ v2;
    int v8 = v3 | v4;
    int v9 = v5 * 2;
    long v10 = v6 + v1;
    int v11 = v7 - v8;
    short v12 = (short)(v9 + v4);
    char v13 = (char)(v5 ^ v3);
    int v14 = v11 * v12;
    long v15 = v10 - v6;
    int v16 = v14 & 0x7FFF;
    int v17 = v13 + v16;
    int v18 = v17 << 3;
    int v19 = v18 | v16;
    int v20 = v19 ^ v14;
    long v21 = v15 + v20;
    int v22 = v21 & 0xFFFFFFFF;
    int v23 = v22 * v17;
    int v24 = v23 / (v16 + 1);
    int v25 = v24 ^ v18;
    int v26 = v25 | v19;
    int v27 = v26 & v20;
    long v28 = v21 + v27;
    int v29 = v28 & 0xFFFF;
    int v30 = v29 * v22;
    
    /* Complex expressions with many live values */
    v1 = (v2 & v3) | (v4 << (v5 & 7));
    v6 = (v7 * v8) - (v9 + v10);
    v11 = ((v12 + v13) * v14) / (v15 & 0xFF + 1);
    v16 = (v17 ^ v18) | (v19 & v20);
    v21 = (v22 * v23) + (v24 << (v25 & 3));
    v26 = (v27 - v28) * (v29 + v30);
    
    /* More operations keeping values live */
    v2 = v3 + v4 - v5 * v6;
    v7 = v8 ^ v9 | v10 & v11;
    v12 = v13 << 2 + v14 >> 1;
    v15 = v16 * v17 - v18 / (v19 + 1);
    v20 = v21 | v22 ^ v23 & v24;
    v25 = v26 + v27 - v28 * v29;
    
    /* Use inline assembly to create register constraints */
    asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3) : : "cc", "memory");
    asm volatile("" : "+m"(v4), "+r"(v5), "+r"(v6) : : "cc");
    
    /* More complex expressions */
    int t1 = (v1 * v2) + (v3 << (v4 & 3)) - (v5 | v6);
    int t2 = (v7 & v8) ^ (v9 + v10) | (v11 - v12);
    long t3 = (v13 * v14) + (v15 >> (v16 & 7)) * (v17 & 0xFF);
    int t4 = (v18 | v19) & (v20 ^ v21) + (v22 << 1);
    int t5 = (v23 - v24) * (v25 + v26) / (v27 & 0x7F + 1);
    long t6 = (v28 * v29) | (v30 << 2) + (t1 & t2);
    
    /* Force memory accesses with volatile */
    volatile int mem1 = t1;
    volatile short mem2 = t2;
    volatile char mem3 = t3 & 0xFF;
    
    /* More operations after memory access */
    t1 = t1 ^ mem1;
    t2 = t2 + mem2;
    t3 = t3 | (mem3 * 256);
    
    /* Final aggregation keeping all values live as long as possible */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ 
                 v6 ^ v7 ^ v8 ^ v9 ^ v10 ^
                 v11 ^ v12 ^ v13 ^ v14 ^ v15 ^
                 v16 ^ v17 ^ v18 ^ v19 ^ v20 ^
                 v21 ^ v22 ^ v23 ^ v24 ^ v25 ^
                 v26 ^ v27 ^ v28 ^ v29 ^ v30 ^
                 t1 ^ t2 ^ (t3 & 0xFFFFFFFF) ^ t4 ^ t5 ^ (t6 & 0xFFFFFFFF);
    
    return result & global_mask;
}

/* Another high-pressure function with different patterns */
__attribute__((noinline, noipa))
int secondary_pressure(int base) {
    /* Create many interdependent variables */
    int a = base;
    int b = a * 3;
    int c = b + 7;
    int d = c ^ a;
    int e = d << 2;
    int f = e | b;
    int g = f - c;
    int h = g * d;
    int i = h >> 1;
    int j = i & 0xFF;
    int k = j + e;
    int l = k ^ f;
    int m = l * g;
    int n = m / (h + 1);
    int o = n | i;
    int p = o ^ j;
    int q = p + k;
    int r = q * l;
    int s = r & m;
    int t = s | n;
    int u = t ^ o;
    int v = u + p;
    int w = v * q;
    int x = w ^ r;
    int y = x | s;
    int z = y & t;
    
    /* Complex expression with many operands */
    int complex = (a * b) + (c << (d & 3)) - (e | f) ^ 
                  (g & h) | (i + j) * (k - l) / (m & 0xF + 1) +
                  (n ^ o) & (p | q) << (r & 7) - 
                  (s * t) | (u ^ v) + (w & x) * (y | z);
    
    /* Force some spills with inline asm */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
                       "r"(f), "r"(g), "r"(h), "r"(i), "r"(j) : "memory");
    
    return complex + z;
}

int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 1;
    if (iterations > 1000) iterations = 1000;
    
    int total = 0;
    
    /* Loop to ensure the function is called multiple times */
    for (volatile int i = 0; i < iterations; i++) {
        /* Use volatile inputs to prevent constant propagation */
        volatile int input1 = global_seed + i;
        volatile int input2 = argc * 3 + i;
        volatile int input3 = (i * 7) & 0xFF;
        
        /* Call the high-pressure functions */
        int result1 = create_reload_pressure(input1, input2, input3);
        int result2 = secondary_pressure(result1 + i);
        
        total += result1 ^ result2;
        
        /* Modify global to prevent optimization */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: 0x%08x\n", total & 0xFFFFFFFF);
    
    return (total & 0xFF);
}
