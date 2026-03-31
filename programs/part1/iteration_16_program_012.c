#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define SIZE 1024
#define ITERATIONS 100

/* Function with complex scheduling patterns */
__attribute__((hot, optimize("O3")))
static float hot_function(float* restrict a, float* restrict b, float* restrict c, int n) {
    float sum = 0.0f;
    
    /* Mixed integer/float operations with dependencies */
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        /* RAW hazard: b depends on a */
        float temp = a[i] * 2.0f;
        
        /* WAR hazard: temp is written then read */
        b[i] = temp + 1.0f;
        
        /* WAW hazard: sum is written multiple times */
        sum += b[i] * c[i];
        
        /* Inline assembly barrier */
        asm volatile("" ::: "memory");
        
        /* More complex dependency chain */
        c[i] = sinf(b[i]) * cosf(temp);
    }
    
    return sum;
}

/* Cold function with different scheduling characteristics */
__attribute__((cold, noinline))
static double cold_function(double* restrict arr, int n) {
    double result = 0.0;
    int* indices = (int*)malloc(n * sizeof(int));
    
    /* Pointer chasing pattern */
    double* ptr = arr;
    for (int i = 0; i < n; i++) {
        indices[i] = i * 2 % n;
    }
    
    /* Complex control flow with scheduling challenges */
    for (int i = 0; i < n; i++) {
        int idx = indices[i];
        
        /* Conditional move vs branch */
        double val = (idx % 3 == 0) ? arr[idx] * 2.0 : 
                    (idx % 3 == 1) ? arr[idx] / 2.0 : 
                    sqrt(fabs(arr[idx]));
        
        /* Assembly with register clobber */
        asm volatile("" : "=r"(val) : "0"(val) : "r0", "r1", "cc");
        
        /* Multiple early exit points */
        if (val > 1000.0) {
            result += 1.0;
            continue;
        }
        
        if (val < -1000.0) {
            result -= 1.0;
            continue;
        }
        
        /* Mixed operations */
        result += val * (i % 5 + 1);
    }
    
    free(indices);
    return result;
}

/* Function with SIMD-friendly operations */
__attribute__((optimize("O3"), optimize("sched-pressure")))
static void vectorized_loop(float* restrict in1, float* restrict in2, 
                           float* restrict out, int n) {
    /* Compile-time known size helps vectorization */
    float local_buf[256];
    
    #pragma GCC unroll 8
    for (int i = 0; i < n && i < 256; i++) {
        /* SIMD-friendly pattern */
        float a = in1[i];
        float b = in2[i];
        
        /* Multiple dependent operations */
        float t1 = a + b;
        float t2 = a - b;
        float t3 = a * b;
        
        /* Scheduling barrier */
        asm volatile("" ::: "memory");
        
        /* More dependencies */
        out[i] = t1 * t2 + t3;
        local_buf[i] = out[i] * 0.5f;
    }
    
    /* Process local buffer */
    for (int i = 0; i < 256; i++) {
        out[i % n] += local_buf[i];
    }
}

/* Function with switch statement for control flow complexity */
__attribute__((noinline))
static int switch_pattern(int x, int* counter) {
    int result = 0;
    
    /* Sparse switch cases */
    switch (x % 13) {
        case 0:
            result = x * 2;
            /* Fall through */
        case 1:
            result += x / 2;
            break;
        case 5:
            result = x * x;
            /* Assembly barrier in middle of switch */
            asm volatile("" ::: "memory");
            break;
        case 7:
            result = x | 0xFF;
            break;
        case 11:
            result = x & 0x0F0F;
            break;
        default:
            result = ~x;
            /* Multiple operations in default case */
            for (int i = 0; i < 4; i++) {
                result ^= (1 << i);
            }
    }
    
    (*counter)++;
    return result;
}

/* Main test function with nested loops */
__attribute__((optimize("O3")))
static double nested_loop_test(void) {
    double arr1[SIZE], arr2[SIZE];
    float farr1[SIZE], farr2[SIZE], fout[SIZE];
    int counters[4] = {0};
    double total = 0.0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = sin(i * 0.01);
        arr2[i] = cos(i * 0.01);
        farr1[i] = (float)arr1[i];
        farr2[i] = (float)arr2[i];
    }
    
    /* Outer loop with multiple inner patterns */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call hot function */
        float fsum = hot_function(farr1, farr2, fout, SIZE);
        total += fsum;
        
        /* Call cold function every few iterations */
        if (iter % 7 == 0) {
            total += cold_function(arr1, SIZE / 4);
        }
        
        /* Vectorized operations */
        vectorized_loop(farr1, farr2, fout, SIZE);
        
        /* Process with switch pattern */
        for (int i = 0; i < SIZE; i += 8) {
            int idx = (i + iter) % SIZE;
            int val = switch_pattern(idx, &counters[idx % 4]);
            total += val;
        }
        
        /* Update arrays to create new dependencies */
        for (int i = 0; i < SIZE; i++) {
            /* Mixed integer/float operations */
            farr1[i] = fout[i] * 0.9f + farr1[i] * 0.1f;
            farr2[i] = fout[(i + 1) % SIZE] * 0.8f + farr2[i] * 0.2f;
            
            /* Integer operations for variety */
            counters[i % 4] ^= (int)farr1[i];
        }
        
        /* Memory barrier between loop iterations */
        asm volatile("" ::: "memory");
    }
    
    return total;
}

/* Additional test with unrolled loops */
__attribute__((optimize("O3")))
static int unrolled_pointer_chase(int* data, int size) {
    int sum = 0;
    int* ptr = data;
    
    /* Manual unrolling with pointer chasing */
    for (int i = 0; i < size - 4; i += 4) {
        /* Multiple dependent loads */
        int a = *ptr;
        ptr = &data[(ptr - data + a) % size];
        
        int b = *ptr;
        ptr = &data[(ptr - data + b) % size];
        
        int c = *ptr;
        ptr = &data[(ptr - data + c) % size];
        
        int d = *ptr;
        ptr = &data[(ptr - data + d) % size];
        
        /* Complex calculation with hazards */
        sum += (a * b) + (c * d);
        sum ^= (a ^ b) | (c ^ d);
        
        /* Scheduling barrier */
        asm volatile("" ::: "memory");
    }
    
    return sum;
}

int main(void) {
    double total_result = 0.0;
    
    printf("Starting selective scheduling stress test...\n");
    
    /* Allocate and initialize test data */
    int* int_data = (int*)malloc(SIZE * sizeof(int));
    for (int i = 0; i < SIZE; i++) {
        int_data[i] = (i * 13 + 7) % 97;
    }
    
    /* Run multiple test patterns */
    for (int test_run = 0; test_run < 3; test_run++) {
        printf("Test run %d\n", test_run + 1);
        
        /* Main nested loop test */
        total_result += nested_loop_test();
        
        /* Pointer chasing test */
        int chase_result = unrolled_pointer_chase(int_data, SIZE);
        total_result += chase_result;
        
        /* Additional mixed workload */
        float temp_arr[SIZE];
        for (int i = 0; i < SIZE; i++) {
            temp_arr[i] = sinf(i * 0.1f) * cosf(i * 0.05f);
        }
        
        /* Process with hot function */
        float fsum = hot_function(temp_arr, temp_arr, temp_arr, SIZE);
        total_result += fsum;
    }
    
    free(int_data);
    
    /* Print result to prevent optimization */
    printf("Total result: %f\n", total_result);
    printf("Test completed.\n");
    
    return (fabs(total_result) > 1e-6) ? 0 : 1;
}
