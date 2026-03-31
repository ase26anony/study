/* sel-sched-trigger.c
 * Program designed to trigger GCC selective scheduler debug dumping
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-sched1 -fdump-rtl-sched2 sel-sched-trigger.c -o sel-sched-trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int volatile_bound = 1000;
volatile int volatile_seed = 42;

/* Mixed-width data types for register pressure */
typedef struct {
    int32_t a;
    int64_t b;
    float c;
    double d;
} MixedData;

/* Simple PRNG to create data dependencies */
static inline uint32_t prng(uint32_t *state) {
    *state = (*state * 1103515245 + 12345) & 0x7fffffff;
    return *state;
}

/* Complex computation with data dependencies */
double compute_kernel(MixedData *data, int size, volatile int bound) {
    double sum = 0.0;
    double prod = 1.0;
    uint32_t rng_state = volatile_seed;
    
    /* Outer loop with volatile bound */
    for (volatile int outer = 0; outer < bound; outer++) {
        /* Inner loop with carried dependencies */
        for (int i = 1; i < size; i++) {
            /* Data-dependent computation with mixed operations */
            double temp1 = data[i].d * data[i-1].d;
            double temp2 = data[i].c * data[i-1].c;
            
            /* Conditional operations creating control flow */
            if (data[i].a % 3 == 0) {
                /* Branch 1: Complex FP operations */
                sum += temp1 / (data[i].b + 1.0);
                prod *= temp2 * 0.5;
                
                /* Inline assembly to create fixed RTL */
                asm volatile("" ::: "memory");
            } else if (data[i].a % 3 == 1) {
                /* Branch 2: Integer-heavy operations */
                int64_t int_op = data[i].b * data[i-1].b;
                sum += (int_op % 100) * 0.01;
                prod /= (abs(data[i].a) + 1.0);
                
                /* Switch statement for additional control flow */
                switch (data[i].a % 4) {
                    case 0:
                        sum += 0.1;
                        break;
                    case 1:
                        sum -= 0.1;
                        break;
                    case 2:
                        prod *= 1.1;
                        break;
                    case 3:
                        prod /= 1.1;
                        break;
                }
            } else {
                /* Branch 3: Mixed operations with pointer chasing */
                MixedData *ptr = &data[i];
                for (int j = 0; j < 2; j++) {
                    sum += ptr->d;
                    ptr = &data[(ptr->a + j) % size];
                }
                prod *= 0.9 + (prng(&rng_state) % 100) * 0.001;
            }
            
            /* Ternary operator for conditional move pattern */
            double select = (data[i].b > data[i-1].b) ? 
                           data[i].d : data[i-1].d;
            sum += select * 0.01;
            
            /* Non-trivial addressing mode */
            int idx = (i * 13 + outer * 7) % size;
            prod += data[idx].c * 0.001;
        }
        
        /* Partial unrolling hint */
        #pragma GCC unroll 4
        for (int k = 0; k < 4 && k < size; k++) {
            /* Additional computation in unrolled section */
            sum += data[k].d * k * 0.0001;
            prod *= 1.0 + data[k].c * 0.00001;
        }
    }
    
    return sum + prod;
}

/* Second computation kernel for additional scheduling regions */
void matrix_vector_multiply(float *matrix, float *vector, float *result, 
                           int rows, int cols, volatile int iter) {
    /* Volatile iteration counter */
    for (volatile int it = 0; it < iter; it++) {
        /* Nested loops for matrix operations */
        for (int i = 0; i < rows; i++) {
            float acc = 0.0f;
            
            /* Inner loop with stride access */
            for (int j = 0; j < cols; j++) {
                /* Complex addressing with mixed operations */
                int idx = i * cols + j;
                acc += matrix[idx] * vector[j];
                
                /* Data-dependent conditional */
                if (j % 3 == 0) {
                    acc += matrix[idx] * 0.1f;
                } else if (j % 3 == 1) {
                    acc -= matrix[idx] * 0.05f;
                }
                
                /* Inline assembly barrier */
                asm volatile("" ::: "memory");
            }
            
            /* Division with non-constant divisor */
            result[i] = acc / (vector[i % cols] + 1.0f);
            
            /* Mixed-width operation */
            result[i] += (double)(i * it) * 0.001;
        }
        
        /* Update vector with feedback */
        for (int i = 0; i < cols; i++) {
            vector[i] = result[i % rows] * 0.9f + vector[i] * 0.1f;
        }
    }
}

int main() {
    const int data_size = 1024;
    const int matrix_rows = 64;
    const int matrix_cols = 64;
    
    /* Initialize with pseudo-random data */
    MixedData *data = (MixedData*)malloc(data_size * sizeof(MixedData));
    float *matrix = (float*)malloc(matrix_rows * matrix_cols * sizeof(float));
    float *vector = (float*)malloc(matrix_cols * sizeof(float));
    float *result = (float*)malloc(matrix_rows * sizeof(float));
    
    uint32_t rng = time(NULL);
    
    /* Fill arrays with random data */
    for (int i = 0; i < data_size; i++) {
        data[i].a = prng(&rng) % 1000;
        data[i].b = prng(&rng) % 10000;
        data[i].c = (prng(&rng) % 1000) * 0.001f;
        data[i].d = (prng(&rng) % 10000) * 0.0001;
    }
    
    for (int i = 0; i < matrix_rows * matrix_cols; i++) {
        matrix[i] = (prng(&rng) % 1000) * 0.001f;
    }
    
    for (int i = 0; i < matrix_cols; i++) {
        vector[i] = (prng(&rng) % 1000) * 0.001f;
    }
    
    /* Run first computation kernel */
    volatile int bound1 = volatile_bound / 2 + (rand() % 10);
    double result1 = compute_kernel(data, data_size, bound1);
    
    /* Run second computation kernel */
    volatile int bound2 = volatile_bound / 4 + (rand() % 5);
    matrix_vector_multiply(matrix, vector, result, 
                          matrix_rows, matrix_cols, bound2);
    
    /* Final reduction to prevent optimization */
    double final_sum = result1;
    for (int i = 0; i < matrix_rows; i++) {
        final_sum += result[i];
    }
    
    /* XOR-style reduction for integer parts */
    uint64_t int_hash = 0;
    for (int i = 0; i < data_size; i++) {
        int_hash ^= data[i].a;
        int_hash ^= data[i].b;
    }
    
    final_sum += int_hash * 0.000000001;
    
    printf("Result: %.10f\n", final_sum);
    
    /* Cleanup */
    free(data);
    free(matrix);
    free(vector);
    free(result);
    
    return 0;
}
