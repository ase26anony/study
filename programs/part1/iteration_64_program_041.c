#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for aliasing and memory effects */
volatile int g_counter = 0;
int g_array[256];
int g_results[4] = {0};

/* Prevent inlining to create scheduling boundaries */
__attribute__((noinline)) 
__attribute__((optimize("O3")))
int compute_chain(int seed, int iterations) {
    volatile int barrier;
    int a = seed, b = seed * 2, c = seed * 3;
    int d = seed + 1, e = seed + 2, f = seed + 3;
    int g = seed * 5, h = seed * 7, i = seed * 11;
    int j = seed ^ 0x55, k = seed ^ 0xAA, l = seed ^ 0xFF;
    
    /* Long chain of dependent operations */
    for (int n = 0; n < iterations; n++) {
        /* Data-dependent exit condition */
        if (n > 100 && (a & 0xF) == 0) {
            barrier = a;  /* Memory barrier */
            break;
        }
        
        /* Complex dependency chain */
        a = (a * 1103515245 + 12345) & 0x7FFFFFFF;
        b = b ^ (a >> 16);
        c = c + (b & 0xFF);
        d = d * 3 - c;
        e = e ^ (d << 3);
        f = f + (e >> 5);
        g = g * 5 + f;
        h = h ^ (g >> 2);
        i = i + (h & 0x3F);
        j = j * 7 - i;
        k = k ^ (j << 4);
        l = l + (k >> 3);
        
        /* Memory clobber to potentially trigger state save */
        asm volatile("" : : : "memory");
        
        /* Branch with probability hint */
        if (__builtin_expect_with_probability((l & 0x7) == 0, 0, 0.3)) {
            barrier = l;
            a += barrier;
        }
    }
    
    /* Final mixing */
    return a ^ b ^ c ^ d ^ e ^ f ^ g ^ h ^ i ^ j ^ k ^ l;
}

