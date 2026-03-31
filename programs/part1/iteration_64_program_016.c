#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int g_var1 = 42;
volatile int g_var2 = 73;
int g_array[256] = {0};

/* Helper functions to prevent inlining */
__attribute__((noinline)) 
__attribute__((optimize("O3")))
int compute_chain(int seed) {
    volatile int barrier = 0;
    int a = seed, b = seed * 2, c = seed + 1;
    int d = 0, e = 0, f = 0, g = 0, h = 0;
    
    /* Long chain of dependent operations */
    for (int i = 0; i < 32; i++) {
        a += b ^ c;
        b *= a | 1;
        c ^= b + i;
        d = a + c;
        e = b - d;
        f = c * e;
        g = d ^ f;
        h = e + g;
        
        /* Memory barrier to split scheduling regions */
        asm volatile("" : : : "memory");
        
        /* Data-dependent exit condition */
        if (__builtin_expect_with_probability(g_array[i] != 0, 0, 0.3)) {
            barrier = 1;
            break;
        }
        
        /* More dependent operations */
        a = h + i;
        b = g - i;
        c = f * (i + 1);
    }
    
    /* Mix with global variables for aliasing effects */
    int* p1 = (int*)&g_var1;
    int* p2 = (int*)&g_var2;
    *p1 += a;
    *p2 ^= b;
    
    return a + b + c + d + e + f + g + h + barrier;
}

__attribute__((noinline))
__attribute__((optimize("O3")))
int recursive_compute(int n, int acc) {
    if (n <= 0) return acc;
    
    int local_vars[8] = {acc, n, 0, 0, 0, 0, 0, 0};
    
    /* Complex computation in recursion */
    for (int i = 0; i < 8; i++) {
        local_vars[i] += i * n;
        local_vars[(i + 1) % 8] ^= local_vars[i];
    }
    
    /* Memory operation with uncertain latency */
    volatile int* mem = (volatile int*)&g_array[n % 256];
    int mem_val = *mem;
    
    /* Chain of operations after memory access */
    int result = acc;
    for (int i = 0; i < 4; i++) {
        result += local_vars[i] * mem_val;
        result ^= local_vars[i + 4];
        asm volatile("" : : : "memory");  /* Scheduling barrier */
    }
    
    return recursive_compute(n - 1, result);
}

__attribute__((noinline))
__attribute__((optimize("O3")))
int switch_complex(int selector) {
    /* Many local variables to create register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8;
    int v9 = 9, v10 = 10, v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    int result = 0;
    
    /* Switch with many cases - creates complex control flow */
    switch (selector % 12) {
        case 0:
            v1 += v2; v3 *= v4; v5 ^= v6;
            result = v1 + v3 + v5;
            /* Irregular control flow with goto */
            if (__builtin_expect(v1 > 100, 0)) goto merge_point;
            break;
        case 1:
            v2 -= v3; v4 |= v5; v6 &= v7;
            result = v2 * v4 * v6;
            break;
        case 2:
            v7 <<= v8; v9 >>= v10; v11 %= v12;
            result = v7 | v9 | v11;
            break;
        case 3:
            v13 += v14 * v15; v1 ^= v2; v3 |= v4;
            result = v13 - v1 - v3;
            break;
        case 4:
            v5 &= v6; v7 <<= 1; v8 >>= 2;
            result = v5 + v7 + v8;
            break;
        case 5:
            v9 *= v10; v11 /= (v12 | 1); v13 += v14;
            result = v9 ^ v11 ^ v13;
            break;
        case 6:
            v15 -= v1; v2 |= v3; v4 &= v5;
            result = v15 * v2 * v4;
            break;
        case 7:
            v6 <<= v7; v8 >>= v9; v10 %= (v11 | 1);
            result = v6 | v8 | v10;
            break;
        case 8:
            v12 += v13 * v14; v15 ^= v1; v2 |= v3;
            result = v12 - v15 - v2;
            break;
        case 9:
            v4 &= v5; v6 <<= 3; v7 >>= 4;
            result = v4 + v6 + v7;
            break;
        case 10:
            v8 *= v9; v10 /= (v11 | 1); v12 += v13;
            result = v8 ^ v10 ^ v12;
            break;
        case 11:
            v14 -= v15; v1 |= v2; v3 &= v4;
            result = v14 * v1 * v3;
            break;
    }
    
merge_point:
    /* Merge computation from all paths */
    result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + 
              v9 + v10 + v11 + v12 + v13 + v14 + v15;
    
    /* Do-while with break creating internal control edges */
    int counter = 0;
    do {
        if (__builtin_expect_with_probability(result > 1000, 0, 0.2)) {
            result >>= 1;
            break;
        }
        result <<= 1;
        counter++;
        asm volatile("" : : : "memory");
    } while (counter < 4);
    
    return result;
}

__attribute__((noinline))
__attribute__((optimize("O3")))
int pipelined_computation(int iterations) {
    int acc = 0;
    
    /* Outer loop with software pipelining characteristics */
    for (int i = 0; i < iterations; i++) {
        int tmp = i;
        
        /* Short inner loop with independent-ish operations */
        for (int j = 0; j < 8; j++) {
            tmp += (j * g_array[(i + j) % 256]);
            tmp ^= (j + 1);
            
            /* Memory clobber to force scheduler consideration */
            if (__builtin_expect_with_probability((j & 3) == 0, 1, 0.7)) {
                asm volatile("" : : : "memory");
            }
        }
        
        /* Mix with globals for aliasing */
        int* alias1 = (int*)&g_var1;
        int* alias2 = (int*)&g_var2;
        *alias1 += tmp & 0xFF;
        *alias2 ^= tmp >> 8;
        
        acc += tmp;
        
        /* Data-dependent loop exit */
        if (__builtin_expect_with_probability(acc > 1000000, 0, 0.1)) {
            break;
        }
    }
    
    return acc;
}

int main() {
    int result = 0;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 256; i++) {
        g_array[i] = (i * 37) & 0xFF;
    }
    
    printf("Starting scheduler stress test...\n");
    
    /* Kernel 1: Long chain with data-dependent break */
    result += compute_chain(12345);
    
    /* Kernel 2: Complex switch statement */
    for (int i = 0; i < 50; i++) {
        result ^= switch_complex(i);
    }
    
    /* Kernel 3: Recursive computation */
    result += recursive_compute(8, result & 0xFF);
    
    /* Kernel 4: Manual software pipelining */
    result += pipelined_computation(100);
    
    /* Additional complex loop with irregular control flow */
    int x = 0, y = 0, z = 0;
    volatile int* ptr = &g_array[0];
    
    while (x < 100) {
        /* Label for goto creating irregular CFG */
        restart_point:
        
        y = *ptr;
        z = y * x;
        
        /* Chain of dependent operations */
        for (int i = 0; i < 16; i++) {
            z += (z << i) | (z >> (32 - i));
            z ^= x + i;
            
            /* Occasional goto back */
            if (__builtin_expect_with_probability((z & 0xFF) == 0x42, 0, 0.05)) {
                x++;
                goto restart_point;
            }
        }
        
        result += z;
        x++;
        ptr = &g_array[x % 256];
    }
    
    printf("Result checksum: %d\n", result);
    return result != 0 ? 0 : 1;
}
