/* haifa-sched-trigger.c
 * Program designed to trigger haifa scheduler state save/restore logic
 * Compile with: gcc -O3 -fschedule-insns2 -fno-guess-branch-probability -fno-inline -fsel-sched-pipelining -o trigger haifa-sched-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int g_counter = 0;
int g_array[256];
int g_results[16];

/* Function prototypes */
__attribute__((noinline)) long compute_chain(int seed, int iterations);
__attribute__((noinline)) int switch_computation(int mode, int value);
__attribute__((noinline)) void nested_loop_operation(int *data, int size);
__attribute__((optimize("O3"))) int recursive_compute(int depth, int value);
__attribute__((noinline)) void memory_barrier(void);

/* Memory barrier to split scheduling regions */
void memory_barrier(void) {
    asm volatile("" : : : "memory");
}

/* Long chain of dependent operations with data-dependent exit */
__attribute__((noinline)) 
long compute_chain(int seed, int iterations) {
    volatile int *volatile_ptr = &g_counter;
    long a = seed * 6364136223846793005ULL;
    long b = seed ^ 0xDEADBEEF;
    long c = 1;
    int i = 0;
    
    /* Create register pressure with many locals */
    int r1 = seed, r2 = seed + 1, r3 = seed + 2, r4 = seed + 3;
    int r5 = seed + 4, r6 = seed + 5, r7 = seed + 6, r8 = seed + 7;
    int r9 = seed + 8, r10 = seed + 9, r11 = seed + 10, r12 = seed + 11;
    int r13 = seed + 12, r14 = seed + 13, r15 = seed + 14, r16 = seed + 15;
    
    /* Loop with data-dependent exit condition */
    while (i < iterations) {
        /* Data-dependent break - scheduler may need to save state */
        if (__builtin_expect_with_probability(g_array[i] == 0, 0, 0.3)) {
            break;
        }
        
        /* Long chain of dependent operations */
        a = (a * 1103515245 + 12345) & 0x7FFFFFFF;
        b = b ^ (b << 13);
        b = b ^ (b >> 17);
        b = b ^ (b << 5);
        c = c + (a & 0xFF) * (b & 0xFF);
        
        /* Mix in all the register variables */
        r1 += r2; r3 ^= r4; r5 *= r6; r7 -= r8;
        r9 |= r10; r11 &= r12; r13 <<= 1; r14 >>= 1;
        r15 = r15 ^ r16; r16 = r16 + r1;
        
        /* Memory operation with volatile */
        *volatile_ptr = i;
        
        /* Scheduling barrier */
        memory_barrier();
        
        /* More dependent operations */
        a = a ^ c;
        b = b + r1;
        c = c * 16807 % 2147483647;
        
        i++;
    }
    
    /* Complex return computation */
    return (a ^ b ^ c) + r1 + r3 + r5 + r7 + r9 + r11 + r13 + r15;
}

