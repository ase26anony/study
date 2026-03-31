#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int global_a = 42;
volatile int global_b = 73;
int global_c = 101;
int global_d = 255;

/* Helper functions marked noinline to prevent optimization */
__attribute__((noinline, optimize("O3")))
int complex_chain(int seed) {
    volatile int barrier;
    int a = seed, b = seed * 2, c = seed + 1, d = seed - 1;
    int e = a ^ b, f = c | d, g = a & c, h = b ^ d;
    
    /* Long chain of dependent operations */
    for (int i = 0; i < 32; i++) {
        a += (b * c) ^ d;
        b ^= (a << i) | (c >> (31 - i));
        c += (d * a) - b;
        d ^= (c + b) * a;
        
        /* Memory barrier to split scheduling regions */
        if (i == 16) {
            asm volatile("" : : : "memory");
            barrier = global_a;
        }
        
        /* Data-dependent branch with probability hint */
        if (__builtin_expect_with_probability((a & 0xFF) > 128, 0, 0.7)) {
            e += f * g;
            f ^= h << 2;
        } else {
            g += h * e;
            h ^= f >> 1;
        }
    }
    
    /* Mix in volatile read */
    barrier = global_b;
    return a + b + c + d + e + f + g + h + barrier;
}

__attribute__((noinline, optimize("O3")))
int switch_computation(int selector) {
    /* Many local variables to create register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8;
    int v9 = 9, v10 = 10, v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    /* Complex switch with many cases - scheduler may save state */
    switch (selector & 0xF) {
        case 0:
            v1 += v2 * v3; v4 ^= v5; v6 += v7 - v8;
            v9 = v10 * v11; v12 ^= v13; v14 += v15;
            break;
        case 1:
            v2 += v3 * v4; v5 ^= v6; v7 += v8 - v9;
            v10 = v11 * v12; v13 ^= v14; v15 += v16;
            break;
        case 2:
            v3 += v4 * v5; v6 ^= v7; v8 += v9 - v10;
            v11 = v12 * v13; v14 ^= v15; v16 += v17;
            break;
        case 3:
            v4 += v5 * v6; v7 ^= v8; v9 += v10 - v11;
            v12 = v13 * v14; v15 ^= v16; v17 += v18;
            break;
        case 4:
            v5 += v6 * v7; v8 ^= v9; v10 += v11 - v12;
            v13 = v14 * v15; v16 ^= v17; v18 += v19;
            break;
        case 5:
            v6 += v7 * v8; v9 ^= v10; v11 += v12 - v13;
            v14 = v15 * v16; v17 ^= v18; v19 += v20;
            break;
        case 6:
            v7 += v8 * v9; v10 ^= v11; v12 += v13 - v14;
            v15 = v16 * v17; v18 ^= v19; v20 += v1;
            break;
        case 7:
            v8 += v9 * v10; v11 ^= v12; v13 += v14 - v15;
            v16 = v17 * v18; v19 ^= v20; v1 += v2;
            break;
        case 8:
            v9 += v10 * v11; v12 ^= v13; v14 += v15 - v16;
            v17 = v18 * v19; v20 ^= v1; v2 += v3;
            break;
        case 9:
            v10 += v11 * v12; v13 ^= v14; v15 += v16 - v17;
            v18 = v19 * v20; v1 ^= v2; v3 += v4;
            break;
        case 10:
            v11 += v12 * v13; v14 ^= v15; v16 += v17 - v18;
            v19 = v20 * v1; v2 ^= v3; v4 += v5;
            break;
        case 11:
            v12 += v13 * v14; v15 ^= v16; v17 += v18 - v19;
            v20 = v1 * v2; v3 ^= v4; v5 += v6;
            break;
        default:
            v13 += v14 * v15; v16 ^= v17; v18 += v19 - v20;
            v1 = v2 * v3; v4 ^= v5; v6 += v7;
            break;
    }
    
    /* Merge all variables */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
}

__attribute__((noinline, optimize("O3")))
int nested_loops(int iterations) {
    int result = 0;
    
    /* Outer loop with software pipelining characteristics */
    for (int i = 0; i < iterations; i++) {
        int a = i, b = i * 2, c = i + 1;
        
        /* Inner loop with data-dependent exit condition */
        int j = 0;
        do {
            if (__builtin_expect_with_probability(j > 8, 0, 0.3)) {
                /* Early exit path - scheduler may save state */
                a ^= b;
                b += c;
                c *= a;
                break;
            }
            
            a += (b * c) ^ (j + 1);
            b ^= (a << (j & 3)) | (c >> (4 - (j & 3)));
            c += (a - b) * (j + 1);
            
            /* Irregular control flow with goto */
            if ((a & 3) == 0) {
                goto continue_inner;
            }
            
            a ^= 0xAAAAAAAA;
            b += 0x55555555;
            
        continue_inner:
            j++;
        } while (j < 10);
        
        result += a + b + c;
        
        /* Memory operation with uncertain latency */
        asm volatile("" : : : "memory");
        global_c = i;
    }
    
    return result;
}

/* Recursive function to create return state restoration points */
__attribute__((noinline, optimize("O3")))
int recursive_compute(int depth, int value) {
    if (depth <= 0) {
        return value ^ 0x12345678;
    }
    
    int a = value * 3;
    int b = value + depth;
    int c = value ^ (depth * 7);
    
    /* Mix with volatile read */
    volatile int mem = global_d;
    a += mem;
    
    /* Recursive call - scheduler may save state for continuation */
    int recurse_result = recursive_compute(depth - 1, a + b + c);
    
    /* Post-recursion computation */
    b ^= recurse_result;
    c += recurse_result * 2;
    a = (a * b) ^ c;
    
    return a + b + c;
}

__attribute__((optimize("O3")))
int main() {
    uint64_t checksum = 0;
    
    printf("Starting scheduler stress test...\n");
    
    /* Kernel 1: Long dependent chain with probability hints */
    for (int i = 0; i < 100; i++) {
        int result = complex_chain(i);
        checksum = (checksum * 31) + result;
        
        /* Data-dependent loop exit */
        if (__builtin_expect_with_probability(result > 1000000, 0, 0.1)) {
            break;
        }
    }
    
    /* Kernel 2: Switch statement with many cases */
    for (int i = 0; i < 50; i++) {
        int result = switch_computation(i);
        checksum = (checksum * 127) + result;
    }
    
    /* Kernel 3: Nested loops with irregular control flow */
    int loop_result = nested_loops(20);
    checksum = (checksum * 19) + loop_result;
    
    /* Kernel 4: Recursive computation */
    int recurse_result = recursive_compute(4, 42);
    checksum = (checksum * 7) + recurse_result;
    
    /* Final mixing */
    checksum ^= (uint64_t)global_a;
    checksum ^= (uint64_t)global_b << 16;
    checksum ^= (uint64_t)global_c << 32;
    checksum ^= (uint64_t)global_d << 48;
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    return (int)(checksum & 0xFFFFFFFF);
}