__attribute__((noinline))
__attribute__((optimize("O3")))
int recursive_scheduler_test(int depth, int val) {
    int local1 = val * 2;
    int local2 = val + depth;
    int local3 = val ^ 0x1234;
    int local4 = val * depth;
    
    if (depth <= 0) {
        return val;
    }
    
    /* Create register pressure with many variables */
    int t1 = local1 * 3;
    int t2 = local2 ^ t1;
    int t3 = local3 + t2;
    int t4 = local4 - t3;
    int t5 = t1 ^ t4;
    int t6 = t2 * t5;
    int t7 = t3 + t6;
    int t8 = t4 ^ t7;
    int t9 = t5 - t8;
    int t10 = t6 * t9;
    
    /* Recursive call - scheduler may save/restore state around call */
    int result = recursive_scheduler_test(depth - 1, t10);
    
    /* Post-recursion computation */
    result = result ^ t1 ^ t2 ^ t3 ^ t4 ^ t5 ^ t6 ^ t7 ^ t8 ^ t9 ^ t10;
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

__attribute__((noinline))
__attribute__((optimize("O3")))
int switch_scheduler_test(int selector) {
    /* Many local variables to create register pressure */
    int v1 = selector, v2 = selector * 2, v3 = selector * 3;
    int v4 = selector + 1, v5 = selector + 2, v6 = selector + 3;
    int v7 = selector ^ 1, v8 = selector ^ 2, v9 = selector ^ 3;
    int v10 = selector * 5, v11 = selector * 7, v12 = selector * 11;
    int v13 = selector + 10, v14 = selector + 20, v15 = selector + 30;
    int v16 = selector ^ 0x10, v17 = selector ^ 0x20, v18 = selector ^ 0x30;
    int v19 = selector * 13, v20 = selector * 17;
    
    /* Complex switch with many cases - scheduler may need state management */
    switch (selector & 0xF) {
        case 0:
            v1 = v2 * v3 + v4;
            v5 = v6 ^ v7;
            v8 = v9 - v10;
            break;
        case 1:
            v2 = v3 + v4 * v5;
            v6 = v7 ^ v8;
            v9 = v10 - v11;
            break;
        case 2:
            v3 = v4 ^ v5 + v6;
            v7 = v8 * v9;
            v10 = v11 - v12;
            break;
        case 3:
            v4 = v5 - v6 * v7;
            v8 = v9 ^ v10;
            v11 = v12 + v13;
            break;
        case 4:
            v5 = v6 + v7 ^ v8;
            v9 = v10 * v11;
            v12 = v13 - v14;
            break;
        case 5:
            v6 = v7 * v8 - v9;
            v10 = v11 ^ v12;
            v13 = v14 + v15;
            break;
        case 6:
            v7 = v8 ^ v9 + v10;
            v11 = v12 * v13;
            v14 = v15 - v16;
            break;
        case 7:
            v8 = v9 - v10 * v11;
            v12 = v13 ^ v14;
            v15 = v16 + v17;
            break;
        case 8:
            v9 = v10 + v11 ^ v12;
            v13 = v14 * v15;
            v16 = v17 - v18;
            break;
        case 9:
            v10 = v11 * v12 - v13;
            v14 = v15 ^ v16;
            v17 = v18 + v19;
            break;
        case 10:
            v11 = v12 ^ v13 + v14;
            v15 = v16 * v17;
            v18 = v19 - v20;
            break;
        case 11:
            v12 = v13 - v14 * v15;
            v16 = v17 ^ v18;
            v19 = v20 + v1;
            break;
        case 12:
            v13 = v14 + v15 ^ v16;
            v17 = v18 * v19;
            v20 = v1 - v2;
            break;
        case 13:
            v14 = v15 * v16 - v17;
            v18 = v19 ^ v20;
            v1 = v2 + v3;
            break;
        case 14:
            v15 = v16 ^ v17 + v18;
            v19 = v20 * v1;
            v2 = v3 - v4;
            break;
        default:  /* case 15 */
            v16 = v17 - v18 * v19;
            v20 = v1 ^ v2;
            v3 = v4 + v5;
            break;
    }
    
    /* Combine all variables */
    return v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10 ^
           v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18 ^ v19 ^ v20;
}

__attribute__((noinline))
__attribute__((optimize("O3")))
int loop_with_inner_function(int outer_iters) {
    int total = 0;
    
    for (int i = 0; i < outer_iters; i++) {
        /* Data-dependent loop with irregular control flow */
        int j = 0;
        
        /* do-while with break inside conditional */
        do {
            if (__builtin_expect_with_probability((i ^ j) & 0x3, 1, 0.7)) {
                total += i * j;
            } else {
                total -= i + j;
                if (j > 50) {
                    /* Early break creating complex control flow */
                    break;
                }
            }
            
            /* Small computation chain */
            int temp = i;
            temp = (temp * 3 + j) & 0xFF;
            temp = temp ^ (j << 2);
            temp = temp * 5 - i;
            
            total ^= temp;
            
            j++;
            
            /* Memory operation with volatile */
            volatile int* ptr = &g_counter;
            *ptr = *ptr + 1;
            
        } while (j < 100);
        
        /* Goto creating irregular CFG */
        if ((i & 0x7) == 0) {
            goto skip_point;
        }
        
        total += i * 100;
        
    skip_point:
        /* Continue normal execution */
        total = (total * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    return total;
}

__attribute__((noinline))
__attribute__((optimize("O3")))
int software_pipelined_style(int n) {
    int acc1 = 0, acc2 = 0, acc3 = 0;
    int i;
    
    /* Manual software pipelining attempt */
    for (i = 0; i < n - 2; i += 3) {
        /* Phase 1 */
        int t1 = g_array[i] * 3;
        acc1 += t1;
        
        /* Phase 2 (from previous iteration conceptually) */
        int t2 = g_array[i + 1] ^ 0x55;
        acc2 += t2;
        
        /* Phase 3 (from two iterations ago) */
        int t3 = g_array[i + 2] + 0xAA;
        acc3 += t3;
        
        /* Cross-phase dependencies */
        acc1 ^= acc2;
        acc2 += acc3;
        acc3 ^= acc1;
        
        /* Scheduling barrier */
        asm volatile("" : : : "memory");
    }
    
    /* Drain pipeline */
    for (; i < n; i++) {
        acc1 += g_array[i];
        acc2 ^= g_array[i];
        acc3 -= g_array[i];
    }
    
    return acc1 ^ acc2 ^ acc3;
}

/* Main function orchestrating all scheduler tests */
int main() {
    int result = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        g_array[i] = (i * 1103515245 + 12345) & 0xFF;
    }
    
    printf("Starting scheduler stress test...\n");
    
    /* Test 1: Long dependency chains with branch probability hints */
    result ^= compute_chain(42, 200);
    g_results[0] = result;
    
    /* Test 2: Recursive function creating call/return boundaries */
    result ^= recursive_scheduler_test(4, result);
    g_results[1] = result;
    
    /* Test 3: Complex switch statement with many cases */
    for (int i = 0; i < 32; i++) {
        result ^= switch_scheduler_test(result + i);
    }
    g_results[2] = result;
    
    /* Test 4: Loop with irregular control flow and goto */
    result ^= loop_with_inner_function(50);
    g_results[3] = result;
    
    /* Test 5: Manual software pipelining */
    result ^= software_pipelined_style(128);
    
    /* Final mixing to ensure all computations are used */
    result = result ^ g_results[0] ^ g_results[1] ^ g_results[2] ^ g_results[3];
    
    /* Use result to prevent elimination */
    printf("Result checksum: %d\n", result);
    printf("Global counter: %d\n", g_counter);
    
    return result != 0 ? 0 : 1;
}
