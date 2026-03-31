#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int g_var1 = 42;
volatile int g_var2 = 73;
int g_array[256];

/* Helper functions with noinline to prevent inlining */
__attribute__((noinline, optimize("O3")))
static int compute_chain(int seed) {
    volatile int barrier;
    int a = seed, b = seed * 2, c = seed + 1, d = seed ^ 0x55;
    int e = seed >> 2, f = seed << 1, g = seed % 17, h = seed | 0xFF;
    
    /* Long chain of dependent operations */
    for (int i = 0; i < 32; i++) {
        a += b * c;
        b ^= d + e;
        c *= f - g;
        d += h ^ a;
        e = (e << 3) | (e >> 29);
        f = f * 3 + 1;
        g = g ^ (g >> 1);
        h = h + (h << 2) + (h << 3);
        
        /* Memory barrier to split scheduling regions */
        if (i == 16) {
            asm volatile("" : : : "memory");
            barrier = g_var1;
        }
        
        /* Data-dependent exit condition */
        if (a & 0x1000) {
            d += g_var2;
            break;
        }
    }
    
    /* Another chain with __builtin_expect */
    int result = a;
    for (int j = 0; j < 8; j++) {
        if (__builtin_expect_with_probability((result & (1 << j)) != 0, 0, 0.7)) {
            result += b * c;
        } else {
            result ^= d + e;
        }
        result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    return result;
}

__attribute__((noinline, optimize("O3")))
static int switch_computation(int input) {
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    /* Switch with many cases - creates complex control flow */
    switch (input % 12) {
        case 0:
            v1 += v2 * v3; v4 ^= v5; v6 = v7 << 2;
            v8 += g_var1; v9 *= v10; v11 = v12 | v13;
            break;
        case 1:
            v2 += v3 * v4; v5 ^= v6; v7 = v8 << 3;
            v9 += g_var2; v10 *= v11; v12 = v13 | v14;
            break;
        case 2:
            v3 += v4 * v5; v6 ^= v7; v8 = v9 << 1;
            v10 += v1; v11 *= v12; v13 = v14 | v15;
            break;
        case 3:
            v4 += v5 * v6; v7 ^= v8; v9 = v10 << 4;
            v11 += v2; v12 *= v13; v14 = v15 | v16;
            break;
        case 4:
            v5 += v6 * v7; v8 ^= v9; v10 = v11 << 2;
            v12 += v3; v13 *= v14; v15 = v16 | v17;
            break;
        case 5:
            v6 += v7 * v8; v9 ^= v10; v11 = v12 << 3;
            v13 += v4; v14 *= v15; v16 = v17 | v18;
            break;
        case 6:
            v7 += v8 * v9; v10 ^= v11; v12 = v13 << 1;
            v14 += v5; v15 *= v16; v17 = v18 | v19;
            break;
        case 7:
            v8 += v9 * v10; v11 ^= v12; v13 = v14 << 4;
            v15 += v6; v16 *= v17; v18 = v19 | v20;
            break;
        case 8:
            v9 += v10 * v11; v12 ^= v13; v14 = v15 << 2;
            v16 += v7; v17 *= v18; v19 = v20 | v1;
            break;
        case 9:
            v10 += v11 * v12; v13 ^= v14; v15 = v16 << 3;
            v17 += v8; v18 *= v19; v20 = v1 | v2;
            break;
        case 10:
            v11 += v12 * v13; v14 ^= v15; v16 = v17 << 1;
            v18 += v9; v19 *= v20; v1 = v2 | v3;
            break;
        case 11:
            v12 += v13 * v14; v15 ^= v16; v17 = v18 << 4;
            v19 += v10; v20 *= v1; v2 = v3 | v4;
            break;
    }
    
    /* Merge all variables */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
}

__attribute__((noinline, optimize("O3")))
static int nested_loops(int iterations) {
    int sum = 0;
    
    /* Outer loop with software pipelining opportunity */
    for (int i = 0; i < iterations; i++) {
        int temp = i;
        
        /* Inner loop with irregular control flow */
        int j = 0;
        do {
            if (j & 1) {
                temp = (temp * 3 + 1) ^ g_array[j & 255];
            } else {
                temp = (temp >> 1) + g_array[(j + 1) & 255];
            }
            
            /* Create unpredictable branch */
            if (__builtin_expect((temp & 0xF) == 0, 0)) {
                asm volatile("" : : : "memory");
                break;
            }
            
            j++;
        } while (j < 8);
        
        sum += temp;
        
        /* Goto to create irregular CFG */
        if ((i & 3) == 0) {
            goto adjust;
        }
        continue;
        
    adjust:
        sum -= 1;
    }
    
    return sum;
}

/* Recursive function to create return points for state restoration */
__attribute__((noinline))
static int recursive_compute(int depth, int value) {
    if (depth <= 0) {
        return value ^ 0xAAAAAAAA;
    }
    
    int a = value * 1664525 + 1013904223;
    int b = recursive_compute(depth - 1, a >> 16);
    int c = recursive_compute(depth - 1, a & 0xFFFF);
    
    /* Complex merge point */
    volatile int mem_barrier = g_var1;
    asm volatile("" : : : "memory");
    
    return (b * c + a) & 0x7FFFFFFF;
}

/* Main orchestrator */
__attribute__((optimize("O3")))
int main() {
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        g_array[i] = (i * 1103515245 + 12345) & 0xFF;
    }
    
    int result = 0;
    
    /* Kernel 1: Long chain with data-dependent breaks */
    result ^= compute_chain(12345);
    
    /* Kernel 2: Switch-based computation */
    for (int i = 0; i < 100; i++) {
        result += switch_computation(result + i);
    }
    
    /* Kernel 3: Nested loops with goto */
    result += nested_loops(50);
    
    /* Kernel 4: Recursive computation */
    result ^= recursive_compute(3, result);
    
    /* Final mixing */
    for (int i = 0; i < 32; i++) {
        if (result & (1 << i)) {
            result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
        } else {
            result ^= g_array[i & 255];
        }
        
        /* Memory operation in the middle */
        if (i == 16) {
            g_var1 = result & 0xFF;
            asm volatile("" : : : "memory");
        }
    }
    
    printf("Result: %d\n", result);
    return 0;
}
