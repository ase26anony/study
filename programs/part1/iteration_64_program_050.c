#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int g_var1 = 42;
volatile int g_var2 = 73;
int g_array[256];

/* Helper functions to prevent inlining */
__attribute__((noinline)) 
__attribute__((optimize("O3")))
int compute_chain(int seed) {
    volatile int barrier;
    int a = seed, b = seed * 2, c = seed + 1;
    int d = 0, e = 0, f = 0, g = 0, h = 0;
    
    /* Long chain of dependent operations */
    for (int i = 0; i < 100; i++) {
        a += b ^ c;
        b *= a | 0x5555;
        c ^= b + g_var1;
        d = c - a;
        e = d * b;
        f = e ^ c;
        g = f + a;
        h = g * d;
        
        /* Memory barrier to split scheduling regions */
        if (i % 7 == 0) {
            asm volatile("" : : : "memory");
            barrier = g_var2;
        }
        
        /* Data-dependent exit condition */
        if (h > 0x7FFFFFFF) {
            h &= 0xFFFF;
            break;
        }
    }
    
    /* Mix in some unpredictable branching */
    if (__builtin_expect_with_probability((a & 0xFF) > 128, 1, 0.7)) {
        return h + a;
    } else {
        return h - a;
    }
}

__attribute__((noinline))
__attribute__((optimize("O3")))
int recursive_compute(int n, int acc) {
    int local1, local2, local3, local4, local5;
    
    if (n <= 0) return acc;
    
    /* Create register pressure */
    local1 = acc * 3;
    local2 = local1 ^ 0xAAAA;
    local3 = local2 + g_var1;
    local4 = local3 * 2;
    local5 = local4 - acc;
    
    /* Memory operation with uncertain latency */
    volatile int* ptr = (volatile int*)&g_var2;
    int mem_val = *ptr;
    
    /* Recursive call - scheduler may save/restore state around this */
    int result = recursive_compute(n - 1, local5 + mem_val);
    
    /* More operations after return */
    result ^= local1;
    result *= local3;
    
    return result;
}

__attribute__((noinline))
__attribute__((optimize("O3")))
int switch_complex(int selector) {
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0;
    int r6 = 0, r7 = 0, r8 = 0, r9 = 0, r10 = 0;
    int r11 = 0, r12 = 0, r13 = 0, r14 = 0, r15 = 0;
    
    /* Large switch with different computation patterns */
    switch (selector & 0xF) {
        case 0:
            r1 = g_var1 * 2;
            r2 = r1 ^ 0x1234;
            for (int i = 0; i < 5; i++) r2 += i;
            break;
        case 1:
            r3 = g_var2 + 100;
            r4 = r3 / 3;
            asm volatile("" : : : "memory");
            break;
        case 2:
            r5 = recursive_compute(2, selector);
            r6 = r5 & 0xFF;
            break;
        case 3:
            r7 = selector * selector;
            r8 = r7 % 17;
            break;
        case 4:
            r9 = compute_chain(selector);
            r10 = r9 | 0xAA;
            break;
        case 5:
            r11 = selector << 3;
            r12 = r11 >> 1;
            break;
        case 6:
            r13 = ~selector;
            r14 = r13 + g_var1;
            break;
        case 7:
            r15 = selector ^ g_var2;
            r1 = r15 * 3;
            break;
        case 8:
            r2 = selector + 0x1000;
            r3 = r2 - 0x200;
            break;
        case 9:
            r4 = selector | 0xF0F0;
            r5 = r4 & 0x0F0F;
            break;
        case 10:
            r6 = selector * 7;
            r7 = r6 / 2;
            break;
        case 11:
            r8 = selector % 23;
            r9 = r8 + 11;
            break;
        case 12:
            r10 = selector ^ 0xFFFF;
            r11 = r10 + 1;
            break;
        case 13:
            r12 = selector << 2;
            r13 = r12 >> 4;
            break;
        case 14:
            r14 = selector + g_var1;
            r15 = r14 - g_var2;
            break;
        case 15:
            r1 = selector * selector;
            r2 = r1 % 31;
            break;
    }
    
    /* Merge all results */
    return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 +
           r11 + r12 + r13 + r14 + r15;
}

__attribute__((noinline))
__attribute__((optimize("O3")))
int loop_with_inner_function(int iterations) {
    int sum = 0;
    int i = 0;
    
    /* Loop with irregular control flow */
    do {
        if (i >= iterations) break;
        
        /* Call helper that creates scheduling region */
        int val = compute_chain(i);
        
        /* Complex condition with memory access */
        volatile int* alias1 = (volatile int*)&g_var1;
        volatile int* alias2 = (volatile int*)&g_var2;
        
        if (__builtin_expect((*alias1 ^ *alias2) > val, 0)) {
            sum += val;
            i++;
            continue;
        }
        
        /* Another path with different operations */
        sum -= val;
        i += 2;
        
        /* Jump back to create interesting control flow */
        if (i < iterations / 2) {
            goto loop_start;
        }
        
        loop_start:
        asm volatile("" : : : "memory");
        
    } while (i < iterations);
    
    return sum;
}

int main() {
    int result = 0;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 256; i++) {
        g_array[i] = i * 3 + 1;
    }
    
    /* Kernel 1: Long chain with data-dependent exit */
    result += compute_chain(12345);
    
    /* Kernel 2: Switch with many cases */
    for (int i = 0; i < 50; i++) {
        result ^= switch_complex(i);
    }
    
    /* Kernel 3: Nested loop structure */
    result += loop_with_inner_function(100);
    
    /* Kernel 4: Recursive computation */
    result += recursive_compute(4, result & 0xFF);
    
    /* Kernel 5: Manual software pipelining attempt */
    {
        int a = 1, b = 2, c = 3, d = 4, e = 5;
        int f = 6, g = 7, h = 8, j = 9, k = 10;
        int l = 11, m = 12, n = 13, o = 14, p = 15;
        
        for (int i = 0; i < 1000; i++) {
            /* Independent operations that could be pipelined */
            a = b + c;
            d = e * f;
            g = h ^ j;
            k = l - m;
            n = o | p;
            
            /* Create dependencies to force ordering */
            b = a + d;
            e = g + k;
            h = n + i;
            
            /* Memory barrier every 8 iterations */
            if (i % 8 == 0) {
                asm volatile("" : : : "memory");
                g_var1 = i;
            }
            
            /* Unpredictable branch */
            if (__builtin_expect_with_probability((i & 0x3F) == 0, 0, 0.3)) {
                c = d + e;
                f = g + h;
            }
        }
        
        result += a + b + c + d + e + f + g + h + j + k + l + m + n + o + p;
    }
    
    printf("Final checksum: %d\n", result);
    return 0;
}
