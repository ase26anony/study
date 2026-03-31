#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int global_a = 42;
volatile int global_b = 73;
int global_array[256];

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
        c ^= b + global_a;  /* Memory access creates uncertainty */
        
        /* Data-dependent exit condition */
        if (a > 1000000) {
            /* Scheduling barrier */
            asm volatile("" : : : "memory");
            break;
        }
        
        /* More dependencies */
        d = a + b;
        e = c - d;
        f = d * e;
        g = f ^ e;
        h = g + a;
        
        /* Branch with probability hint */
        if (__builtin_expect_with_probability((h & 0xFF) > 128, 0, 0.7)) {
            a += global_b;
        }
    }
    
    /* Memory clobber affecting scheduling */
    barrier = a;
    asm volatile("" : : "r"(barrier) : "memory");
    
    return a + b + c + d + e + f + g + h;
}

__attribute__((noinline))
__attribute__((optimize("O3")))
int nested_loop_helper(int* arr, int n) {
    int sum = 0;
    int temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;
    
    /* Short inner loop with multiple variables */
    for (int i = 0; i < 8; i++) {
        temp1 += arr[i % n];
        temp2 ^= temp1;
        temp3 = temp2 * (i + 1);
        temp4 += temp3 >> 2;
        
        /* Scheduling barrier in the middle */
        if (i == 4) {
            asm volatile("" : : : "memory");
        }
    }
    
    /* Irregular control flow */
    do {
        if (temp1 > temp2) {
            sum += temp1;
            break;
        }
        sum += temp2;
        
        if (temp3 < temp4) {
            sum += temp3;
            if (sum > 1000) break;
        }
    } while (0);
    
    return sum + temp4;
}

__attribute__((noinline))
__attribute__((optimize("O3")))
int complex_switch(int selector, int* results) {
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0;
    int r6 = 0, r7 = 0, r8 = 0, r9 = 0, r10 = 0;
    int r11 = 0, r12 = 0, r13 = 0, r14 = 0, r15 = 0;
    
    /* Switch with many cases - creates complex control flow */
    switch (selector % 12) {
        case 0:
            r1 = global_a * 2;
            r2 = r1 + global_b;
            r3 = r2 ^ 0xAAAA;
            break;
        case 1:
            r4 = global_b - global_a;
            r5 = r4 * 3;
            r6 = r5 >> 1;
            break;
        case 2:
            r7 = compute_chain(selector);
            r8 = r7 & 0xFF;
            break;
        case 3:
            r9 = selector * selector;
            r10 = r9 % 17;
            break;
        case 4:
            r11 = global_a ^ global_b;
            r12 = r11 | 0x5555;
            break;
        case 5:
            r13 = selector + 1;
            r14 = r13 * 2;
            r15 = r14 - 3;
            break;
        case 6:
            r1 = selector * 3;
            r3 = r1 + 5;
            r5 = r3 * 7;
            break;
        case 7:
            r2 = global_a + 10;
            r4 = r2 * global_b;
            r6 = r4 / 2;
            break;
        case 8:
            r7 = selector << 2;
            r8 = r7 | 0xF0;
            r9 = r8 ^ 0xAA;
            break;
        case 9:
            r10 = global_b * 4;
            r11 = r10 - 8;
            r12 = r11 + selector;
            break;
        case 10:
            r13 = compute_chain(selector + 100);
            r14 = r13 & 0x7F;
            r15 = r14 * 3;
            break;
        case 11:
            r1 = selector % 13;
            r2 = r1 * 11;
            r3 = r2 + global_a;
            break;
    }
    
    /* Merge point with many live variables */
    results[0] = r1 + r3 + r5 + r7 + r9 + r11 + r13 + r15;
    results[1] = r2 + r4 + r6 + r8 + r10 + r12 + r14;
    
    return results[0] + results[1];
}

/* Recursive function with depth */
__attribute__((noinline))
int recursive_compute(int n, int depth) {
    if (depth >= 4) return n;
    
    int a = n * 2;
    int b = a + global_a;
    int c = b ^ global_b;
    
    /* Recursive calls create return points for state restoration */
    int r1 = recursive_compute(a, depth + 1);
    int r2 = recursive_compute(b, depth + 1);
    int r3 = recursive_compute(c, depth + 1);
    
    /* Complex merge computation */
    volatile int mem_barrier = r1;
    asm volatile("" : : "r"(mem_barrier) : "memory");
    
    return (r1 + r2 + r3) * (depth + 1);
}

int main() {
    int result = 0;
    int switch_results[2];
    
    /* Initialize global array with non-zero values */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i + 1;
    }
    
    /* Kernel 1: Long chain with data-dependent exit */
    result += compute_chain(1);
    result += compute_chain(100);
    result += compute_chain(1000);
    
    /* Kernel 2: Nested loops with helper function */
    for (int i = 0; i < 50; i++) {
        /* Data-dependent loop condition */
        int limit = global_array[i % 256] & 0xF;
        for (int j = 0; j < limit; j++) {
            result += nested_loop_helper(global_array, 256);
        }
        
        /* Branch probability hint */
        if (__builtin_expect_with_probability((i & 0x3) == 0, 1, 0.3)) {
            result += i * 2;
        }
    }
    
    /* Kernel 3: Complex switch statement */
    for (int i = 0; i < 100; i++) {
        result += complex_switch(i + result, switch_results);
        
        /* Goto creating irregular control flow */
        if ((i % 13) == 0) {
            goto special_case;
        }
        continue;
        
    special_case:
        result += recursive_compute(i, 0);
    }
    
    /* Kernel 4: Manual software pipelining style */
    int pipe_a = 0, pipe_b = 0, pipe_c = 0;
    for (int i = 0; i < 100; i++) {
        /* Independent operations that could be pipelined */
        int stage1 = pipe_a + global_array[i % 256];
        int stage2 = pipe_b * stage1;
        int stage3 = pipe_c ^ stage2;
        
        /* Rotate pipeline registers */
        pipe_a = stage1;
        pipe_b = stage2;
        pipe_c = stage3;
        
        /* Scheduling barrier every 25 iterations */
        if ((i % 25) == 0) {
            asm volatile("" : : : "memory");
        }
    }
    result += pipe_a + pipe_b + pipe_c;
    
    /* Final checksum */
    printf("Result checksum: %d\n", result);
    
    /* Access globals to prevent elimination */
    volatile int final_check = global_a + global_b;
    (void)final_check;
    
    return 0;
}