/* Switch with many cases - creates complex control flow */
__attribute__((noinline))
int switch_computation(int mode, int value) {
    int result = value;
    
    /* Many local variables for register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    
    /* Switch with many cases - scheduler may save state at merge point */
    switch (mode % 12) {
        case 0:
            result = v1 * v2 + v3 - v4;
            v5 = result ^ v6;
            break;
        case 1:
            result = v2 << v3;
            v7 = v8 & v9;
            break;
        case 2:
            result = v4 | v5;
            v10 = v11 * v12;
            break;
        case 3:
            result = v6 ^ v7;
            v13 = v14 - v15;
            break;
        case 4:
            result = v8 + v9 + v10;
            v1 = v2 * v3;
            break;
        case 5:
            result = v11 & v12;
            v4 = v5 | v6;
            break;
        case 6:
            result = v13 << 3;
            v7 = v8 >> 2;
            break;
        case 7:
            result = v14 * v15;
            v9 = v10 ^ v11;
            break;
        case 8:
            result = v1 + v2 + v3 + v4;
            v12 = v13 & v14;
            break;
        case 9:
            result = v5 - v6 - v7;
            v15 = v1 * v2;
            break;
        case 10:
            result = v8 | v9 | v10;
            v3 = v4 ^ v5;
            break;
        case 11:
            result = v11 << v12;
            v6 = v7 + v8;
            break;
        default:
            /* Unreachable but creates control flow edge */
            goto switch_end;
    }
    
switch_end:
    /* Merge point - scheduler may restore state here */
    return result + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Nested loops with function calls */
__attribute__((noinline))
void nested_loop_operation(int *data, int size) {
    int i, j;
    
    /* Outer loop */
    for (i = 0; i < size; i++) {
        volatile int *ptr = &g_counter;
        
        /* Inner loop with data dependency */
        for (j = 0; j < 8; j++) {
            /* Data-dependent continue */
            if (__builtin_expect_with_probability((data[i] & (1 << j)) == 0, 0, 0.2)) {
                continue;
            }
            
            /* Memory operation */
            *ptr = i * j;
            
            /* Inline asm barrier */
            asm volatile("" : : : "memory");
            
            /* Computation */
            data[i] = data[i] ^ (j * 0x01010101);
            
            /* Another barrier */
            memory_barrier();
        }
        
        /* Irregular control flow with goto */
        if (data[i] > 1000) {
            goto adjust_value;
        }
        continue;
        
    adjust_value:
        data[i] = data[i] % 256;
    }
}

/* Recursive function with arithmetic */
__attribute__((optimize("O3")))
int recursive_compute(int depth, int value) {
    if (depth <= 0) {
        return value;
    }
    
    /* Local variables for register pressure */
    int a = value * 2;
    int b = value + 1;
    int c = value ^ 0x55;
    int d = value << 3;
    int e = value >> 2;
    
    /* Memory operation */
    volatile int *vp = &g_counter;
    *vp = depth;
    
    /* Recursive calls create call/return boundaries */
    int r1 = recursive_compute(depth - 1, a);
    int r2 = recursive_compute(depth - 1, b);
    
    /* Scheduling barrier between recursive calls */
    memory_barrier();
    
    int r3 = recursive_compute(depth - 2, c);
    
    /* Complex computation with all results */
    return (r1 ^ r2 ^ r3) + a + b + c + d + e;
}

/* Main orchestrator */
int main(void) {
    long total = 0;
    int i;
    
    /* Initialize global array with non-zero values */
    for (i = 0; i < 256; i++) {
        g_array[i] = (i * 37) & 0xFF;
    }
    
    /* Put some zeros to trigger data-dependent breaks */
    g_array[50] = 0;
    g_array[100] = 0;
    g_array[150] = 0;
    
    printf("Starting scheduler stress test...\n");
    
    /* Kernel 1: Long chain with data-dependent break */
    total += compute_chain(42, 200);
    
    /* Kernel 2: Switch with many cases */
    for (i = 0; i < 100; i++) {
        total += switch_computation(i, i * 3);
    }
    
    /* Kernel 3: Nested loops */
    for (i = 0; i < 16; i++) {
        g_results[i] = i * 100;
    }
    nested_loop_operation(g_results, 16);
    for (i = 0; i < 16; i++) {
        total += g_results[i];
    }
    
    /* Kernel 4: Recursive computation */
    total += recursive_compute(4, 123);
    
    /* Kernel 5: Manual software pipelining style */
    {
        int pipe_a[8] = {1, 2, 3, 4, 5, 6, 7, 8};
        int pipe_b[8] = {0};
        int pipe_c[8] = {0};
        
        /* Do-while with break inside */
        i = 0;
        do {
            if (__builtin_expect_with_probability(pipe_a[i] > 5, 0, 0.4)) {
                pipe_b[i] = pipe_a[i] * 2;
                /* Internal break in do-while */
                if (pipe_b[i] > 10) break;
            } else {
                pipe_b[i] = pipe_a[i] + 10;
            }
            
            /* Second stage */
            pipe_c[i] = pipe_b[i] ^ 0xFF;
            
            /* Memory clobber */
            asm volatile("" : : : "memory");
            
            i++;
        } while (i < 8);
        
        for (i = 0; i < 8; i++) {
            total += pipe_c[i];
        }
    }
    
    /* Final checksum */
    printf("Result checksum: %ld\n", total);
    
    /* Use result to prevent elimination */
    if (total == 0) {
        printf("Unexpected zero result\n");
    }
    
    return (int)(total & 0x7FFFFFFF);
}
