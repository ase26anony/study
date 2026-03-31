/* reload_coverage.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile variables to prevent optimization */
volatile int global_seed = 42;
volatile int global_mask = 0xFFFFFFFF;

/* Non-inlineable function that creates massive register pressure */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3, int input4) {
    /* Declare many local variables with different types to exhaust registers */
    int v1 = input1;
    int v2 = input2 + 1;
    int v3 = v1 * v2;
    short v4 = (short)(input3 & 0xFFFF);
    short v5 = (short)(input4 | 0x1234);
    char v6 = (char)(input1 ^ 0x55);
    char v7 = (char)(input2 & 0xAA);
    long v8 = (long)input1 * input3;
    long v9 = (long)input2 * input4;
    int v10 = v1 + v2;
    int v11 = v3 - v1;
    short v12 = v4 | v5;
    char v13 = v6 ^ v7;
    long v14 = v8 >> 2;
    long v15 = v9 << 1;
    
    /* More variables to increase pressure */
    int v16 = (v10 & v11) | (v12 << 3);
    int v17 = v13 * 7 - v1;
    short v18 = (v4 + v5) & 0xFF;
    char v19 = (v6 * v7) % 128;
    long v20 = v8 ^ v9;
    int v21 = (v16 << 2) | (v17 >> 1);
    int v22 = v18 * v19 + v10;
    short v23 = (v12 - v18) & 0x7FFF;
    char v24 = (v13 + v19) | 0x40;
    long v25 = v14 * v15 / 3;
    
    /* Even more variables */
    int v26 = v21 ^ v22;
    int v27 = v23 * v24;
    short v28 = (v16 & 0xFF) + v17;
    char v29 = (v18 | v19) ^ 0x33;
    long v30 = v20 + v25;
    int v31 = v26 << (v27 & 0x7);
    int v32 = v28 * v29 - v21;
    short v33 = (v22 + v23) >> 1;
    char v34 = (v24 * 3) % 64;
    long v35 = v25 ^ v30;
    
    /* Complex expressions with multiple uses of variables */
    v1 = (v2 & v3) | (v4 << (v5 & 0x7));
    v6 = (v7 * v8) - (v9 >> 2);
    v10 = ((v11 + v12) * (v13 - v14)) & global_mask;
    v15 = (v16 | v17) ^ (v18 & v19);
    v20 = (v21 << 3) + (v22 >> 1) - (v23 * v24);
    
    /* Use inline assembly to create register constraints */
    asm volatile("" : "+r"(v1), "+r"(v2) : : "cc", "memory");
    asm volatile("" : "+m"(v3), "+r"(v4) : : "cc");
    
    /* More complex operations mixing all variables */
    v25 = (v26 * v27) / (v28 + 1);
    v29 = (v30 & v31) | (v32 ^ v33);
    v34 = ((v35 + v1) * (v2 - v3)) % 256;
    
    /* Create addressing mode pressure with array accesses */
    int temp_array[8];
    for (int i = 0; i < 8; i++) {
        temp_array[i] = (v1 + i) ^ (v2 - i);
    }
    
    /* Use array elements in complex expressions */
    v5 = temp_array[0] * temp_array[1] - temp_array[2];
    v10 = (temp_array[3] << 2) | (temp_array[4] >> 3);
    v15 = temp_array[5] + temp_array[6] * temp_array[7];
    
    /* More inline assembly with memory constraints */
    asm volatile("" 
                 : "=r"(v20), "=m"(temp_array[0])
                 : "r"(v21), "m"(temp_array[1])
                 : "cc");
    
    /* Final computation using all variables to prevent dead code elimination */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ (v5 & 0xFF) ^ 
                 v6 ^ v7 ^ (v8 & 0xFFFFFFFF) ^ (v9 & 0xFFFFFFFF) ^
                 v10 ^ v11 ^ v12 ^ v13 ^ (v14 & 0xFFFFFFFF) ^ 
                 (v15 & 0xFFFFFFFF) ^ v16 ^ v17 ^ v18 ^ v19 ^
                 (v20 & 0xFFFFFFFF) ^ v21 ^ v22 ^ v23 ^ v24 ^
                 (v25 & 0xFFFFFFFF) ^ v26 ^ v27 ^ v28 ^ v29 ^
                 (v30 & 0xFFFFFFFF) ^ v31 ^ v32 ^ v33 ^ v34 ^
                 (v35 & 0xFFFFFFFF);
    
    /* Mix with global volatile to force memory accesses */
    result ^= global_seed;
    
    return result;
}

/* Another pressure function with different patterns */
__attribute__((noinline, noipa))
int secondary_pressure(int base) {
    /* Different variable patterns */
    int a = base * 3;
    int b = base + 0x12345678;
    int c = a ^ b;
    int d = (a & b) | (c << 1);
    int e = d * 7;
    int f = e >> 3;
    int g = f - a;
    int h = g * b;
    int i = h % 997;
    int j = i | 0xAA55;
    int k = j ^ d;
    int l = k << 4;
    int m = l + e;
    int n = m & 0x0F0F0F0F;
    int o = n * 3;
    int p = o - f;
    int q = p ^ g;
    int r = q | h;
    int s = r * i;
    int t = s >> 2;
    int u = t ^ j;
    int v = u + k;
    int w = v * l;
    int x = w % 256;
    int y = x | m;
    int z = y ^ n;
    
    /* Complex expression chain */
    a = (b + c) * (d - e);
    f = (g | h) & (i ^ j);
    k = (l << 2) + (m >> 1);
    n = (o * p) / (q + 1);
    r = (s & t) | (u ^ v);
    w = (x + y) * (z - a);
    
    /* Force spills with large expression */
    return a + b + c + d + e + f + g + h + i + j + 
           k + l + m + n + o + p + q + r + s + t + 
           u + v + w + x + y + z;
}

int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 1;
    if (iterations > 1000) iterations = 1000;
    
    int total_result = 0;
    
    /* Loop to ensure the pressure functions are actually called */
    for (volatile int i = 0; i < iterations; i++) {
        /* Use command line arguments and globals as inputs */
        int input1 = argc + i;
        int input2 = global_seed ^ i;
        int input3 = (argc * 7919) & 0xFFFF;
        int input4 = (i * 65537) | 0x1234;
        
        /* Call the register pressure functions */
        int result1 = create_reload_pressure(input1, input2, input3, input4);
        int result2 = secondary_pressure(input1 + input2);
        
        /* Combine results in a non-trivial way */
        total_result ^= (result1 * 31) + (result2 * 17);
        total_result = (total_result << 1) | (total_result >> 31);
        
        /* Modify global to prevent optimization */
        global_seed ^= result1;
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Final result: 0x%08x\n", total_result & 0xFFFFFFFF);
    
    return (total_result & 0xFF);
}
