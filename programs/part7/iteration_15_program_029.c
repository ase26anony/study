/* test_reload.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent constant propagation */
volatile int global_seed1 = 12345;
volatile int global_seed2 = 67890;
volatile int global_seed3 = 54321;

/* Prevent inlining and inter-procedural optimization */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3) {
    /* Declare many local variables with different types to exhaust registers */
    int v1 = input1 + 1;
    int v2 = input2 - 1;
    int v3 = input3 * 2;
    short v4 = (short)(input1 & 0xFFFF);
    short v5 = (short)(input2 >> 8);
    char v6 = (char)(input3 & 0xFF);
    long v7 = (long)input1 * input2;
    long v8 = (long)input2 * input3;
    int v9 = v1 ^ v2;
    int v10 = v2 | v3;
    short v11 = v4 + v5;
    char v12 = v6 ^ 0x55;
    long v13 = v7 - v8;
    int v14 = v9 & v10;
    int v15 = v1 << 3;
    int v16 = v2 >> 2;
    int v17 = v3 + v4;
    int v18 = v5 * v6;
    int v19 = v7 & 0xFFFFFFFF;
    int v20 = v8 >> 16;
    int v21 = v9 | v10;
    int v22 = v11 * v12;
    int v23 = v13 & 0xFFFF;
    int v24 = v14 ^ v15;
    int v25 = v16 + v17;
    int v26 = v18 - v19;
    int v27 = v20 | v21;
    int v28 = v22 & v23;
    int v29 = v24 << 1;
    int v30 = v25 >> 1;
    
    /* Complex expressions with multiple operands to force reloads */
    v1 = (v2 & v3) | (v4 << v5) - v6;
    v7 = (v8 * v9) + (v10 << v11) - (v12 & v13);
    v14 = ((v15 | v16) ^ (v17 & v18)) + (v19 >> v20);
    v21 = (v22 * v23) - (v24 << 3) + (v25 & 0xFF);
    v26 = ((v27 | v28) ^ (v29 & v30)) + (v1 >> 2);
    
    /* More operations to keep values live */
    v2 = v3 + v4 * v5 - v6 / (v7 ? v7 : 1);
    v8 = v9 | (v10 & v11) ^ (v12 << v13);
    v14 = v15 * v16 + v17 - v18 * v19;
    v20 = (v21 & v22) | (v23 ^ v24) + (v25 << v26);
    
    /* Use inline assembly to create register constraints */
    asm volatile ("" : "+r" (v1), "+r" (v2), "+r" (v3) : : "cc", "memory");
    asm volatile ("" : "+m" (v4), "+r" (v5) : : "cc");
    
    /* More complex expressions with volatile memory accesses */
    volatile int mem1 = v6;
    volatile int mem2 = v7;
    v8 = v8 + mem1 * mem2;
    
    v9 = (v10 << mem1) | (v11 >> mem2);
    v12 = v13 * v14 + v15 - v16 / (mem1 ? mem1 : 1);
    
    /* Even more operations to maintain pressure */
    v17 = v18 & v19 | v20 ^ v21;
    v22 = v23 * v24 - v25 + v26;
    v27 = (v28 << 2) | (v29 >> 3) ^ v30;
    
    /* Create addressing mode pressure */
    int* ptr = &v1;
    v2 = *ptr + v3;
    ptr = &v4;
    v5 = *ptr - v6;
    
    /* Final aggregation to prevent dead code elimination */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10;
    result ^= v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18 ^ v19 ^ v20;
    result ^= v21 ^ v22 ^ v23 ^ v24 ^ v25 ^ v26 ^ v27 ^ v28 ^ v29 ^ v30;
    
    return result;
}

/* Another high-pressure function with different patterns */
__attribute__((noinline, noipa))
int secondary_pressure(int base) {
    /* Different variable naming pattern */
    int a = base + 100;
    int b = base * 2;
    int c = base & 0xFF;
    int d = base | 0xAA;
    int e = base ^ 0x55;
    int f = a + b;
    int g = c - d;
    int h = e * f;
    int i = g & h;
    int j = a | b;
    int k = c ^ d;
    int l = e + f;
    int m = g - h;
    int n = i * j;
    int o = k & l;
    int p = m | n;
    int q = o ^ p;
    int r = a << 2;
    int s = b >> 1;
    int t = c & 0xF0;
    int u = d | 0x0F;
    int v = e ^ 0x33;
    int w = f + g;
    int x = h - i;
    int y = j * k;
    int z = l & m;
    
    /* Complex nested expressions */
    a = ((b & c) | (d << e)) + ((f ^ g) - (h >> i));
    j = (k * l) & (m | n) ^ (o << p);
    q = ((r & s) | (t ^ u)) + ((v << w) - (x >> y));
    
    /* Force memory accesses */
    volatile int temp = z;
    a = a + temp;
    b = b - temp;
    
    /* Use all variables in final computation */
    return a + b + c + d + e + f + g + h + i + j +
           k + l + m + n + o + p + q + r + s + t +
           u + v + w + x + y + z;
}

int main(int argc, char **argv) {
    int total = 0;
    
    /* Use volatile loop counter to prevent optimization */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    if (iterations > 1000) iterations = 1000;
    
    printf("Starting reload pressure test with %d iterations\n", iterations);
    
    for (volatile int i = 0; i < iterations; i++) {
        /* Mix global and argument inputs */
        int input1 = global_seed1 + i;
        int input2 = global_seed2 - i;
        int input3 = global_seed3 ^ i;
        
        /* Call high-pressure functions */
        total ^= create_reload_pressure(input1, input2, input3);
        total += secondary_pressure(input1 + input2);
        
        /* Modify globals slightly */
        global_seed1 ^= total;
        global_seed2 += i;
        global_seed3 -= total & 0xFF;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: 0x%08x\n", total & 0xFFFFFFFF);
    
    return (total & 0xFF);
}
