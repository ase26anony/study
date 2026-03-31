#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for aliasing and memory effects */
volatile int g_counter = 0;
int g_array[256];
int g_results[4] = {0};
unsigned long g_checksum = 0;

/* Helper functions to prevent inlining and create scheduling boundaries */
__attribute__((noinline)) 
int compute_chain(int start, int iterations) {
    int a = start, b = start * 2, c = start * 3;
    int d = start + 5, e = start + 7, f = start + 11;
    
    /* Long chain of dependent operations */
    for (int i = 0; i < iterations; i++) {
        a += b ^ c;
        b += c ^ d;
        c += d ^ e;
        d += e ^ f;
        e += f ^ a;
        f += a ^ b;
        
        /* Memory barrier to potentially split scheduling regions */
        asm volatile("" : : : "memory");
        
        /* Data-dependent exit condition */
        if (a > 1000000) {
            a = a % 1000;
            b = b % 1000;
        }
    }
    
    return a + b + c + d + e + f;
}

__attribute__((noinline))
void memory_intensive_op(int* ptr1, int* ptr2, int count) {
    volatile int* vptr = (volatile int*)ptr1;
    
    for (int i = 0; i < count; i++) {
        /* Mix of memory and arithmetic operations */
        int val = *vptr;
        val ^= ptr2[i % 16];
        val += i * 3;
        val ^= val >> 4;
        *vptr = val;
        
        /* Another scheduling barrier */
        asm volatile("" : : : "memory");
        
        ptr2[i % 16] = val ^ 0x5A5A5A5A;
    }
}

__attribute__((noinline))
int recursive_compute(int depth, int value) {
    if (depth <= 0) {
        return value;
    }
    
    /* Complex computation at each recursion level */
    int a = value * 3;
    int b = value + depth;
    int c = value ^ depth;
    
    a = (a * b) ^ c;
    b = (b + c) * a;
    c = (c ^ a) + b;
    
    /* Recursive call - scheduler may need to save/restore state */
    int result = recursive_compute(depth - 1, a + b + c);
    
    /* More computation after recursion */
    result ^= (a << 4);
    result += (b >> 2);
    result *= (c & 0xFF);
    
    return result;
}

__attribute__((optimize("O3")))
void complex_switch_case(int selector, int* results) {
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    /* Switch with many cases - creates complex control flow */
    switch (selector % 12) {
        case 0:
            v1 += v2 * v3;
            v4 ^= v5 | v6;
            v7 = v8 + v9 - v10;
            results[0] = v1 + v4 + v7;
            break;
        case 1:
            v2 *= v3 << 2;
            v5 = v6 ^ v7;
            v8 += v9 * v10;
            results[1] = v2 + v5 + v8;
            break;
        case 2:
            v3 = v4 ^ v5 ^ v6;
            v7 += v8 * 3;
            v9 = v10 | v11;
            results[2] = v3 + v7 + v9;
            break;
        case 3:
            v4 += recursive_compute(2, v5);
            v6 ^= v7 * v8;
            v9 = v10 + v11;
            results[3] = v4 + v6 + v9;
            break;
        case 4:
            v5 = compute_chain(v6, 50);
            v7 ^= v8 + v9;
            v10 *= v11 >> 1;
            results[0] = v5 + v7 + v10;
            break;
        case 5:
            v6 += v7 * v8 * v9;
            v10 = v11 ^ v12;
            v13 += v14 | v15;
            results[1] = v6 + v10 + v13;
            break;
        case 6:
            v7 = (v8 * v9) + (v10 ^ v11);
            v12 += v13 << 2;
            v14 = v15 & v16;
            results[2] = v7 + v12 + v14;
            break;
        case 7:
            v8 += v9 * v10;
            v11 = v12 ^ v13 ^ v14;
            v15 *= v16 + 1;
            results[3] = v8 + v11 + v15;
            break;
        case 8:
            v9 = compute_chain(v10, 25);
            v12 += v13 * v14;
            v15 ^= v16 | v17;
            results[0] = v9 + v12 + v15;
            break;
        case 9:
            v10 *= v11 + v12;
            v13 = v14 ^ v15;
            v16 += v17 * v18;
            results[1] = v10 + v13 + v16;
            break;
        case 10:
            v11 = v12 + v13 + v14;
            v15 ^= v16 * v17;
            v18 += v19 | v20;
            results[2] = v11 + v15 + v18;
            break;
        case 11:
            v12 = recursive_compute(3, v13);
            v14 += v15 * v16;
            v17 = v18 ^ v19;
            results[3] = v12 + v14 + v17;
            break;
    }
    
    /* Use all variables to prevent optimization */
    g_checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                  v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
}

