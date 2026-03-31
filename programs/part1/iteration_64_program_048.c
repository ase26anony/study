/* haifa-sched-trigger.c
 * Designed to trigger haifa-sched.cc uncovered lines 4681-4691
 * Compile with: gcc -O3 -fschedule-insns2 -fno-guess-branch-probability -fno-inline haifa-sched-trigger.c -o haifa-sched-trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing to create scheduler complexity */
volatile int g_counter = 0;
int g_array[256];
int *g_ptr1 = &g_array[0];
int *g_ptr2 = &g_array[128];

/* NOINLINE functions to create scheduling region boundaries */
__attribute__((noinline)) 
static int compute_chain(int seed) {
    /* Long chain of dependent operations */
    int a = seed;
    int b = a * 3;
    int c = b ^ 0x55AA55AA;
    int d = c + 777;
    int e = d * 13;
    int f = e ^ 0x12345678;
    int g = f - 999;
    int h = g * 7;
    int i = h ^ 0x87654321;
    int j = i + 1234;
    
    /* Memory barrier to split scheduling region */
    asm volatile("" : : : "memory");
    
    /* Continue the chain */
    int k = j * 11;
    int l = k ^ 0xDEADBEEF;
    int m = l - 4321;
    int n = m * 17;
    int o = n ^ 0xCAFEBABE;
    
    return o;
}

__attribute__((noinline))
static int recursive_compute(int depth, int value) {
    if (depth <= 0) {
        return value;
    }
    
    /* Create register pressure with many variables */
    int v1 = value * 2;
    int v2 = v1 + depth;
    int v3 = v2 ^ 0x11111111;
    int v4 = v3 * 3;
    int v5 = v4 - 100;
    int v6 = v5 ^ 0x22222222;
    int v7 = v6 * 5;
    int v8 = v7 + 200;
    int v9 = v8 ^ 0x33333333;
    
    /* Recursive call - scheduler may save/restore state here */
    int result = recursive_compute(depth - 1, v9);
    
    /* More operations after recursion */
    int r1 = result * 7;
    int r2 = r1 ^ 0x44444444;
    
    return r2;
}

__attribute__((noinline))
static void complex_switch(int selector, int *results) {
    /* Many local variables for register pressure */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    int q = 17, r = 18, s = 19, t = 20, u = 21, v = 22, w = 23, x = 24;
    
    /* Switch with many cases - creates complex control flow */
    switch (selector & 0xF) {
        case 0:
            a += b; c *= d; e ^= f; g -= h;
            results[0] = a + c + e + g;
            break;
        case 1:
            i += j; k *= l; m ^= n; o -= p;
            results[1] = i + k + m + o;
            break;
        case 2:
            q += r; s *= t; u ^= v; w -= x;
            results[2] = q + s + u + w;
            break;
        case 3:
            a ^= i; b *= q; c += s; d ^= u;
            results[3] = a + b + c + d;
            break;
        case 4:
            e -= m; f *= w; g ^= x; h += v;
            results[4] = e + f + g + h;
            break;
        case 5:
            j ^= t; k += p; l -= n; m *= r;
            results[5] = j + k + l + m;
            break;
        case 6:
            n += o; p ^= q; r *= s; t -= u;
            results[6] = n + p + r + t;
            break;
        case 7:
            v ^= w; x += a; b -= c; d *= e;
            results[7] = v + x + b + d;
            break;
        case 8:
            f += g; h ^= i; j *= k; l -= m;
            results[8] = f + h + j + l;
            break;
        case 9:
            n *= o; p += q; r ^= s; t -= u;
            results[9] = n + p + r + t;
            break;
        case 10:
            v += w; x ^= a; b *= c; d -= e;
            results[10] = v + x + b + d;
            break;
        case 11:
            f ^= g; h += i; j *= k; l -= m;
            results[11] = f + h + j + l;
            break;
        case 12:
            n += o; p ^= q; r *= s; t -= u;
            results[12] = n + p + r + t;
            break;
        case 13:
            v *= w; x += a; b ^= c; d -= e;
            results[13] = v + x + b + d;
            break;
        case 14:
            f += g; h *= i; j ^= k; l -= m;
            results[14] = f + h + j + l;
            break;
        case 15:
            n ^= o; p += q; r *= s; t -= u;
            results[15] = n + p + r + t;
            break;
    }
    
    /* Memory clobber to affect scheduling */
    asm volatile("" : : : "memory");
}

