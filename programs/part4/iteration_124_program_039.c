/* sel-sched-test.c - Test program to trigger selective scheduler debug dumping */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper function to prevent optimization */
static int __attribute__((noinline)) helper_compute(int a, int b) {
    return a * b + (a ^ b);
}

/* Function with mixed operations to generate diverse RTL */
static int __attribute__((noinline, optimize("O2"))) 
test_mixed_operations(int* arr, int n) {
    int sum = 0;
    float fsum = 0.0f;
    double dsum = 0.0;
    
    for (int i = 0; i < n; i++) {
        /* Integer arithmetic with data dependency */
        int val = arr[i];
        sum += val * i;
        
        /* Floating point operations */
        fsum += (float)val * 1.5f;
        dsum += (double)val * 2.5;
        
        /* Conditional move/ternary operation */
        int cond_val = (val > 100) ? val : 100;
        sum += cond_val;
        
        /* Built-in function for complex RTL */
        sum += __builtin_popcount(val);
        
        /* Memory access pattern */
        if (i % 2 == 0) {
            arr[i] = sum % 256;
        } else {
            arr[i] = (sum * i) % 256;
        }
        
        /* Scheduling barrier */
        asm volatile("" : : : "memory");
        
        /* Function call to generate call RTL */
        sum += helper_compute(val, i);
    }
    
    /* Mix results to prevent dead code elimination */
    return sum + (int)fsum + (int)dsum;
}

/* Function with nested loops for outer loop pipelining */
static int64_t __attribute__((noinline, optimize("O3")))
test_nested_loops(int32_t* matrix, int rows, int cols) {
    int64_t total = 0;
    
    for (int i = 0; i < rows; i++) {
        int32_t row_sum = 0;
        for (int j = 0; j < cols; j++) {
            /* Complex addressing with multiple operations */
            int32_t elem = matrix[i * cols + j];
            
            /* Mixed-width operations */
            int64_t wide_val = (int64_t)elem * j;
            row_sum += (int32_t)(wide_val & 0xFFFFFFFF);
            
            /* Bit manipulation operations */
            elem = (elem << 3) | (elem >> 29);  /* Rotate left by 3 */
            elem ^= (i * j);  /* XOR with position */
            
            /* Conditional store */
            matrix[i * cols + j] = (elem > 0) ? elem : -elem;
            
            /* Another scheduling barrier */
            asm volatile("" : : : "memory");
        }
        total += row_sum;
        
        /* Control flow divergence */
        if (i % 3 == 0) {
            total += __builtin_ctz(i + 1);  /* Count trailing zeros */
        } else if (i % 3 == 1) {
            total -= __builtin_clz(i);      /* Count leading zeros */
        }
    }
    
    return total;
}

/* Function with pointer chasing to create memory dependencies */
static int __attribute__((noinline, target("arch=haswell")))
test_pointer_chasing(int* data, int* indices, int n) {
    int result = 0;
    int* current = data;
    
    for (int i = 0; i < n; i++) {
        /* Load from memory with complex addressing */
        int idx = indices[i] % n;
        int value = data[idx];
        
        /* Multiple dependent operations */
        result = result * 31 + value;
        result ^= (result >> 16);
        
        /* Floating point conversion and back */
        float temp = (float)result;
        result = (int)(temp * 1.618f);  /* Golden ratio */
        
        /* Volatile read to prevent reordering */
        volatile int barrier = indices[i];
        (void)barrier;
        
        /* Conditional update with side effect */
        current = (value > result) ? &data[idx] : current;
        if (current) {
            *current = result;
        }
    }
    
    return result;
}

/* Main test driver */
int main(int argc, char** argv) {
    const int SIZE = 1024;
    const int MATRIX_SIZE = 64;
    
    /* Allocate and initialize test data */
    int* array = (int*)malloc(SIZE * sizeof(int));
    int32_t* matrix = (int32_t*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int32_t));
    int* indices = (int*)malloc(SIZE * sizeof(int));
    
    if (!array || !matrix || !indices) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE; i++) {
        array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        indices[i] = (i * 1664525 + 1013904223) % SIZE;
    }
    
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        matrix[i] = (i * 48271) % 1000;
    }
    
    /* Run all test functions */
    int result1 = test_mixed_operations(array, SIZE);
    printf("Test 1 result: %d\n", result1);
    
    int64_t result2 = test_nested_loops(matrix, MATRIX_SIZE, MATRIX_SIZE);
    printf("Test 2 result: %lld\n", (long long)result2);
    
    int result3 = test_pointer_chasing(array, indices, SIZE);
    printf("Test 3 result: %d\n", result3);
    
    /* Final checksum to verify computation */
    int final_sum = result1 + (int)(result2 & 0xFFFFFFFF) + result3;
    printf("Final checksum: %d\n", final_sum);
    
    /* Cleanup */
    free(array);
    free(matrix);
    free(indices);
    
    return 0;
}
