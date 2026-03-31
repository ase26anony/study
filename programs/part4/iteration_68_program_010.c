/* sel_sched_trigger.c - Program to trigger selective scheduler debug dumping */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_bound = 1000;
volatile int g_volatile_seed = 42;

/* Complex data-dependent computation with carried dependencies */
static inline int64_t complex_op(int64_t a, int64_t b, int64_t c) {
    /* Mixed-width operations create register pressure */
    int32_t a32 = (int32_t)a;
    int64_t b64 = b;
    
    /* Conditional move via ternary */
    int64_t cond_val = (a32 > 0) ? (b64 * 3) : (b64 / 2);
    
    /* Floating-point operations mixed with integer */
    double fp_val = (double)(a32 & 0xFF) * 1.5;
    int64_t int_val = (int64_t)(fp_val * (double)cond_val);
    
    /* Division with non-constant divisor */
    if (c != 0) {
        int_val /= (c | 1);  /* Avoid division by zero */
    }
    
    return int_val ^ (a32 * b64);
}

/* Pointer chasing pattern */
static int64_t chase_pointer(int64_t *data, int size, int start) {
    int idx = start;
    int64_t sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent access pattern */
        idx = (idx * 13 + 17) % size;
        
        /* Complex addressing mode */
        int64_t val = data[idx] + data[(idx + 1) % size] * 2;
        
        /* Inline assembly to create fixed RTL */
        asm volatile("" : "+r"(val) : : "memory");
        
        sum += val;
        
        /* Branch with substantial computation on both sides */
        if (val > 1000) {
            sum = (sum * 3) / 2;
        } else {
            sum = (sum / 2) + (val % 256);
        }
    }
    
    return sum;
}

/* Matrix-vector like computation */
static void matrix_vector_op(double *matrix, double *vector, double *result, 
                            int rows, int cols, volatile int bound) {
    for (int i = 0; i < rows; i++) {
        double sum = 0.0;
        
        #pragma GCC unroll 4
        for (int j = 0; j < cols; j++) {
            /* Strided memory access */
            double elem = matrix[i * cols + j];
            double vec_elem = vector[j];
            
            /* Floating-point operations */
            double prod = elem * vec_elem;
            
            /* Conditional based on random data */
            if ((j & 3) == 0) {
                prod *= 1.1;
            } else {
                prod /= 1.05;
            }
            
            sum += prod;
            
            /* Another inline assembly barrier */
            asm volatile("" : "+x"(sum) : : "memory");
        }
        
        /* Division with volatile bound to prevent optimization */
        result[i] = sum / (double)(bound + 1);
    }
}

/* Switch statement with multiple computation paths */
static int64_t switch_computation(int64_t val, int mode) {
    int64_t result = val;
    
    switch (mode & 7) {
        case 0:
            result = (result * 3) / 2;
            result += (result >> 4) ^ (result << 3);
            break;
        case 1:
            result = (result / 5) * 7;
            result ^= (result % 256) * 0x12345678;
            break;
        case 2:
            result = (result + 0xABCD) * (result - 0x1234);
            result = result | (result >> 16);
            break;
        case 3:
            result = (result ^ 0xFFFFFFFF) * 3;
            result = result / ((result & 0xFF) + 1);
            break;
        case 4:
            result = (result << 3) | (result >> 61);
            result += (result * result) / 1000;
            break;
        default:
            result = (result + mode) * (result - mode);
            result = result ^ (result >> 32);
            break;
    }
    
    return result;
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int MATRIX_ROWS = 64;
    const int MATRIX_COLS = 64;
    
    /* Initialize with pseudo-random data */
    int64_t *data = (int64_t*)malloc(ARRAY_SIZE * sizeof(int64_t));
    double *matrix = (double*)malloc(MATRIX_ROWS * MATRIX_COLS * sizeof(double));
    double *vector = (double*)malloc(MATRIX_COLS * sizeof(double));
    double *result = (double*)malloc(MATRIX_ROWS * sizeof(double));
    
    /* Simple PRNG for initialization */
    unsigned int seed = time(NULL) ^ g_volatile_seed;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        data[i] = (int64_t)(seed ^ (seed >> 16));
    }
    
    for (int i = 0; i < MATRIX_ROWS * MATRIX_COLS; i++) {
        seed = seed * 1103515245 + 12345;
        matrix[i] = (double)(seed % 1000) / 100.0;
    }
    
    for (int i = 0; i < MATRIX_COLS; i++) {
        seed = seed * 1103515245 + 12345;
        vector[i] = (double)(seed % 500) / 50.0;
    }
    
    /* Main computation with nested loops */
    int64_t total_sum = 0;
    volatile int outer_bound = g_volatile_bound % 100;
    
    for (int outer = 0; outer < outer_bound; outer++) {
        int64_t inner_sum = 0;
        
        /* Inner loop with carried dependency */
        for (int i = 1; i < ARRAY_SIZE; i++) {
            /* Data-dependent computation with cross-iteration dependency */
            int64_t prev = data[i-1];
            int64_t curr = data[i];
            
            /* Complex operation with dependency chain */
            int64_t val = complex_op(prev, curr, inner_sum & 0xFF);
            
            /* Update with dependency on previous iteration */
            inner_sum = inner_sum + val * (i % 8);
            
            /* Branch inside hot loop */
            if ((i & 15) == 0) {
                inner_sum = switch_computation(inner_sum, i);
            } else {
                inner_sum ^= (val << 3) | (val >> 61);
            }
            
            /* Volatile function call prevents optimization */
            if (i % 128 == 0) {
                inner_sum += rand() % 16;
            }
        }
        
        total_sum ^= inner_sum;
        
        /* Pointer chasing every few iterations */
        if (outer % 3 == 0) {
            int64_t chase_result = chase_pointer(data, ARRAY_SIZE, outer);
            total_sum += chase_result * 3;
        }
    }
    
    /* Second computation kernel */
    matrix_vector_op(matrix, vector, result, MATRIX_ROWS, MATRIX_COLS, outer_bound);
    
    /* Final reduction */
    double final_fp = 0.0;
    for (int i = 0; i < MATRIX_ROWS; i++) {
        final_fp += result[i] * (i + 1);
    }
    
    /* Mix integer and floating-point results */
    total_sum += (int64_t)(final_fp * 1000.0);
    
    /* Ensure side effect is observable */
    printf("Result: %ld (checksum: %f)\n", total_sum, final_fp);
    
    /* Cleanup */
    free(data);
    free(matrix);
    free(vector);
    free(result);
    
    return (total_sum != 0) ? 0 : 1;
}
