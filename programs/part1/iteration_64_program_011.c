#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int g_var1 = 42;
volatile int g_var2 = 73;
int g_array[256];

/* Helper functions marked noinline to prevent inlining */
__attribute__((noinline, optimize("O3")))
int compute_chain(int seed) {
    volatile int barrier;
    int a = seed, b = seed * 2, c = seed + 1;
    int d = 0, e = 0, f = 0, g = 0, h = 0;
    
    /* Long chain of dependent operations */
    for (int i = 0; i < 32; i++) {
        a += b ^ c;
        b *= a | 0x5555;
        c ^= b + g_var1;
        d = a + c;
        e = b - d;
        f = c * e;
        g = d ^ f;
        h = e + g;
        
        /* Memory barrier to split scheduling regions */
        asm volatile("" : : : "memory");
        
        /* Data-dependent exit condition */
        if (__builtin_expect_with_probability(h > 1000000, 0, 0.3)) {
            barrier = h;
            break;
        }
        
        /* More dependent operations */
        a = h + g_var2;
        b = f - e;
        c = g * d;
    }
    
    return a + b + c + d + e + f + g + h;
}

__attribute__((noinline, optimize("O3")))
int recursive_compute(int n, int acc) {
    if (n <= 0) return acc;
    
    int x = acc * 3;
    int y = x ^ n;
    int z = y + g_var1;
    
    /* Create register pressure with many variables */
    int v1 = x, v2 = y, v3 = z, v4 = x*y, v5 = y*z, v6 = z*x;
    int v7 = v1+v2, v8 = v3+v4, v9 = v5+v6, v10 = v7+v8;
    int v11 = v9 ^ v10, v12 = v11 * 7, v13 = v12 - 13;
    int v14 = v13 & 0xFF, v15 = v14 | 0xAA, v16 = v15 << 2;
    int v17 = v16 >> 1, v18 = v17 + 19, v19 = v18 * 3;
    int v20 = v19 % 97;
    
    /* Memory operation with uncertain latency */
    volatile int mem_read = g_array[n & 0xFF];
    
    /* Complex expression to keep variables alive */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    
    return recursive_compute(n - 1, result + mem_read);
}

__attribute__((noinline, optimize("O3")))
int switch_complex(int selector) {
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0;
    int r7 = 0, r8 = 0, r9 = 0, r10 = 0, r11 = 0, r12 = 0;
    
    /* Switch with many cases - creates complex CFG */
    switch (selector & 0xF) {
        case 0:
            r1 = g_var1 * 2;
            r2 = r1 + 5;
            r3 = r2 ^ 0xDEAD;
            for (int i = 0; i < 8; i++) r3 += i;
            break;
        case 1:
            r4 = g_var2 / 3;
            r5 = r4 | 0xBEEF;
            r6 = r5 << 3;
            asm volatile("" : : : "memory");
            break;
        case 2:
            r7 = recursive_compute(3, selector);
            r8 = r7 & 0xFF;
            break;
        case 3:
            r9 = compute_chain(selector);
            r10 = r9 % 256;
            break;
        case 4:
            r1 = selector * 7;
            r3 = r1 ^ selector;
            r5 = r3 + 11;
            break;
        case 5:
            r2 = selector + g_var1;
            r4 = r2 * 3;
            r6 = r4 - g_var2;
            break;
        case 6:
            r7 = selector | 0xABCD;
            r8 = r7 << 1;
            r9 = r8 >> 2;
            break;
        case 7:
            r10 = selector & 0xF0F0;
            r11 = r10 + 999;
            r12 = r11 * 2;
            break;
        case 8:
            r1 = selector ^ 0x1234;
            r4 = r1 + 4321;
            r7 = r4 * 5;
            break;
        case 9:
            r2 = selector - 100;
            r5 = r2 * r2;
            r8 = r5 % 17;
            break;
        case 10:
            r3 = selector + 1000;
            r6 = r3 / 7;
            r9 = r6 | 0xAA;
            break;
        case 11:
            r4 = selector * selector;
            r7 = r4 + 12345;
            r10 = r7 & 0xFFF;
            break;
        case 12:
            r5 = compute_chain(selector + 1);
            r8 = r5 ^ 0x55;
            r11 = r8 * 3;
            break;
        case 13:
            r6 = selector << 4;
            r9 = r6 >> 2;
            r12 = r9 + 777;
            break;
        case 14:
            r7 = recursive_compute(2, selector);
            r10 = r7 * 11;
            r1 = r10 % 19;
            break;
        default:  /* case 15 */
            r8 = selector * 13;
            r11 = r8 + 888;
            r2 = r11 ^ 0xEE;
            break;
    }
    
    /* Merge point with many live variables */
    return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12;
}

__attribute__((noinline, optimize("O3")))
int loop_with_inner_function(int iterations) {
    int total = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Data-dependent loop with unpredictable exit */
        int j = i;
        do {
            /* Small inner computation */
            int x = j * 3;
            int y = x + g_var1;
            int z = y ^ g_var2;
            
            /* Call to noinline function creates scheduling boundary */
            int inner = compute_chain(z);
            
            total += inner;
            
            /* Complex loop control */
            j = (j * 7 + 1) & 0xFF;
            
            /* Early exit with probability */
            if (__builtin_expect_with_probability((j & 0xF) == 0, 0, 0.2)) {
                goto early_exit;
            }
            
        } while (j != 0);
        
        early_exit:
        /* Empty label for goto target */
        asm volatile("" : : : "memory");
    }
    
    return total;
}

__attribute__((optimize("O3")))
int main() {
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        g_array[i] = i * 3 + 1;
    }
    
    int result = 0;
    
    /* Kernel 1: Long chain with data-dependent break */
    result += compute_chain(1);
    result += compute_chain(100);
    result += compute_chain(1000);
    
    /* Kernel 2: Switch with many cases */
    for (int i = 0; i < 32; i++) {
        result += switch_complex(i);
    }
    
    /* Kernel 3: Nested loops with inner function calls */
    result += loop_with_inner_function(8);
    
    /* Kernel 4: Recursive computation */
    result += recursive_compute(4, 1);
    
    /* Kernel 5: Manual software pipelining style */
    {
        int a = 1, b = 2, c = 3, d = 4;
        for (int i = 0; i < 16; i++) {
            /* Independent operations that could be pipelined */
            int t1 = a * 3 + g_var1;
            int t2 = b * 5 + g_var2;
            int t3 = c * 7 ^ result;
            int t4 = d * 11 | i;
            
            /* Memory barrier between pipeline stages */
            asm volatile("" : : : "memory");
            
            a = t1 + t2;
            b = t3 - t4;
            c = t1 ^ t3;
            d = t2 | t4;
            
            /* Call to create scheduling region boundary */
            if (__builtin_expect_with_probability((i & 3) == 0, 0, 0.25)) {
                c += compute_chain(i);
            }
        }
        result += a + b + c + d;
    }
    
    printf("Result: %d\n", result);
    return 0;
}
