#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int g_var1 = 42;
volatile int g_var2 = 73;
int g_array[256] = {0};

/* Helper functions marked noinline to prevent inlining */
__attribute__((noinline, optimize("O3")))
int complex_chain(int seed) {
    volatile int barrier;
    int a = seed, b = seed * 2, c = seed + 1;
    int d = 0, e = 0, f = 0, g = 0, h = 0;
    
    /* Long chain of dependent operations */
    for (int i = 0; i < 32; i++) {
        a += b ^ c;
        b *= a | 0x5A5A5A5A;
        c ^= b + a;
        d = c - b;
        e = d * a;
        f = e ^ d;
        g = f + c;
        h = g * b;
        
        /* Memory barrier to split scheduling regions */
        asm volatile("" : : : "memory");
        
        /* Data-dependent condition with probability hint */
        if (__builtin_expect_with_probability((a & 0xFF) > 128, 0, 0.7)) {
            barrier = g_var1;
            a ^= barrier;
        }
    }
    
    /* Mix results */
    return a + b + c + d + e + f + g + h;
}

__attribute__((noinline, optimize("O3")))
int recursive_compute(int n, int depth) {
    if (depth >= 4) return n;
    
    int local_vars[8] = {n, n*2, n*3, n*4, n*5, n*6, n*7, n*8};
    
    /* Complex computation with many variables */
    for (int i = 0; i < 8; i++) {
        local_vars[i] += local_vars[(i+1)%8] ^ local_vars[(i+2)%8];
        local_vars[i] *= local_vars[(i+3)%8] | 0x12345678;
    }
    
    /* Recursive call */
    int result = recursive_compute(local_vars[n % 8], depth + 1);
    
    /* More computation after recursion */
    for (int i = 0; i < 8; i++) {
        local_vars[i] ^= result;
    }
    
    return local_vars[0] + local_vars[1] + local_vars[2];
}

__attribute__((noinline, optimize("O3")))
int switch_complex(int selector) {
    /* Many local variables to create register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    /* Switch with many cases - each modifies different variables */
    switch (selector % 12) {
        case 0:
            v1 += v2 * v3;
            v4 ^= v5;
            v6 = v7 | v8;
            break;
        case 1:
            v2 = v3 - v4;
            v5 *= v6;
            v7 ^= v8 & v9;
            break;
        case 2:
            v3 += v4 * v5;
            v6 = v7 ^ v8;
            v9 |= v10;
            break;
        case 3:
            v4 = v5 + v6;
            v7 *= v8;
            v10 ^= v11;
            break;
        case 4:
            v5 += v6 - v7;
            v8 = v9 * v10;
            v11 |= v12;
            break;
        case 5:
            v6 ^= v7;
            v8 += v9;
            v10 *= v11;
            break;
        case 6:
            v7 = v8 | v9;
            v10 += v11;
            v12 ^= v13;
            break;
        case 7:
            v8 *= v9;
            v10 = v11 + v12;
            v13 |= v14;
            break;
        case 8:
            v9 += v10;
            v11 ^= v12;
            v13 = v14 * v15;
            break;
        case 9:
            v10 = v11 | v12;
            v13 += v14;
            v15 ^= v16;
            break;
        case 10:
            v11 *= v12;
            v13 = v14 + v15;
            v16 |= v17;
            break;
        case 11:
            v12 += v13;
            v14 ^= v15;
            v16 = v17 * v18;
            break;
    }
    
    /* Combine all variables */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
}

__attribute__((noinline, optimize("O3")))
int loop_with_irregular_control(int iterations) {
    int result = 0;
    int i = 0;
    
restart_loop:
    while (i < iterations) {
        /* Data-dependent exit condition */
        if (g_array[i % 256] != 0) {
            result += complex_chain(i);
            i++;
            continue;
        }
        
        /* Irregular control flow with goto */
        if (__builtin_expect_with_probability((i & 0x3F) == 0, 0, 0.3)) {
            /* Memory operation with uncertain latency */
            volatile int mem_read = g_var2;
            result ^= mem_read;
            goto restart_loop;
        }
        
        /* Chain of operations */
        int temp = i;
        temp += temp * 3;
        temp ^= temp >> 4;
        temp *= 0x9E3779B9;
        temp += temp << 5;
        temp ^= temp >> 13;
        
        result += temp;
        
        /* Another scheduling barrier */
        asm volatile("" : : : "memory");
        
        i++;
    }
    
    return result;
}

__attribute__((noinline, optimize("O3")))
int nested_loop_pipelining(int outer_iters, int inner_iters) {
    int sum = 0;
    
    for (int i = 0; i < outer_iters; i++) {
        /* Outer loop computation */
        int outer_acc = i;
        outer_acc = (outer_acc * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Inner loop with independent-ish iterations */
        for (int j = 0; j < inner_iters; j++) {
            /* Software pipelining candidate */
            int a = outer_acc + j;
            int b = a * j;
            int c = b ^ a;
            int d = c + outer_acc;
            
            /* Mix with globals via pointer aliasing */
            int* p1 = (int*)&g_var1;
            int* p2 = (int*)&g_var2;
            d ^= *p1;
            d += *p2;
            
            sum += d;
            
            /* Break in do-while to create control edges */
            do {
                if (__builtin_expect_with_probability((d & 0xFF) == 0, 0, 0.1)) {
                    sum ^= 0xABCD;
                    break;
                }
                sum += 1;
            } while (0);
        }
        
        /* Call to function with different optimization level */
        sum += switch_complex(i);
    }
    
    return sum;
}

int main() {
    /* Initialize array with pattern */
    for (int i = 0; i < 256; i++) {
        g_array[i] = (i * 97) & 0xFF;
    }
    
    int total = 0;
    
    /* Execute different scheduling stress patterns */
    
    /* 1. Complex chain with memory barriers */
    total += complex_chain(12345);
    
    /* 2. Switch with many cases and variables */
    for (int i = 0; i < 100; i++) {
        total += switch_complex(i);
    }
    
    /* 3. Loop with irregular control flow */
    total += loop_with_irregular_control(500);
    
    /* 4. Nested loops for pipelining */
    total += nested_loop_pipelining(50, 20);
    
    /* 5. Recursive computation */
    total += recursive_compute(100, 0);
    
    /* Additional mixed pattern */
    for (int i = 0; i < 50; i++) {
        /* Alternate between different patterns */
        if (i & 1) {
            total ^= complex_chain(i);
        } else {
            total += switch_complex(total & 0xF);
        }
        
        /* Occasionally trigger irregular path */
        if (__builtin_expect_with_probability((i % 13) == 0, 0, 0.2)) {
            volatile int temp = g_var1;
            total *= (temp & 0xF) + 1;
        }
    }
    
    printf("Result checksum: %d\n", total);
    return 0;
}
