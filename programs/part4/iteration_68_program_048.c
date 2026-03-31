/* sel_sched_trigger.c - Program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_bound = 1000;
volatile int g_volatile_seed = 42;

/* Function with complex control flow for scheduling */
static inline int complex_condition(int x, int y) {
    /* Multiple basic blocks with different operations */
    if (x > y) {
        return (x * y) / (y != 0 ? y : 1);
    } else if (x < y) {
        return (y - x) * (x % 7);
    } else {
        return (x ^ y) | (x & 0xFF);
    }
}

/* Mixed-width operations to create register pressure */
static inline uint64_t mixed_width_ops(uint32_t a, uint64_t b, int c) {
    uint64_t result = 0;
    
    /* 32-bit and 64-bit mixed operations */
    result = ((uint64_t)a * b) + (c * 0x12345678);
    result = result / ((c != 0) ? c : 1);
    result = result ^ (a << 3);
    result = result | (b & 0xFFFFFFFF);
    
    return result;
}

/* Pointer chasing pattern with data dependencies */
static float pointer_chase(float* data, int size, int stride) {
    float sum = 0.0f;
    volatile int v_size = size; /* Prevent optimization */
    
    for (int i = 1; i < v_size; i++) {
        /* Data-dependent computation with carried dependency */
        sum += data[i] * data[i - 1];
        
        /* Complex addressing mode */
        int idx = (i * stride) % size;
        sum -= data[idx] * 0.5f;
        
        /* Conditional operation */
        sum = (data[i] > 0.0f) ? sum + data[i] : sum - data[i];
    }
    
    return sum;
}

/* Matrix-vector multiplication kernel */
static void matvec_multiply(float* result, float** matrix, float* vector, 
                           int rows, int cols) {
    volatile int v_rows = rows;
    volatile int v_cols = cols;
    
    #pragma GCC unroll 4
    for (int i = 0; i < v_rows; i++) {
        float sum = 0.0f;
        
        #pragma GCC unroll 2
        for (int j = 0; j < v_cols; j++) {
            /* Complex computation with multiple operations */
            sum += matrix[i][j] * vector[j];
            
            /* Inline assembly to create fixed RTL patterns */
            asm volatile ("" : : : "memory");
            
            /* Additional floating-point operation */
            sum = sum / ((j % 8) + 1);
        }
        
        /* Conditional store */
        result[i] = (sum > 0.0f) ? sum : -sum;
    }
}

/* Main computation with nested loops */
static uint64_t compute_kernel(int* data, int size) {
    uint64_t acc = 0;
    volatile int v_size = size;
    
    /* Outer loop */
    for (int i = 0; i < v_size - 1; i++) {
        uint64_t inner_acc = 0;
        
        /* Inner loop with carried dependency */
        for (int j = 1; j < g_volatile_bound; j++) {
            /* Data-dependent computation */
            int val = data[(i + j) % size];
            
            /* Complex integer operations */
            inner_acc += val * data[(i + j - 1) % size];
            inner_acc = inner_acc ^ (val << (j % 16));
            
            /* Division with non-constant divisor */
            inner_acc = inner_acc / ((val % 16) + 1);
            
            /* Mixed-width operation */
            inner_acc = mixed_width_ops(val, inner_acc, j);
            
            /* Switch statement for control flow complexity */
            switch (val % 5) {
                case 0:
                    inner_acc += complex_condition(val, j);
                    break;
                case 1:
                    inner_acc -= val * j;
                    break;
                case 2:
                    inner_acc |= 0xAAAAAAAA;
                    break;
                case 3:
                    inner_acc = (inner_acc >> 3) | (inner_acc << 61);
                    break;
                default:
                    inner_acc ^= 0x55555555;
                    break;
            }
            
            /* Another inline assembly barrier */
            asm volatile ("" : : : "memory");
        }
        
        acc ^= inner_acc;
    }
    
    return acc;
}

/* Second computation kernel with different pattern */
static double second_kernel(double* array, int n) {
    double prod = 1.0;
    volatile int v_n = n;
    
    for (int i = 0; i < v_n; i++) {
        /* Strided memory access */
        int stride = (i * 3) % n;
        
        /* Floating-point operations */
        double val = array[stride];
        prod *= val;
        
        /* Conditional with expensive operation */
        if (val != 0.0) {
            prod /= (val > 0.0 ? val : -val);
        }
        
        /* More complex FP math */
        prod = prod + sin(val * 0.01) * cos(val * 0.02);
        
        /* Prevent optimization with external call */
        prod += (rand() % 100) * 0.001;
    }
    
    return prod;
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int MATRIX_SIZE = 64;
    
    /* Initialize with pseudo-random data */
    srand(g_volatile_seed);
    
    /* Allocate and initialize arrays */
    int* int_data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float* float_data = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double* double_data = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float** matrix = (float**)malloc(MATRIX_SIZE * sizeof(float*));
    float* vector = (float*)malloc(MATRIX_SIZE * sizeof(float));
    float* result = (float*)malloc(MATRIX_SIZE * sizeof(float));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_data[i] = rand() % 1000;
        float_data[i] = (rand() % 1000) / 100.0f;
        double_data[i] = (rand() % 1000) / 100.0;
    }
    
    for (int i = 0; i < MATRIX_SIZE; i++) {
        matrix[i] = (float*)malloc(MATRIX_SIZE * sizeof(float));
        vector[i] = (rand() % 100) / 10.0f;
        for (int j = 0; j < MATRIX_SIZE; j++) {
            matrix[i][j] = (rand() % 100) / 10.0f;
        }
    }
    
    /* Perform computations that should trigger selective scheduling */
    uint64_t result1 = compute_kernel(int_data, ARRAY_SIZE);
    float result2 = pointer_chase(float_data, ARRAY_SIZE, 7);
    double result3 = second_kernel(double_data, ARRAY_SIZE);
    matvec_multiply(result, matrix, vector, MATRIX_SIZE, MATRIX_SIZE);
    
    /* Final reduction to prevent optimization */
    uint64_t final_result = result1;
    final_result ^= *(uint64_t*)&result2;
    final_result ^= *(uint64_t*)&result3;
    
    for (int i = 0; i < MATRIX_SIZE; i++) {
        final_result += (uint64_t)(result[i] * 1000);
    }
    
    printf("Final result: %lu\n", final_result);
    
    /* Cleanup */
    for (int i = 0; i < MATRIX_SIZE; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(vector);
    free(result);
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}
