/* haifa-sched-trigger.c
 * Program designed to trigger haifa scheduler state save/restore logic
 * Compile with: gcc -O3 -fschedule-insns2 -fno-guess-branch-probability -fno-inline -o haifa-trigger haifa-sched-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for aliasing and memory effects */
volatile int g_counter = 0;
int g_array[256];
int g_results[4] = {0};
int * volatile g_ptr1 = &g_results[0];
int * volatile g_ptr2 = &g_results[1];

/* Memory barrier */
#define MEMORY_BARRIER() asm volatile("" : : : "memory")

/* Helper functions marked noinline to prevent inlining */
__attribute__((noinline, optimize("O3")))
int compute_chain(int seed) {
    volatile int barrier = seed;
    int a = barrier, b = barrier + 1, c = barrier + 2;
    int d = barrier + 3, e = barrier + 4, f = barrier + 5;
    
    /* Long chain of dependent operations */
    a = a * 1103515245 + 12345;
    b = b ^ (a >> 16);
    c = c + (b & 0xFFFF);
    d = d * 1664525 + 1013904223;
    e = e ^ (d << 13);
    f = f + (e & 0xFF);
    
    a = a ^ (f << 7);
    b = b + (a >> 5);
    c = c * 134775813 + 1;
    d = d ^ (c << 16);
    e = e + (d & 0xFFFFFF);
    f = f * 1103515245 + 12345;
    
    /* Memory barrier in middle of computation */
    MEMORY_BARRIER();
    
    a = a + (f >> 16);
    b = b ^ (a << 13);
    c = c + (b & 0xFFF);
    d = d * 1664525 + 1013904223;
    e = e ^ (d >> 15);
    f = f + (e & 0xFFFF);
    
    /* Data-dependent branch with probability hint */
    if (__builtin_expect_with_probability((f & 0xFF) > 128, 1, 0.7)) {
        a = a * 3 + 1;
        b = b ^ 0xAAAAAAAA;
    } else {
        a = a >> 1;
        b = b ^ 0x55555555;
    }
    
    return a + b + c + d + e + f;
}

__attribute__((noinline, optimize("O3")))
int recursive_compute(int depth, int val) {
    if (depth <= 0) {
        MEMORY_BARRIER();
        return val;
    }
    
    int x = val * 1103515245 + 12345;
    int y = x ^ (val << 13);
    
    /* Create register pressure with many variables */
    int r1 = x + y, r2 = x - y, r3 = x * y, r4 = x ^ y;
    int r5 = r1 + r2, r6 = r3 - r4, r7 = r5 * r6, r8 = r5 ^ r6;
    int r9 = r7 + r8, r10 = r7 - r8, r11 = r9 * r10, r12 = r9 ^ r10;
    
    /* Access globals through different pointer types for aliasing */
    *g_ptr1 += r11;
    *g_ptr2 ^= r12;
    
    return recursive_compute(depth - 1, r11 + r12) + x;
}

__attribute__((noinline, optimize("O3")))
int switch_complex(int selector, int init) {
    int result = init;
    int v1 = init + 1, v2 = init + 2, v3 = init + 3;
    int v4 = init + 4, v5 = init + 5, v6 = init + 6;
    int v7 = init + 7, v8 = init + 8, v9 = init + 9;
    int v10 = init + 10, v11 = init + 11, v12 = init + 12;
    
    /* Complex switch with many cases - scheduler may save state at merge point */
    switch (selector & 0xF) {
        case 0:
            v1 = v1 * 3 + 1;
            v2 = v2 ^ v1;
            v3 = v3 + (v2 >> 1);
            result += v1 + v2 + v3;
            break;
        case 1:
            v4 = v4 * 1664525 + 1013904223;
            v5 = v5 ^ (v4 << 13);
            v6 = v6 + (v5 & 0xFF);
            result += v4 + v5 + v6;
            break;
        case 2:
            v7 = (v7 << 5) | (v7 >> 27);
            v8 = v8 + v7;
            v9 = v9 ^ v8;
            result += v7 + v8 + v9;
            break;
        case 3:
            v10 = v10 * 1103515245 + 12345;
            v11 = v11 ^ v10;
            v12 = v12 + (v11 & 0xFFFF);
            result += v10 + v11 + v12;
            break;
        case 4:
            v1 = v1 ^ v2;
            v3 = v3 + v1;
            v4 = v4 * v3;
            result += v1 + v3 + v4;
            break;
        case 5:
            v5 = v5 + 0x9E3779B9;
            v6 = v6 ^ (v5 >> 2);
            v7 = v7 + v6;
            result += v5 + v6 + v7;
            break;
        case 6:
            v8 = v8 * 134775813 + 1;
            v9 = v9 ^ v8;
            v10 = v10 + (v9 & 0xFFF);
            result += v8 + v9 + v10;
            break;
        case 7:
            v11 = (v11 << 13) | (v11 >> 19);
            v12 = v12 + v11;
            v1 = v1 ^ v12;
            result += v11 + v12 + v1;
            break;
        case 8:
            v2 = v2 * 0x5DEECE66D + 0xB;
            v3 = v3 ^ (v2 >> 16);
            v4 = v4 + v3;
            result += v2 + v3 + v4;
            break;
        case 9:
            v5 = v5 + v6 + v7;
            v6 = v6 ^ v5;
            v7 = v7 * v6;
            result += v5 + v6 + v7;
            break;
        case 10:
            v8 = v8 + 0x61C88647;
            v9 = v9 ^ v8;
            v10 = v10 + (v9 << 7);
            result += v8 + v9 + v10;
            break;
        case 11:
            v11 = v11 * 0x343FD + 0x269EC3;
            v12 = v12 ^ v11;
            v1 = v1 + (v12 >> 5);
            result += v11 + v12 + v1;
            break;
        case 12:
            v2 = v2 + v3 + v4;
            v3 = v3 ^ v2;
            v4 = v4 * 3 + 1;
            result += v2 + v3 + v4;
            break;
        case 13:
            v5 = (v5 << 17) | (v5 >> 15);
            v6 = v6 + v5;
            v7 = v7 ^ v6;
            result += v5 + v6 + v7;
            break;
        case 14:
            v8 = v8 * 0x6C078965 + 1;
            v9 = v9 ^ (v8 >> 15);
            v10 = v10 + v9;
            result += v8 + v9 + v10;
            break;
        case 15:
            v11 = v11 + 0x9E3779B9;
            v12 = v12 ^ v11;
            v1 = v1 + (v12 & 0xFFFFFF);
            result += v11 + v12 + v1;
            break;
    }
    
    MEMORY_BARRIER();
    return result;
}

