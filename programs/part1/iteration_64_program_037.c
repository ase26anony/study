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
    volatile int barrier;
    int a = seed, b = seed * 2, c = seed + 1;
    int d = 0, e = 0, f = 0, g = 0, h = 0;
    
    /* Long chain of dependent operations */
    a += g_var1;
    b ^= a;
    c *= b + 1;
    d = c - a;
    e = d * 3;
    f = e ^ b;
    g = f + c;
    h = g * 2;
    
    /* Memory barrier to split scheduling regions */
    asm volatile("" : : : "memory");
    
    /* More dependent operations */
    a += h;
    b ^= g;
    c *= f;
    d += e;
    e ^= d;
    f = g_var2 + e;
    g = f * c;
    h = g ^ a;
    
    /* Another barrier */
    barrier = g_var1;
    (void)barrier;
    
    return a + b + c + d + e + f + g + h;
}

__attribute__((noinline))
__attribute__((optimize("O3")))
int loop_with_break(int *arr, int n) {
    int sum = 0;
    int i = 0;
    
    /* Loop with data-dependent exit condition */
    do {
        if (__builtin_expect_with_probability(arr[i] != 0, 1, 0.7)) {
            sum += compute_chain(arr[i]);
            i++;
        } else {
            /* Complex break path that might trigger state save */
            sum += g_var2;
            break;
        }
        
        /* Additional computation to create scheduling pressure */
        int tmp = sum;
        tmp ^= arr[i-1];
        tmp *= 3;
        tmp += g_var1;
        sum = tmp;
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
        
    } while (i < n && __builtin_expect_with_probability(i % 7 != 0, 1, 0.8));
    
    return sum;
}

__attribute__((noinline))
__attribute__((optimize("O3")))
int recursive_compute(int depth, int val) {
    if (depth <= 0) {
        return val;
    }
    
    /* Multiple local variables for register pressure */
    int a = val, b = val + 1, c = val * 2;
    int d = 0, e = 0, f = 0, g = 0, h = 0;
    
    /* Dependent operations before recursion */
    a += g_var1;
    b ^= a;
    c *= b;
    
    /* Recursive call - scheduler may save state here */
    int rec = recursive_compute(depth - 1, c);
    
    /* More operations after recursion */
    d = rec + a;
    e = d ^ b;
    f = e * c;
    g = f + rec;
    h = g ^ d;
    
    /* Memory access with volatile */
    volatile int mem = g_var2;
    g += mem;
    
    return a + b + c + d + e + f + g + h + rec;
}

__attribute__((noinline))
__attribute__((optimize("O3")))
int complex_switch(int selector) {
    /* Many local variables to create register pressure */
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0;
    int r6 = 0, r7 = 0, r8 = 0, r9 = 0, r10 = 0;
    int r11 = 0, r12 = 0, r13 = 0, r14 = 0, r15 = 0;
    
    /* Switch with many cases - each creates different scheduling pattern */
    switch (selector % 12) {
        case 0:
            r1 = g_var1 * 2;
            r2 = r1 ^ selector;
            r3 = r2 + g_var2;
            asm volatile("" : : : "memory");
            break;
        case 1:
            r4 = selector + g_var1;
            r5 = r4 * 3;
            r6 = r5 ^ g_var2;
            break;
        case 2:
            r7 = g_var2 - selector;
            r8 = r7 * 5;
            r9 = r8 ^ g_var1;
            break;
        case 3:
            r10 = selector * selector;
            r11 = r10 + g_var1;
            r12 = r11 ^ g_var2;
            asm volatile("" : : : "memory");
            break;
        case 4:
            r13 = g_var1 + g_var2;
            r14 = r13 * selector;
            r15 = r14 ^ 0xABCD;
            break;
        case 5:
            r1 = selector << 2;
            r3 = r1 | g_var1;
            r5 = r3 ^ selector;
            break;
        case 6:
            r2 = g_var2 >> 1;
            r4 = r2 + selector;
            r6 = r4 * 7;
            asm volatile("" : : : "memory");
            break;
        case 7:
            r7 = selector % 13;
            r8 = r7 + g_var1;
            r9 = r8 * g_var2;
            break;
        case 8:
            r10 = g_var1 ^ g_var2;
            r11 = r10 + selector;
            r12 = r11 * 11;
            break;
        case 9:
            r13 = selector * 3;
            r14 = r13 ^ g_var1;
            r15 = r14 + g_var2;
            asm volatile("" : : : "memory");
            break;
        case 10:
            r1 = g_var2 - g_var1;
            r2 = r1 * selector;
            r3 = r2 ^ 0x1234;
            break;
        case 11:
            r4 = selector + 100;
            r5 = r4 ^ g_var2;
            r6 = r5 * g_var1;
            break;
        default:
            r7 = selector;
            break;
    }
    
    /* Merge point with many live variables */
    return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + 
           r11 + r12 + r13 + r14 + r15;
}

__attribute__((noinline))
__attribute__((optimize("O3")))
int nested_loop_pipeline(int outer_iters) {
    int total = 0;
    
    for (int i = 0; i < outer_iters; i++) {
        /* Software pipelining style computation */
        int a = i, b = i * 2, c = i + 1;
        int d = 0, e = 0;
        
        /* Inner loop with independent-ish iterations */
        for (int j = 0; j < 8; j++) {
            a += j;
            b ^= a;
            c *= b + 1;
            
            /* Memory operation in the middle */
            d = g_array[(i + j) & 0xFF];
            e += d;
            
            /* More computation */
            a ^= e;
            b += c;
            c *= d + 1;
        }
        
        /* Cross-iteration dependency */
        g_array[i & 0xFF] = a + b + c + d + e;
        total += g_array[i & 0xFF];
        
        /* Scheduling barrier every few iterations */
        if (__builtin_expect_with_probability((i & 3) == 0, 0, 0.6)) {
            asm volatile("" : : : "memory");
        }
    }
    
    return total;
}

int main() {
    int result = 0;
    
    /* Initialize array with non-zero values */
    for (int i = 0; i < 256; i++) {
        g_array[i] = (i * 37 + 123) & 0xFF;
    }
    
    /* Kernel 1: Loop with data-dependent break */
    result += loop_with_break(g_array, 100);
    
    /* Kernel 2: Complex switch statement */
    for (int i = 0; i < 50; i++) {
        result ^= complex_switch(result + i);
    }
    
    /* Kernel 3: Recursive computation */
    result += recursive_compute(4, result);
    
    /* Kernel 4: Nested loop for software pipelining */
    result += nested_loop_pipeline(64);
    
    /* Kernel 5: Mixed pattern with goto for irregular CFG */
    {
        int x = result;
        int y = 0;
        
    restart_point:
        for (int i = 0; i < 16; i++) {
            x += compute_chain(i);
            y ^= x;
            
            /* Conditional goto creating irregular control flow */
            if (__builtin_expect_with_probability((x & 0xF) == 0, 0, 0.3)) {
                x += g_var1;
                goto restart_point;
            }
            
            /* Memory operation */
            volatile int tmp = g_var2;
            y += tmp;
        }
        result += x + y;
    }
    
    /* Final checksum to prevent elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
