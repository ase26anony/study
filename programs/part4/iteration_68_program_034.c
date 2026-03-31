/* sel_sched_trigger.c - Program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_bound = 1000;
volatile int g_volatile_seed = 42;

/* Complex data-dependent computation with mixed operations */
static inline int64_t complex_op(int64_t a, int64_t b, float c, double d) {
    /* Mixed-width arithmetic and conditional moves */
    int32_t a32 = (int32_t)a;
    int64_t result = (a > b) ? (a * b) : (a + b * 2);
    
    /* Floating-point operations */
    float f1 = (float)result * c;
    double d1 = (double)f1 / (d + 1.0);
    
    /* Non-constant division creates scheduling complexity */
    result = (int64_t)(result / (abs(b % 16) + 1));
    
    /* Inline assembly to create fixed RTL patterns */
    asm volatile ("" : "+r" (result) : : "memory");
    
    return result + (int64_t)d1;
}

/* Pointer-chasing pattern with data dependencies */
int64_t pointer_chase(int64_t* data, int size, volatile int* bound) {
    int64_t sum = 0;
    int chase_idx = 0;
    
    for (int i = 0; i < *bound; i++) {
        /* Data-dependent index calculation */
        chase_idx = (chase_idx + data[i % size] % 7) % size;
        
        /* Complex computation with dependency chain */
        int64_t val = data[chase_idx];
        sum = complex_op(sum, val, (float)(i % 256), (double)(chase_idx % 128));
        
        /* Conditional branch with substantial computation in both paths */
        if (val > 0) {
            /* Branch 1: Integer-heavy operations */
            sum += (val * (i % 31)) / ((abs(val % 13) + 1));
            sum ^= (sum << 13) | (sum >> 51);
        } else {
            /* Branch 2: Floating-point heavy operations */
            float fval = (float)val * 0.5f;
            double dval = (double)fval * 1.5;
            sum += (int64_t)(dval * (i % 17));
            sum = (sum * 1103515245 + 12345) & 0x7fffffff;
        }
        
        /* Memory access with non-trivial addressing */
        int64_t* ptr = &data[(chase_idx + i) % size];
        sum += *ptr;
    }
    
    return sum;
}

/* Matrix-vector multiplication kernel */
void matvec_multiply(float* matrix, float* vector, float* result, 
                     int rows, int cols, volatile int iter) {
    #pragma GCC unroll 4
    for (int r = 0; r < rows; r++) {
        float sum = 0.0f;
        
        /* Inner loop with partial unrolling hint */
        #pragma GCC unroll 2
        for (int c = 0; c < cols; c++) {
            /* Mixed operations and memory access pattern */
            float val = matrix[r * cols + c] * vector[c];
            
            /* Conditional operation */
            sum += (val > 0.0f) ? val : val * 0.5f;
            
            /* Inline assembly barrier */
            asm volatile ("" : : : "memory");
        }
        
        /* Non-constant division */
        result[r] = sum / ((iter % 8) + 1.0f);
    }
}

/* Switch-based computation with multiple basic blocks */
int64_t switch_computation(int64_t x, int mode) {
    int64_t result = x;
    
    switch (mode % 5) {
        case 0:
            /* Integer arithmetic path */
            result = (result * 6364136223846793005ULL) + 1442695040888963407ULL;
            result ^= result >> 27;
            break;
            
        case 1:
            /* Floating-point path */
            {
                double d = (double)result * 0.6180339887498949;
                result = (int64_t)(d * 1000000.0);
            }
            break;
            
        case 2:
            /* Bit manipulation path */
            result = ((result & 0xAAAAAAAAAAAAAAAA) >> 1) |
                     ((result & 0x5555555555555555) << 1);
            break;
            
        case 3:
            /* Division-heavy path */
            result = result / ((abs((int)result % 19) + 1) * 2);
            break;
            
        case 4:
            /* Mixed operations path */
            {
                float f = (float)result;
                result = (int64_t)(f * f) + (result % 1023);
            }
            break;
    }
    
    return result;
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int MATRIX_ROWS = 64;
    const int MATRIX_COLS = 64;
    
    /* Initialize with pseudo-random data */
    int64_t* data = (int64_t*)malloc(ARRAY_SIZE * sizeof(int64_t));
    float* matrix = (float*)malloc(MATRIX_ROWS * MATRIX_COLS * sizeof(float));
    float* vector = (float*)malloc(MATRIX_COLS * sizeof(float));
    float* result = (float*)malloc(MATRIX_ROWS * sizeof(float));
    
    /* Simple PRNG for initialization */
    uint64_t seed = time(NULL);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
        data[i] = (int64_t)seed;
    }
    
    for (int i = 0; i < MATRIX_ROWS * MATRIX_COLS; i++) {
        seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
        matrix[i] = (float)(seed % 1000) / 100.0f;
    }
    
    for (int i = 0; i < MATRIX_COLS; i++) {
        seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
        vector[i] = (float)(seed % 1000) / 100.0f;
    }
    
    /* Volatile iteration counter */
    volatile int iter_count = g_volatile_bound;
    
    /* Main computation loop with nested structure */
    int64_t total_sum = 0;
    
    for (int outer = 0; outer < 10; outer++) {
        /* First kernel: Pointer chasing with data dependencies */
        int64_t chase_result = pointer_chase(data, ARRAY_SIZE, &iter_count);
        total_sum ^= chase_result;
        
        /* Second kernel: Matrix-vector multiplication */
        for (int i = 0; i < 3; i++) {
            matvec_multiply(matrix, vector, result, MATRIX_ROWS, MATRIX_COLS, iter_count);
            
            /* Reduce matrix results */
            float mat_sum = 0.0f;
            for (int j = 0; j < MATRIX_ROWS; j++) {
                mat_sum += result[j];
            }
            total_sum += (int64_t)(mat_sum * 1000.0f);
        }
        
        /* Third kernel: Switch-based computation */
        for (int i = 0; i < ARRAY_SIZE / 4; i++) {
            int64_t switch_res = switch_computation(data[i], i + outer);
            total_sum += switch_res;
            
            /* Inline assembly to create scheduling boundaries */
            asm volatile ("" : : : "memory");
        }
        
        /* Modify volatile bound to prevent optimization */
        iter_count = (iter_count + 1) % 1000;
    }
    
    /* Final reduction and output */
    printf("Result: %ld\n", (long)total_sum);
    
    /* Cleanup */
    free(data);
    free(matrix);
    free(vector);
    free(result);
    
    return 0;
}
