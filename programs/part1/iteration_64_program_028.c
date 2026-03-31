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
    int d = 0, e = 0, f = 0;
    
    /* Long chain of dependent operations */
    for (int i = 0; i < 32; i++) {
        a += b ^ c;
        b = c * a + i;
        c = (a << 3) | (b >> 2);
        d ^= a + b;
        e += c - d;
        f = (f * 3) + e;
        
        /* Memory barrier to split scheduling regions */
        if (i == 16) {
            asm volatile("" : : : "memory");
            barrier = g_var1;
        }
        
        /* Data-dependent exit to encourage speculative scheduling */
        if (__builtin_expect_with_probability(f & 0x100, 0, 0.3)) {
            break;
        }
    }
    
    /* Another computational chain */
    int result = a + b + c + d + e + f;
    for (int j = 0; j < 8; j++) {
        result = (result * 1103515245 + 12345) & 0x7fffffff;
        result ^= result >> 13;
    }
    
    return result;
}

__attribute__((noinline))
__attribute__((optimize("O3")))
int nested_loop_helper(int* arr, int n) {
    int sum = 0;
    int temp1 = 0, temp2 = 0, temp3 = 0;
    
    /* Inner loop with software pipelining potential */
    for (int i = 0; i < n; i++) {
        temp1 = arr[i] * 3;
        temp2 = temp1 + i;
        temp3 = temp2 ^ temp1;
        sum += temp3;
        
        /* Volatile read to create scheduling uncertainty */
        if (__builtin_expect((i & 7) == 0, 0)) {
            volatile int v = g_var2;
            temp1 += v;
        }
    }
    
    /* Irregular control flow */
    do {
        if (sum > 1000) {
            sum >>= 1;
            break;
        }
        sum *= 2;
    } while (0);
    
    return sum;
}

__attribute__((noinline))
__attribute__((optimize("O3")))
int recursive_compute(int depth, int val) {
    if (depth <= 0) {
        return val;
    }
    
    int local1 = val * 2;
    int local2 = val + depth;
    int local3 = val ^ 0xABCD;
    
    /* Create register pressure with many variables */
    int t1 = local1, t2 = local2, t3 = local3;
    int t4 = t1 + t2, t5 = t2 * t3, t6 = t3 ^ t1;
    int t7 = t4 - t5, t8 = t5 | t6, t9 = t6 & t4;
    int t10 = t7 * t8, t11 = t8 + t9, t12 = t9 - t7;
    
    /* Mix computations */
    int result = t10 + t11 + t12;
    result = recursive_compute(depth - 1, result);
    
    /* More operations after recursion */
    result = (result * 3) ^ 0x1234;
    return result;
}

__attribute__((noinline))
__attribute__((optimize("O3")))
int complex_switch(int selector) {
    /* Many local variables to create register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    /* Switch with many cases - each modifies different variables */
    switch (selector & 0xF) {
        case 0:
            v1 += v2; v3 *= v4; v5 ^= v6;
            asm volatile("" : : : "memory");
            break;
        case 1:
            v2 -= v3; v4 |= v5; v6 &= v7;
            break;
        case 2:
            v7 <<= 2; v8 >>= 1; v9 = v10 ^ v11;
            break;
        case 3:
            v12 = v13 + v14; v15 = v16 * v17;
            break;
        case 4:
            v18 = v19 - v20; v1 = v2 | v3;
            break;
        case 5:
            v4 &= v5; v6 ^= v7; v8 += v9;
            break;
        case 6:
            v10 *= v11; v12 -= v13; v14 |= v15;
            break;
        case 7:
            v16 <<= 3; v17 >>= 2; v18 = v19 ^ v20;
            break;
        case 8:
            v1 = v2 + v3 + v4; v5 = v6 * v7;
            break;
        case 9:
            v8 = v9 - v10; v11 = v12 | v13;
            break;
        case 10:
            v14 &= v15; v16 ^= v17; v18 += v19;
            break;
        case 11:
            v20 *= v1; v2 -= v3; v4 |= v5;
            break;
        case 12:
            v6 <<= 1; v7 >>= 3; v8 = v9 ^ v10;
            break;
        case 13:
            v11 = v12 + v13 + v14; v15 = v16 * v17;
            break;
        case 14:
            v18 = v19 - v20; v1 = v2 | v3 | v4;
            break;
        case 15:
            v5 &= v6; v7 ^= v8; v9 += v10 * v11;
            break;
    }
    
    /* Merge point with complex computation */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    
    /* Loop with goto creating irregular CFG */
    int i = 0;
    compute_loop:
    result += i * i;
    i++;
    if (i < 5) {
        if (__builtin_expect_with_probability(result & 1, 0, 0.4)) {
            goto compute_loop;
        }
    }
    
    return result;
}

int main() {
    int checksum = 0;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 256; i++) {
        g_array[i] = (i * 3) ^ 0x55;
    }
    
    /* Kernel 1: Long chain with data-dependent break */
    for (int iter = 0; iter < 100; iter++) {
        checksum ^= compute_chain(iter + checksum);
        
        /* Pointer aliasing to limit scheduler freedom */
        int* alias1 = (int*)&g_var1;
        int* alias2 = (int*)((char*)&g_var1 + 0);
        *alias1 += 1;
        checksum += *alias2;
    }
    
    /* Kernel 2: Nested loop structure */
    for (int block = 0; block < 50; block++) {
        int start = (block * 5) % 256;
        checksum += nested_loop_helper(&g_array[start], 8);
    }
    
    /* Kernel 3: Recursive computation */
    checksum += recursive_compute(4, checksum);
    
    /* Kernel 4: Complex switch with many cases */
    for (int s = 0; s < 32; s++) {
        checksum ^= complex_switch(s + checksum);
    }
    
    /* Kernel 5: Manual software pipelining pattern */
    int pipe_a = 1, pipe_b = 2, pipe_c = 3;
    for (int stage = 0; stage < 100; stage++) {
        /* Stage 1 computation */
        pipe_a = pipe_b * 3 + stage;
        
        /* Memory operation between stages */
        volatile int mem = g_array[stage & 0xFF];
        
        /* Stage 2 computation */
        pipe_b = pipe_c ^ pipe_a;
        
        /* Another memory barrier */
        asm volatile("" : : : "memory");
        
        /* Stage 3 computation */
        pipe_c = pipe_a + pipe_b + mem;
        
        checksum += pipe_c;
        
        /* Irregular control flow within pipeline */
        if (__builtin_expect_with_probability((stage & 0x1F) == 0, 0, 0.2)) {
            pipe_a = recursive_compute(2, pipe_a);
        }
    }
    
    /* Final mixing */
    checksum = (checksum * 0x5BD1E995) ^ (checksum >> 16);
    
    printf("Result checksum: %d\n", checksum);
    return 0;
}