__attribute__((noinline))
static int loop_with_speculation(int *data, int size) {
    int sum = 0;
    int i = 0;
    
    /* Loop with data-dependent exit condition */
    while (__builtin_expect_with_probability(i < size, 1, 0.7)) {
        /* Create predictable but non-trivial branch probability */
        if (__builtin_expect_with_probability(data[i] != 0, 1, 0.6)) {
            /* Long computation chain in taken branch */
            int val = data[i];
            val = val * 3 + 1;
            val = val ^ 0xAAAAAAAA;
            val = val * 7 - 3;
            val = val ^ 0x55555555;
            val = val * 13 + 7;
            sum += val;
            
            /* Memory operation with volatile to create uncertainty */
            g_counter = val;
        } else {
            /* Different computation for else branch */
            int val = i;
            val = val * 5 - 2;
            val = val ^ 0xCCCCCCCC;
            val = val * 11 + 5;
            sum -= val;
        }
        
        /* Pointer aliasing to create scheduler complexity */
        *g_ptr1 = sum;
        int temp = *g_ptr2;
        sum ^= temp;
        
        i++;
        
        /* Small inner loop to create nested scheduling regions */
        for (int j = 0; j < 3; j++) {
            sum = (sum * 3) ^ j;
        }
    }
    
    return sum;
}

__attribute__((noinline))
static int software_pipelined_compute(int iterations) {
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    /* Manual software pipelining pattern */
    for (int i = 0; i < iterations; i++) {
        /* Stage 1 */
        int val1 = i * 3;
        val1 = val1 ^ 0x11111111;
        
        /* Stage 2 (from previous iteration conceptually) */
        int val2 = acc2 * 7;
        val2 = val2 - 5;
        
        /* Stage 3 (from two iterations ago) */
        int val3 = acc3 ^ 0x33333333;
        
        /* Rotate accumulators */
        acc3 = acc2;
        acc2 = acc1;
        acc1 = val1 + val2 + val3;
        
        /* Irregular control flow with goto */
        if (__builtin_expect_with_probability((i & 0x7) == 0, 0, 0.3)) {
            /* Jump back creates complex CFG */
            i--;
            continue;
        }
    }
    
    return acc1 + acc2 + acc3;
}

__attribute__((optimize("O3")))
int main() {
    int final_result = 0;
    
    /* Initialize global array with non-zero values */
    for (int i = 0; i < 256; i++) {
        g_array[i] = (i * 7) ^ 0x1234;
    }
    
    /* Kernel 1: Long chain with data-dependent loop */
    printf("Starting kernel 1...\n");
    int kernel1_result = loop_with_speculation(g_array, 128);
    final_result ^= kernel1_result;
    printf("Kernel 1 result: %d\n", kernel1_result);
    
    /* Kernel 2: Complex switch statement */
    printf("Starting kernel 2...\n");
    int switch_results[16] = {0};
    for (int i = 0; i < 100; i++) {
        complex_switch(i, switch_results);
    }
    for (int i = 0; i < 16; i++) {
        final_result += switch_results[i];
    }
    printf("Kernel 2 accumulated\n");
    
    /* Kernel 3: Recursive computation */
    printf("Starting kernel 3...\n");
    int kernel3_result = recursive_compute(4, 42);
    final_result *= kernel3_result;
    printf("Kernel 3 result: %d\n", kernel3_result);
    
    /* Kernel 4: Software pipelined loop */
    printf("Starting kernel 4...\n");
    int kernel4_result = software_pipelined_compute(1000);
    final_result ^= kernel4_result;
    printf("Kernel 4 result: %d\n", kernel4_result);
    
    /* Kernel 5: Multiple compute chains */
    printf("Starting kernel 5...\n");
    int chain_sum = 0;
    for (int i = 0; i < 50; i++) {
        chain_sum += compute_chain(i);
        
        /* do-while with break to create internal control edges */
        int j = 0;
        do {
            if (__builtin_expect_with_probability(j > 5, 0, 0.2)) {
                break;
            }
            chain_sum ^= j;
            j++;
        } while (j < 10);
    }
    final_result += chain_sum;
    printf("Kernel 5 result: %d\n", chain_sum);
    
    /* Final checksum */
    printf("\nFinal checksum: %d\n", final_result);
    
    /* Use result to prevent elimination */
    if (final_result == 0) {
        printf("Unexpected zero result\n");
    }
    
    return 0;
}
