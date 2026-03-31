/* sel-sched-trigger.c - Program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_bound = 1000;
volatile int g_volatile_seed = 42;

/* External function to create dependencies */
extern int external_rand(void);

/* Complex computation with data dependencies */
static inline int64_t compute_kernel(int32_t a, int32_t b, float c, double d) {
    /* Mixed-width operations */
    int64_t wide = (int64_t)a * b;
    float f_result = c * (float)b;
    double d_result = d * (double)a;
    
    /* Conditional move pattern */
    int64_t cond_result = (wide > 0) ? wide : -wide;
    
    /* Non-constant division */
    if (b != 0) {
        cond_result /= (int64_t)(b & 0xFF) + 1;
    }
    
    /* Mixed floating-point operations */
    double mixed = d_result + (double)f_result;
    
    /* Convert back to integer with truncation */
    return cond_result + (int64_t)mixed;
}

/* Pointer chasing pattern */
static int64_t chase_pointers(int32_t* data, int size, int start) {
    int idx = start;
    int64_t sum = 0;
    volatile int vol_idx = idx; /* Prevent optimization */
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent access with stride */
        idx = (idx * 1103515245 + 12345) & (size - 1);
        sum += data[idx] * data[(idx + 1) & (size - 1)];
        
        /* Inline assembly to create fixed RTL */
        asm volatile("" : : "r"(sum) : "memory");
        
        vol_idx = idx; /* Use volatile to prevent dead code elimination */
    }
    return sum;
}

/* Matrix-vector like computation */
static void matrix_vector_kernel(float* matrix, float* vector, float* result, 
                                 int rows, int cols) {
    volatile int v_rows = rows; /* Volatile loop bound */
    
    #pragma GCC unroll 4
    for (int i = 0; i < v_rows; i++) {
        float acc = 0.0f;
        int row_start = i * cols;
        
        for (int j = 0; j < cols; j++) {
            /* Complex addressing mode */
            float val = matrix[row_start + j] * vector[j];
            
            /* Branch with computation in both paths */
            if (val > 0.5f) {
                acc += val * 1.5f;
                /* More operations in true path */
                acc -= (float)j * 0.01f;
            } else {
                acc += val * 0.5f;
                /* Different operations in false path */
                acc += (float)(j % 8) * 0.02f;
            }
            
            /* Non-trivial floating point operation */
            if (j % 3 == 0) {
                acc = acc / 1.1f;
            }
        }
        
        /* Store with potential aliasing */
        result[i] = acc;
        
        /* Memory barrier via inline asm */
        asm volatile("" : : "r"(acc) : "memory");
    }
}

/* Switch-based computation */
static int64_t switch_computation(int opcode, int64_t a, int64_t b) {
    int64_t result = 0;
    
    switch (opcode & 0x7) {
        case 0:
            result = a + b;
            /* Complex operation chain */
            result = (result * 3) / 2;
            break;
        case 1:
            result = a - b;
            result = (result << 2) | (result >> 62);
            break;
        case 2:
            result = a * b;
            /* Non-constant division */
            result /= (b & 0xFF) + 1;
            break;
        case 3:
            result = a ^ b;
            result = ~result;
            break;
        case 4:
            result = (a > b) ? a : b;
            result = result * result;
            break;
        case 5:
            result = (a < b) ? a : b;
            result = result + (result << 3);
            break;
        case 6:
            result = a % ((b & 0x3F) + 1);
            result = result * 7;
            break;
        case 7:
            result = a & b;
            result = result | (a ^ b);
            break;
    }
    
    return result;
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int MATRIX_ROWS = 64;
    const int MATRIX_COLS = 16;
    
    /* Initialize with pseudo-random data */
    int32_t* data = (int32_t*)malloc(ARRAY_SIZE * sizeof(int32_t));
    float* matrix = (float*)malloc(MATRIX_ROWS * MATRIX_COLS * sizeof(float));
    float* vector = (float*)malloc(MATRIX_COLS * sizeof(float));
    float* result = (float*)malloc(MATRIX_ROWS * sizeof(float));
    
    /* Simple PRNG for reproducibility */
    unsigned int seed = g_volatile_seed;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        data[i] = (int32_t)(seed & 0x7FFFFFFF);
    }
    
    for (int i = 0; i < MATRIX_ROWS * MATRIX_COLS; i++) {
        seed = seed * 1103515245 + 12345;
        matrix[i] = (float)(seed % 1000) / 1000.0f;
    }
    
    for (int i = 0; i < MATRIX_COLS; i++) {
        seed = seed * 1103515245 + 12345;
        vector[i] = (float)(seed % 1000) / 1000.0f;
    }
    
    /* Main computation with nested loops */
    int64_t total_sum = 0;
    volatile int outer_bound = g_volatile_bound;
    
    for (int outer = 0; outer < outer_bound; outer++) {
        int inner_bound = (outer % 10) + 5; /* Data-dependent bound */
        
        for (int inner = 0; inner < inner_bound; inner++) {
            /* Data-dependent computation with carried dependency */
            static int64_t running_sum = 0;
            
            /* Mixed operations */
            int idx = (outer * 31 + inner * 17) & (ARRAY_SIZE - 1);
            int32_t val1 = data[idx];
            int32_t val2 = data[(idx + 1) & (ARRAY_SIZE - 1)];
            float fval = (float)val1 / 100.0f;
            double dval = (double)val2 / 200.0;
            
            /* Complex kernel call */
            int64_t kernel_result = compute_kernel(val1, val2, fval, dval);
            
            /* Update running sum with dependency */
            running_sum += kernel_result;
            
            /* Switch-based computation */
            int64_t switch_result = switch_computation(
                inner & 0x7, running_sum, kernel_result);
            
            /* Accumulate to total */
            total_sum ^= switch_result; /* XOR reduction */
            
            /* Volatile operation to prevent dead code elimination */
            asm volatile("" : : "r"(total_sum) : "memory");
        }
        
        /* Pointer chasing every few iterations */
        if (outer % 7 == 0) {
            int64_t chase_result = chase_pointers(data, ARRAY_SIZE, outer);
            total_sum += chase_result;
        }
    }
    
    /* Second computation kernel */
    matrix_vector_kernel(matrix, vector, result, MATRIX_ROWS, MATRIX_COLS);
    
    /* Final reduction */
    float final_float_sum = 0.0f;
    for (int i = 0; i < MATRIX_ROWS; i++) {
        final_float_sum += result[i];
    }
    
    /* Mix integer and float results */
    total_sum += (int64_t)(final_float_sum * 1000.0f);
    
    /* Print result to prevent optimization */
    printf("Final result: %ld\n", (long)total_sum);
    
    /* Cleanup */
    free(data);
    free(matrix);
    free(vector);
    free(result);
    
    return 0;
}
