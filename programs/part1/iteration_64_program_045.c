#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables for pointer aliasing */
volatile int g_var1 = 42;
volatile int g_var2 = 73;
int g_array[256] = {0};

/* Helper functions with specific attributes */
__attribute__((noinline, optimize("O3")))
static int compute_chain(int seed) {
    volatile int barrier = 0;
    int a = seed, b = seed * 2, c = seed + 1;
    int d = 0, e = 0, f = 0, g = 0, h = 0;
    
    /* Long chain of dependent operations */
    for (int i = 0; i < 32; i++) {
        a = a * 1103515245 + 12345;
        b = b ^ (a >> 16);
        c = c + (b & 0xFF);
        d = d * 1664525 + 1013904223;
        e = e ^ d;
        f = f + (e & 0xF);
        g = g * 134775813 + 1;
        h = h ^ g;
        
        /* Memory barrier to split scheduling regions */
        if (i == 16) {
            asm volatile("" : : : "memory");
            barrier = g_var1;
        }
        
        /* Data-dependent exit condition */
        if (__builtin_expect_with_probability((a & 0xFFF) == 0, 0, 0.1)) {
            break;
        }
    }
    
    /* Mix all values */
    return a ^ b ^ c ^ d ^ e ^ f ^ g ^ h ^ barrier;
}

__attribute__((noinline, optimize("O3")))
static int switch_computation(int selector) {
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    /* Complex switch with many cases - creates merge points */
    switch (selector & 0xF) {
        case 0:
            v1 += v2; v3 *= v4; v5 ^= v6;
            v7 = v8 * v9; v10 = v11 + v12;
            break;
        case 1:
            v2 += v3; v4 *= v5; v6 ^= v7;
            v8 = v9 * v10; v11 = v12 + v13;
            break;
        case 2:
            v3 += v4; v5 *= v6; v7 ^= v8;
            v9 = v10 * v11; v12 = v13 + v14;
            break;
        case 3:
            v4 += v5; v6 *= v7; v8 ^= v9;
            v10 = v11 * v12; v13 = v14 + v15;
            break;
        case 4:
            v5 += v6; v7 *= v8; v9 ^= v10;
            v11 = v12 * v13; v14 = v15 + v16;
            break;
        case 5:
            v6 += v7; v8 *= v9; v10 ^= v11;
            v12 = v13 * v14; v15 = v16 + v17;
            break;
        case 6:
            v7 += v8; v9 *= v10; v11 ^= v12;
            v13 = v14 * v15; v16 = v17 + v18;
            break;
        case 7:
            v8 += v9; v10 *= v11; v12 ^= v13;
            v14 = v15 * v16; v17 = v18 + v19;
            break;
        case 8:
            v9 += v10; v11 *= v12; v13 ^= v14;
            v15 = v16 * v17; v18 = v19 + v20;
            break;
        case 9:
            v10 += v11; v12 *= v13; v14 ^= v15;
            v16 = v17 * v18; v19 = v20 + v1;
            break;
        case 10:
            v11 += v12; v13 *= v14; v15 ^= v16;
            v17 = v18 * v19; v20 = v1 + v2;
            break;
        case 11:
            v12 += v13; v14 *= v15; v16 ^= v17;
            v18 = v19 * v20; v1 = v2 + v3;
            break;
        case 12:
            v13 += v14; v15 *= v16; v17 ^= v18;
            v19 = v20 * v1; v2 = v3 + v4;
            break;
        case 13:
            v14 += v15; v16 *= v17; v18 ^= v19;
            v20 = v1 * v2; v3 = v4 + v5;
            break;
        case 14:
            v15 += v16; v17 *= v18; v19 ^= v20;
            v1 = v2 * v3; v4 = v5 + v6;
            break;
        case 15:
            v16 += v17; v18 *= v19; v20 ^= v1;
            v2 = v3 * v4; v5 = v6 + v7;
            break;
    }
    
    /* Combine all variables */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
}