__attribute__((noinline))
void software_pipelined_loop(int iterations) {
    int buffer[32];
    int accum[4] = {0};
    
    /* Initialize buffer */
    for (int i = 0; i < 32; i++) {
        buffer[i] = i * 3 + 1;
    }
    
    /* Outer loop with inner computation */
    for (int i = 0; i < iterations; i++) {
        int idx = i % 32;
        
        /* Manual software pipelining pattern */
        int stage1 = buffer[idx] * 3;
        int stage2 = stage1 + buffer[(idx + 1) % 32];
        int stage3 = stage2 ^ buffer[(idx + 2) % 32];
        int stage4 = stage3 * 7;
        
        /* Data-dependent branch with probability hint */
        if (__builtin_expect_with_probability((stage4 & 0xFF) > 128, 0, 0.7)) {
            /* This path may cause scheduler to save state */
            stage4 = recursive_compute(2, stage4);
            buffer[idx] = stage4 % 256;
        } else {
            stage4 = compute_chain(stage4, 10);
            buffer[idx] = stage4 % 256;
        }
        
        accum[i % 4] += stage4;
        
        /* Memory operation with barrier */
        asm volatile("" : : : "memory");
        g_counter = i;
    }
    
    /* Store results */
    for (int i = 0; i < 4; i++) {
        g_results[i] ^= accum[i];
    }
}

int main() {
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        g_array[i] = i * 3 + 7;
    }
    
    /* Kernel 1: Long chain with data-dependent exit */
    int result1 = 0;
    for (int i = 0; i < 100; i++) {
        /* Loop with unpredictable exit */
        int j = 0;
        while (g_array[j] != 0 && j < 100) {
            result1 += compute_chain(g_array[j], 5);
            j += (result1 % 7) + 1;
            
            /* Branch probability hint */
            if (__builtin_expect_with_probability(j > 50, 0, 0.3)) {
                j = j % 50;
            }
        }
    }
    
    /* Kernel 2: Switch statement with many cases */
    int switch_results[4] = {0};
    for (int i = 0; i < 50; i++) {
        complex_switch_case(i + result1, switch_results);
    }
    
    /* Kernel 3: Memory intensive operations with pointer aliasing */
    int local_array[16];
    for (int i = 0; i < 16; i++) {
        local_array[i] = i * 5 + 3;
    }
    
    /* Create aliasing pointers */
    int* ptr1 = &g_array[0];
    int* ptr2 = (int*)((char*)&g_array[0] + 1); /* Misaligned for aliasing effect */
    
    for (int i = 0; i < 20; i++) {
        memory_intensive_op(ptr1, local_array, 64);
        
        /* Irregular control flow with goto */
        if (__builtin_expect_with_probability((i & 3) == 0, 1, 0.8)) {
            /* Jump back creates interesting control flow */
            if (i > 5 && i < 15) {
                i += 1;
                continue;
            }
        }
    }
    
    /* Kernel 4: Software pipelined computation */
    software_pipelined_loop(200);
    
    /* Kernel 5: Complex loop with nested breaks */
    int final_result = 0;
    for (int outer = 0; outer < 10; outer++) {
        int inner = 0;
        do {
            if (inner > 5) {
                /* do-while with break creates control edges */
                if (__builtin_expect_with_probability((final_result & 0xF) == 0, 0, 0.4)) {
                    break;
                }
            }
            
            final_result += recursive_compute(3, outer * 10 + inner);
            final_result ^= compute_chain(inner, 3);
            
            inner++;
        } while (inner < 8);
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
    }
    
    /* Combine all results */
    unsigned long total = result1 + final_result + g_checksum;
    for (int i = 0; i < 4; i++) {
        total += switch_results[i];
        total += g_results[i];
    }
    
    /* Use volatile to ensure computation isn't eliminated */
    volatile unsigned long output = total;
    
    printf("Result: %lu\n", (unsigned long)output);
    
    return 0;
}
