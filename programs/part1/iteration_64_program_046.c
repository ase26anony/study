#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Global variables to create aliasing and memory dependencies
volatile int g_counter = 0;
int g_array[256] = {0};
int g_result = 0;

// Function attributes to control optimization and inlining
__attribute__((noinline, optimize("O3")))
int complex_chain(int start, int iterations) {
    volatile int memory_barrier;
    int a = start, b = start * 2, c = start * 3;
    int d = start * 4, e = start * 5, f = start * 6;
    
    // Long chain of dependent operations
    for (int i = 0; i < iterations; i++) {
        // Data-dependent exit condition that scheduler can't perfectly analyze
        if (__builtin_expect_with_probability(g_array[i & 255] != 0, 0, 0.3)) {
            // This creates a speculative path
            asm volatile ("" : : : "memory"); // Scheduling barrier
            break;
        }
        
        // Dense computation chain with data dependencies
        a += b ^ c;
        b = c * d + i;
        c = d ^ e ^ f;
        d = e + (f << 2);
        e = f - a * b;
        f = a + b + c + d + e + i;
        
        // Memory operation with uncertain latency
        memory_barrier = g_counter;
        g_array[(i + memory_barrier) & 255] = f;
    }
    
    // Mix results to prevent elimination
    return a ^ b ^ c ^ d ^ e ^ f;
}

__attribute__((noinline, optimize("O3")))
int switch_computation(int selector) {
    // Many local variables to create register pressure
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    
    // Complex switch with many cases - creates merge point challenges
    switch (selector & 0xF) {
        case 0:
            v1 += v2 * v3; v4 ^= v5; v6 = v7 - v8;
            asm volatile ("" : : : "memory");
            break;
        case 1:
            v2 = v3 + v4; v5 *= v6; v7 ^= v8 ^ v9;
            break;
        case 2:
            v3 -= v4 << 2; v5 = v6 + v7; v8 *= v9;
            break;
        case 3:
            v4 ^= v5 ^ v6; v7 += v8; v9 = v10 * v11;
            break;
        case 4:
            v5 = v6 - v7; v8 ^= v9; v10 += v11 * v12;
            break;
        case 5:
            v6 *= v7 + v8; v9 = v10 ^ v11; v12 += v13;
            break;
        case 6:
            v7 += v8 << 1; v9 ^= v10; v11 = v12 * v13;
            break;
        case 7:
            v8 = v9 - v10; v11 *= v12; v13 ^= v14;
            break;
        case 8:
            v9 ^= v10 ^ v11; v12 += v13; v14 = v15 * v16;
            break;
        case 9:
            v10 = v11 + v12; v13 *= v14; v15 ^= v16;
            break;
        case 10:
            v11 -= v12 << 3; v13 = v14 + v15; v16 *= v17;
            break;
        case 11:
            v12 ^= v13 ^ v14; v15 += v16; v17 = v18 * v19;
            break;
        case 12:
            v13 = v14 - v15; v16 ^= v17; v18 += v19 * v20;
            break;
        default:
            v14 *= v15 + v16; v17 = v18 ^ v19; v20 += v1;
            break;
    }
    
    // Complex merge point with many variables
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
}

__attribute__((noinline, optimize("O3")))
int nested_loop_computation(int outer_iters, int inner_iters) {
    int total = 0;
    
    // Outer loop with software pipelining characteristics
    for (int i = 0; i < outer_iters; i++) {
        int local_acc = 0;
        
        // Inner loop with data-dependent operations
        for (int j = 0; j < inner_iters; j++) {
            // Irregular control flow within loop
            if (__builtin_expect_with_probability((i * j) & 0x7F, 0, 0.2)) {
                // goto creating irregular CFG
                if ((i ^ j) & 1) {
                    local_acc += i * j;
                    continue;
                } else {
                    local_acc -= i * j;
                    // Early continue creates control edges
                    if (j & 2) continue;
                }
            }
            
            // Computation with memory side effect
            g_counter = (g_counter + 1) & 255;
            local_acc += g_array[g_counter] ^ j;
            
            // do-while with break
            int k = 0;
            do {
                if (k > 3) break;
                local_acc ^= (i << k);
                k++;
            } while (1);
        }
        
        total += local_acc;
        
        // Memory barrier between loop iterations
        asm volatile ("" : : : "memory");
    }
    
    return total;
}

// Recursive function to create call/return boundaries
__attribute__((noinline))
int recursive_compute(int depth, int value) {
    if (depth <= 0) {
        return value ^ 0x55AA55AA;
    }
    
    // Each recursion level does different computation
    int new_val;
    switch (depth & 3) {
        case 0: new_val = value * 3 + 1; break;
        case 1: new_val = value ^ (value << 8); break;
        case 2: new_val = value + depth * 7; break;
        default: new_val = value - depth * 11; break;
    }
    
    // Recursive call - scheduler may save/restore state around this
    int result = recursive_compute(depth - 1, new_val);
    
    // Post-recursion computation
    return result ^ (depth * 0x12345678);
}

__attribute__((optimize("O3")))
int main() {
    // Initialize array with pattern
    for (int i = 0; i < 256; i++) {
        g_array[i] = i * 1103515245 + 12345;
    }
    
    int checksum = 0;
    
    // Kernel 1: Long chain with speculative breaks
    checksum ^= complex_chain(42, 1000);
    
    // Kernel 2: Switch-based computation with many variables
    for (int i = 0; i < 50; i++) {
        checksum += switch_computation(checksum + i);
    }
    
    // Kernel 3: Nested loops with irregular control flow
    checksum ^= nested_loop_computation(100, 50);
    
    // Kernel 4: Recursive computation
    checksum += recursive_compute(4, checksum);
    
    // Final mixing to ensure all computations are used
    volatile int sink = checksum;
    for (int i = 0; i < 100; i++) {
        // One more complex pattern with goto
        int val = i * 3;
        computation_label:
        val = (val * 13 + 7) & 0xFFFF;
        if (__builtin_expect_with_probability(val < 10000, 1, 0.7)) {
            checksum ^= val;
            if (i & 1) goto computation_label; // Backward jump
        }
    }
    
    printf("Result: %d\n", checksum);
    return 0;
}