__attribute__((noinline, optimize("O3")))
int loop_with_break(int iterations) {
    int sum = 0;
    int i = 0;
    
    /* Loop with data-dependent exit and internal break */
    while (1) {
        int val = g_array[i & 0xFF];
        
        /* Data-dependent break with probability hint */
        if (__builtin_expect_with_probability(val == 0, 0, 0.3)) {
            MEMORY_BARRIER();
            break;
        }
        
        /* Complex computation in loop body */
        int x = val * 1103515245 + 12345;
        int y = x ^ (val << 13);
        int z = y + (x >> 17);
        
        /* Nested do-while with break */
        int j = 0;
        do {
            if (__builtin_expect_with_probability(j > 3, 0, 0.8)) {
                z = z ^ 0xAAAAAAAA;
                break;
            }
            z = z * 1664525 + 1013904223;
            j++;
        } while (1);
        
        sum += z;
        i++;
        
        if (i >= iterations) {
            /* Another potential state save point */
            MEMORY_BARRIER();
            break;
        }
    }
    
    return sum;
}

__attribute__((noinline, optimize("O3")))
int software_pipelined_loop(int n) {
    int a = 0, b = 1, c = 2, d = 3;
    int sum = 0;
    
    /* Manual software pipelining attempt */
    for (int i = 0; i < n; i++) {
        /* Stage 1 computations */
        int t1 = a * 1103515245 + 12345;
        int t2 = b ^ (a >> 16);
        
        /* Memory operation between stages */
        g_counter++;
        
        /* Stage 2 computations */
        int t3 = c + (b & 0xFFFF);
        int t4 = d * 1664525 + 1013904223;
        
        /* Stage 3 computations */
        a = t1 ^ (t4 << 13);
        b = t2 + (t3 & 0xFF);
        c = t3 * 134775813 + 1;
        d = t4 ^ (t1 >> 15);
        
        sum += a + b + c + d;
        
        /* Irregular control flow with goto */
        if (__builtin_expect_with_probability((sum & 0xFF) == 0, 0, 0.1)) {
            i--;  /* Create loop with potential rollback */
            goto continue_loop;
        }
        
        continue_loop:
        /* Empty label for goto target */
        ;
    }
    
    return sum;
}

int main(void) {
    /* Initialize array with non-zero values */
    for (int i = 0; i < 256; i++) {
        g_array[i] = (i * 1103515245 + 12345) & 0xFF;
    }
    
    int result = 0;
    int seed = 42;
    
    /* Kernel 1: Long chain with data-dependent branch */
    result += compute_chain(seed);
    
    /* Kernel 2: Recursive computation */
    result += recursive_compute(4, seed);
    
    /* Kernel 3: Complex switch statement */
    for (int i = 0; i < 16; i++) {
        result += switch_complex(i, seed + i);
    }
    
    /* Kernel 4: Loop with data-dependent break */
    result += loop_with_break(1000);
    
    /* Kernel 5: Manual software pipelining */
    result += software_pipelined_loop(500);
    
    /* Mix in global results */
    result += g_results[0] + g_results[1];
    result += g_counter;
    
    /* Ensure computation isn't eliminated */
    printf("Result: %d\n", result);
    
    return 0;
}
