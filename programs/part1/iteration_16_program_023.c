/* sel-sched-test.c - Comprehensive test to trigger selective scheduling debug output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 100

/* Function attributes to influence scheduling */
__attribute__((hot, optimize("O3", "unroll-loops")))
static float hot_loop_vectorized(float *restrict a, float *restrict b, 
                                 float *restrict c, int n) {
    float sum = 0.0f;
    
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        // Complex data dependencies with RAW hazards
        float t1 = a[i] * b[i];
        float t2 = t1 + c[i];      // RAW on t1
        a[i] = t2 * 0.5f;          // WAR on a[i]
        float t3 = sinf(t2);       // RAW on t2
        b[i] = cosf(t3);           // RAW on t3, WAR on b[i]
        sum += a[i] + b[i] + t3;   // Multiple RAW hazards
    }
    
    // Memory barrier to split scheduling regions
    asm volatile("" ::: "memory");
    
    return sum;
}

__attribute__((cold, noinline))
static int cold_complex_control_flow(int *arr, int n) {
    int result = 0;
    
    // Mixed control flow with scheduling challenges
    for (int i = 0; i < n; i++) {
        // Multiple early exit points
        if (i % 17 == 0 && i > n/2) {
            continue;
        }
        
        // Switch with sparse cases
        switch (arr[i] % 13) {
            case 0: result += arr[i] * 2; break;
            case 1: result -= arr[i]; break;
            case 5: result ^= arr[i]; break;
            case 8: result |= arr[i] << 2; break;
            default: result = (result * 3) / 2; break;
        }
        
        // Conditional move mixed with computation
        int temp = (arr[i] > 0) ? arr[i] : -arr[i];
        result += (temp % 7 == 0) ? temp * 2 : temp / 2;
        
        // Another memory barrier with register clobber
        asm volatile("" ::: "memory", "r0", "r1", "r2", "r3");
    }
    
    return result;
}

__attribute__((optimize("sched-pressure")))
static double pointer_chasing_hazards(double **ptrs, int n) {
    double total = 0.0;
    double *current = ptrs[0];
    
    // Pointer chasing with WAW and WAR hazards
    for (int i = 0; i < n; i++) {
        double val1 = *current;
        double val2 = val1 * 1.61803398875;  // RAW on val1
        *current = val2;                     // WAW on *current
        double val3 = sqrt(val2);            // RAW on val2
        current = ptrs[(int)val3 % n];       // Control dependency
        
        // Mixed integer/floating point operations
        int idx = (int)val3 % 256;
        total += val3 * idx + (double)(idx % 16);
        
        // Inline asm with specific constraints
        asm volatile(
            "/* Scheduling barrier %0 */"
            : "+r" (idx)
            : "r" (val3)
            : "cc", "memory"
        );
    }
    
    return total;
}

__attribute__((optimize("O3"), noinline))
static void nested_loop_scheduling(int size) {
    int matrix[32][32];
    
    // Nested loops with complex dependencies
    for (int i = 0; i < size; i++) {
        #pragma GCC unroll 2
        for (int j = 0; j < size; j++) {
            // Cross-iteration dependencies
            int prev_i = (i > 0) ? matrix[i-1][j] : 0;
            int prev_j = (j > 0) ? matrix[i][j-1] : 0;
            
            // WAW hazard on matrix[i][j]
            matrix[i][j] = i * j;
            matrix[i][j] = prev_i + prev_j;  // WAW
            
            // Complex expression with multiple operations
            matrix[i][j] += (matrix[i][j] % 3 == 0) ? 
                           (i << 2) | (j & 0xF) : 
                           (i * 3) / (j + 1);
        }
        
        // Memory barrier every few iterations
        if (i % 8 == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    // Prevent dead code elimination
    volatile int sink = matrix[0][0];
    (void)sink;
}

__attribute__((optimize("O3", "tree-vectorize")))
static float simd_mixed_operations(float *a, float *b, int n) {
    float sum1 = 0.0f, sum2 = 0.0f, sum3 = 0.0f;
    
    // SIMD-friendly loop with mixed operations
    #pragma GCC unroll 8
    for (int i = 0; i < n; i++) {
        // Independent chains with different latencies
        float chain1 = a[i] * 1.1f;
        chain1 = chain1 + b[i];          // RAW
        chain1 = sinf(chain1);           // RAW
        
        float chain2 = b[i] * 0.9f;
        chain2 = chain2 - a[i];          // RAW
        chain2 = cosf(chain2);           // RAW
        
        float chain3 = a[i] + b[i];
        chain3 = chain3 * chain3;        // RAW
        chain3 = sqrtf(fabsf(chain3));   // RAW
        
        // Cross-chain dependencies
        sum1 += chain1 * chain2;         // RAW on chain1, chain2
        sum2 += chain2 * chain3;         // RAW on chain2, chain3
        sum3 += chain3 * chain1;         // RAW on chain3, chain1
        
        // Write back creating WAR hazards
        a[i] = chain1 + chain2;
        b[i] = chain2 + chain3;          // WAR on chain2
    }
    
    return sum1 + sum2 + sum3;
}

/* Main test driver */
int main(void) {
    // Initialize test data
    float fa[SIZE], fb[SIZE], fc[SIZE];
    int int_arr[SIZE];
    double *double_ptrs[64];
    double double_data[64];
    
    srand(time(NULL));
    
    for (int i = 0; i < SIZE; i++) {
        fa[i] = (float)rand() / RAND_MAX;
        fb[i] = (float)rand() / RAND_MAX;
        fc[i] = (float)rand() / RAND_MAX;
        int_arr[i] = rand() % 1000;
    }
    
    for (int i = 0; i < 64; i++) {
        double_data[i] = (double)rand() / RAND_MAX;
        double_ptrs[i] = &double_data[i];
    }
    
    float total_float = 0.0f;
    int total_int = 0;
    double total_double = 0.0;
    
    // Execute all test functions multiple times
    for (int iter = 0; iter < ITERATIONS; iter++) {
        // Call hot vectorized function
        total_float += hot_loop_vectorized(fa, fb, fc, SIZE);
        
        // Call cold function with complex control flow
        total_int += cold_complex_control_flow(int_arr, SIZE);
        
        // Call pointer chasing function
        total_double += pointer_chasing_hazards(double_ptrs, 64);
        
        // Call nested loop function
        nested_loop_scheduling(32);
        
        // Call SIMD mixed operations function
        total_float += simd_mixed_operations(fa, fb, SIZE);
        
        // Modify data slightly each iteration
        for (int i = 0; i < SIZE; i++) {
            fa[i] += 0.001f;
            fb[i] -= 0.001f;
            int_arr[i] = (int_arr[i] + 1) % 1000;
        }
    }
    
    // Print results to prevent optimization
    printf("Results: float=%f int=%d double=%f\n", 
           total_float, total_int, total_double);
    
    return 0;
}