__attribute__((noinline, optimize("O3")))
static int nested_loops(int iterations) {
    int result = 0;
    int i = 0;
    
    /* Outer loop with irregular control flow */
    do {
        int temp = 0;
        
        /* Inner loop with software pipelining pattern */
        for (int j = 0; j < 8; j++) {
            temp = temp * 6364136223846793005ULL + 1442695040888963407ULL;
            
            /* Conditional break creates control flow complexity */
            if (__builtin_expect_with_probability((temp & 0xFF) == 0, 0, 0.05)) {
                asm volatile("" : : : "memory");
                break;
            }
            
            /* Dependent operations */
            result ^= (temp >> 32);
            result += (temp & 0xFFFFFFFF);
        }
        
        i++;
        
        /* Goto creates irregular CFG */
        if ((i & 3) == 0) {
            goto continue_loop;
        }
        
        /* Additional computation */
        result = result * 1103515245 + 12345;
        
        continue_loop:
        /* Empty statement for label */
        ;
        
    } while (i < iterations);
    
    return result;
}

/* Recursive function to create call/return boundaries */
__attribute__((noinline))
static int recursive_compute(int depth, int value) {
    if (depth <= 0) {
        return value;
    }
    
    int local1 = value * 2;
    int local2 = value + 1;
    int local3 = value ^ 0x55AA55AA;
    int local4 = value - 1;
    int local5 = value >> 2;
    
    /* Mix locals before recursion */
    int mixed = local1 ^ local2 ^ local3 ^ local4 ^ local5;
    
    /* Recursive calls with different patterns */
    int r1 = recursive_compute(depth - 1, mixed);
    int r2 = recursive_compute(depth - 2, mixed + 1);
    
    /* Complex computation after recursion */
    for (int i = 0; i < 4; i++) {
        r1 = r1 * 1664525 + 1013904223;
        r2 = r2 * 1103515245 + 12345;
        asm volatile("" : : : "memory");
    }
    
    return r1 ^ r2;
}

__attribute__((optimize("O3")))
int main() {
    int checksum = 0;
    
    /* Initialize global array with pattern */
    for (int i = 0; i < 256; i++) {
        g_array[i] = i * 1103515245 + 12345;
    }
    
    /* Kernel 1: Long chain with data-dependent exit */
    checksum ^= compute_chain(42);
    
    /* Kernel 2: Switch-based computation */
    for (int i = 0; i < 32; i++) {
        checksum += switch_computation(i);
    }
    
    /* Kernel 3: Nested loops with irregular control flow */
    checksum ^= nested_loops(16);
    
    /* Kernel 4: Recursive computation */
    checksum += recursive_compute(4, 100);
    
    /* Kernel 5: Pointer aliasing stress test */
    {
        int* ptr1 = (int*)&g_var1;
        int* ptr2 = (int*)&g_var2;
        volatile int* volatile_ptr = (volatile int*)&g_array[0];
        
        int alias_sum = 0;
        for (int i = 0; i < 64; i++) {
            /* Access through different pointer types */
            alias_sum += *ptr1;
            alias_sum ^= *ptr2;
            alias_sum += volatile_ptr[i & 0xFF];
            
            /* Dependent chain */
            alias_sum = alias_sum * 6364136223846793005ULL + 1442695040888963407ULL;
            
            /* Memory barrier at irregular intervals */
            if ((i & 7) == 0) {
                asm volatile("" : : : "memory");
            }
        }
        checksum ^= alias_sum;
    }
    
    /* Kernel 6: Loop with multiple exit points */
    {
        int loop_result = 0;
        int counter = 0;
        
        restart_loop:
        do {
            for (int j = 0; j < 4; j++) {
                loop_result = loop_result * 1103515245 + 12345;
                
                /* Multiple potential break points */
                if (__builtin_expect_with_probability(
                    (loop_result & 0x3FF) == 0, 0, 0.02)) {
                    break;
                }
                
                if (__builtin_expect_with_probability(
                    (loop_result & 0x1FF) == 0, 0, 0.01)) {
                    goto restart_loop;
                }
            }
            
            counter++;
        } while (counter < 8);
        
        checksum += loop_result;
    }
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
