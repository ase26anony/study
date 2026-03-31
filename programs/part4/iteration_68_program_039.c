/* sel_sched_trigger.c - Program to trigger selective scheduler debug dumping */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_bound = 1000;
volatile int g_volatile_seed = 42;

/* Mixed-width operations to create register pressure */
typedef struct {
    int32_t a;
    int64_t b;
    float c;
    double d;
} MixedData;

/* Complex computation with data dependencies */
static inline int64_t complex_op(int32_t a, int64_t b, float c, double d) {
    /* Mixed-width arithmetic with dependencies */
    int64_t t1 = (int64_t)a * b;
    double t2 = (double)t1 * d;
    float t3 = c * (float)t2;
    
    /* Conditional move via ternary */
    int64_t result = (t2 > 0.0) ? (int64_t)(t3 * 100.0) : -(int64_t)(t3 * 50.0);
    
    /* Non-constant division to prevent optimization */
    if (result != 0) {
        result /= (abs(a % 7) + 1);
    }
    
    return result;
}

/* Pointer chasing pattern */
int64_t pointer_chase(MixedData* array, int size, int stride) {
    int64_t sum = 0;
    volatile int v_idx = 0;  /* Volatile to prevent optimization */
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent index calculation */
        int idx = (i * stride) % size;
        v_idx = idx;  /* Force memory write */
        
        /* Complex addressing mode */
        MixedData* ptr = &array[idx];
        
        /* Carry dependency across iterations */
        sum = sum + complex_op(ptr->a, ptr->b, ptr->c, ptr->d);
        
        /* Inline assembly to create fixed RTL */
        asm volatile ("" : : "r"(ptr) : "memory");
    }
    
    return sum;
}

/* Nested loop with control flow */
double nested_loop_compute(double* matrix, int rows, int cols) {
    double total = 0.0;
    volatile int outer_bound = rows;
    
    for (int i = 0; i < outer_bound; i++) {
        double row_sum = 0.0;
        
        /* Inner loop with multiple basic blocks */
        #pragma GCC unroll 4
        for (int j = 0; j < cols; j++) {
            double val = matrix[i * cols + j];
            
            /* Complex conditional with both branches having computation */
            if (val > 0.0) {
                /* Branch 1: Positive values */
                row_sum += val * val;
                
                /* Non-trivial division */
                if (j > 0) {
                    row_sum /= (1.0 + matrix[i * cols + j - 1]);
                }
            } else {
                /* Branch 2: Non-positive values */
                row_sum -= val * val;
                
                /* Different computation path */
                if (j % 3 == 0) {
                    row_sum *= 0.5;
                }
            }
            
            /* Switch statement for additional control flow */
            switch (j % 4) {
                case 0:
                    row_sum += 1.0;
                    break;
                case 1:
                    row_sum -= 0.5;
                    break;
                case 2:
                    row_sum *= 1.1;
                    break;
                case 3:
                    row_sum /= 1.05;
                    break;
            }
        }
        
        total += row_sum;
        
        /* External dependency to prevent optimization */
        if (rand() % 100 == 0) {
            total *= 0.99;
        }
    }
    
    return total;
}

/* Matrix-vector multiplication for additional scheduling regions */
void matvec_multiply(float* result, float* matrix, float* vector, 
                     int rows, int cols) {
    volatile int r = rows;
    
    for (int i = 0; i < r; i++) {
        float sum = 0.0f;
        
        /* Unrolled inner loop with mixed operations */
        #pragma GCC unroll 2
        for (int j = 0; j < cols; j++) {
            float m_val = matrix[i * cols + j];
            float v_val = vector[j];
            
            /* Fused multiply-add pattern */
            sum += m_val * v_val;
            
            /* Conditional operation */
            sum = (sum > 1000.0f) ? sum * 0.9f : sum * 1.1f;
            
            /* Memory barrier via inline assembly */
            asm volatile ("" : : "r"(&sum) : "memory");
        }
        
        result[i] = sum;
    }
}

int main() {
    const int ARRAY_SIZE = 1024;
    const int MATRIX_ROWS = 128;
    const int MATRIX_COLS = 64;
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    
    /* Allocate and initialize mixed data array */
    MixedData* data_array = (MixedData*)malloc(ARRAY_SIZE * sizeof(MixedData));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data_array[i].a = rand() % 1000;
        data_array[i].b = rand() % 10000;
        data_array[i].c = (float)rand() / RAND_MAX;
        data_array[i].d = (double)rand() / RAND_MAX;
    }
    
    /* Allocate matrix data */
    double* matrix = (double*)malloc(MATRIX_ROWS * MATRIX_COLS * sizeof(double));
    float* f_matrix = (float*)malloc(MATRIX_ROWS * MATRIX_COLS * sizeof(float));
    float* vector = (float*)malloc(MATRIX_COLS * sizeof(float));
    float* result = (float*)malloc(MATRIX_ROWS * sizeof(float));
    
    for (int i = 0; i < MATRIX_ROWS * MATRIX_COLS; i++) {
        matrix[i] = (double)rand() / RAND_MAX - 0.5;
        f_matrix[i] = (float)rand() / RAND_MAX;
    }
    
    for (int i = 0; i < MATRIX_COLS; i++) {
        vector[i] = (float)rand() / RAND_MAX;
    }
    
    /* Volatile loop bound to prevent optimization */
    volatile int iterations = g_volatile_bound;
    int64_t final_sum = 0;
    
    /* Main computation with multiple scheduling regions */
    for (int iter = 0; iter < iterations; iter++) {
        /* First scheduling region: pointer chasing */
        int64_t chase_result = pointer_chase(data_array, ARRAY_SIZE, 
                                            (iter % 7) + 1);
        
        /* Second scheduling region: nested loops with control flow */
        double matrix_result = nested_loop_compute(matrix, 
                                                  MATRIX_ROWS, 
                                                  MATRIX_COLS);
        
        /* Third scheduling region: matrix-vector multiplication */
        matvec_multiply(result, f_matrix, vector, MATRIX_ROWS, MATRIX_COLS);
        
        /* Combine results with non-trivial operation */
        final_sum ^= (int64_t)chase_result ^ (int64_t)matrix_result;
        
        /* Modify data slightly for next iteration */
        if (iter % 10 == 0) {
            data_array[iter % ARRAY_SIZE].a += 1;
            matrix[iter % (MATRIX_ROWS * MATRIX_COLS)] *= 1.01;
        }
    }
    
    /* Final reduction to ensure side effects */
    double final_reduction = 0.0;
    for (int i = 0; i < MATRIX_ROWS; i++) {
        final_reduction += result[i];
    }
    
    /* Print results to prevent dead code elimination */
    printf("Final sum: %ld\n", (long)final_sum);
    printf("Final reduction: %f\n", final_reduction);
    
    /* Cleanup */
    free(data_array);
    free(matrix);
    free(f_matrix);
    free(vector);
    free(result);
    
    return 0;
}
