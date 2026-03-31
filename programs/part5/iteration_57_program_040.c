#include <stdio.h>
#include <stdlib.h>

// Global variables to create loop-invariant values
int global_invariant1 = 7;
int global_invariant2 = 13;
int global_invariant3 = 19;

// Function with high register pressure and loop-carried dependencies
__attribute__((hot, noinline))
int hot_loop_function(int* arr1, int* arr2, int* arr3, int size) {
    int result = 0;
    
    // Main loop with high iteration count
    for (int i = 2; i < size - 2; i++) {
        // Declare many scalar temporaries to create register pressure
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        int t10, t11, t12, t13, t14, t15, t16, t17, t18, t19;
        
        // Start dependency chain with array accesses using index and offsets
        t0 = arr1[i] + global_invariant1;  // Use invariant value
        t1 = t0 * arr2[i-1];
        t2 = t1 - arr3[i+1];
        t3 = t2 / global_invariant2;       // Division with invariant (higher latency)
        
        // Continue dependency chain
        t4 = t3 + arr1[i-2];
        t5 = t4 * arr2[i];
        t6 = t5 - arr3[i-1];
        t7 = t6 / global_invariant3;       // Another division
        
        // More operations in the chain
        t8 = t7 + arr1[i+1];
        t9 = t8 * arr2[i+1];
        t10 = t9 - arr3[i-2];
        t11 = t10 % global_invariant1;     // Modulo operation (potentially higher latency)
        
        // Additional chain segment
        t12 = t11 + arr1[i];
        t13 = t12 * arr2[i-2];
        t14 = t13 - arr3[i];
        t15 = t14 / 5;                     // Constant division
        
        // Create conditional basic block split
        if (t15 & 1) {
            // Path 1: More computations
            t16 = t15 * 3;
            t17 = t16 + arr1[i+2];
            t18 = t17 - global_invariant2;
            t19 = t18 % 7;
            result += t19;
        } else {
            // Path 2: Different computations (still using dependency chain)
            t16 = t15 / 2;
            t17 = t16 + arr1[i-1];
            t18 = t17 * global_invariant3;
            t19 = t18 - 11;
            result += t19;
        }
        
        // Use the result in a way that prevents dead code elimination
        arr1[i] = (arr1[i] + t19) & 0xFF;
    }
    
    return result;
}

// Non-inlineable helper with higher latency operations
__attribute__((noinline, cold))
int higher_latency_op(int a, int b) {
    // Force higher latency by using division and modulo
    return (a / (b | 1)) + (a % (b | 1));
}

// Another hot function with different pattern
__attribute__((hot, noinline))
int hot_loop_function2(int* arr1, int* arr2, int* arr3, int size) {
    int result = 0;
    
    for (int i = 1; i < size - 1; i++) {
        // Another set of temporaries
        int u0, u1, u2, u3, u4, u5, u6, u7, u8, u9;
        
        // Chain with function call (higher latency)
        u0 = arr1[i] + global_invariant2;
        u1 = u0 * arr2[i];
        u2 = higher_latency_op(u1, global_invariant1);  // Function call
        
        u3 = u2 - arr3[i];
        u4 = u3 * arr1[i-1];
        u5 = higher_latency_op(u4, global_invariant3);  // Another call
        
        u6 = u5 + arr2[i+1];
        u7 = u6 - arr3[i-1];
        u8 = u7 % global_invariant2;  // Modulo
        
        // Conditional with both paths having computations
        if (u8 > 0) {
            u9 = u8 * 2 + arr1[i];
        } else {
            u9 = u8 / 2 - arr2[i];
        }
        
        result += u9;
        arr2[i] = (arr2[i] + u9) & 0xFF;
    }
    
    return result;
}

// Simple deterministic PRNG for array initialization
static unsigned int seed = 12345;
static inline unsigned int lcg_rand() {
    seed = seed * 1103515245 + 12345;
    return seed;
}

int main() {
    const int SIZE = 1024;
    
    // Allocate and initialize arrays with deterministic values
    int* arr1 = (int*)malloc(SIZE * sizeof(int));
    int* arr2 = (int*)malloc(SIZE * sizeof(int));
    int* arr3 = (int*)malloc(SIZE * sizeof(int));
    
    // Initialize with pseudo-random but deterministic values
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = lcg_rand() % 1000;
        arr2[i] = lcg_rand() % 1000;
        arr3[i] = lcg_rand() % 1000;
    }
    
    int total_result = 0;
    
    // Call hot functions multiple times to ensure they're compiled
    // and executed, not optimized away
    for (int iter = 0; iter < 10; iter++) {
        total_result += hot_loop_function(arr1, arr2, arr3, SIZE);
        total_result += hot_loop_function2(arr1, arr2, arr3, SIZE);
        
        // Modify arrays slightly between iterations
        for (int i = 0; i < SIZE; i++) {
            arr1[i] = (arr1[i] + 1) & 0xFFF;
        }
    }
    
    // Print result to prevent dead code elimination
    printf("Total result: %d\n", total_result);
    
    // Cleanup
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
