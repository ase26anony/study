/* sel-sched-trigger.c - Program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_bound = 1000;
volatile int g_volatile_seed = 42;

/* External function to create dependencies */
extern int rand_r(unsigned int *seed);

/* Complex computation with data dependencies */
static inline int64_t complex_op(int64_t a, int64_t b, int64_t c) {
    /* Mixed-width operations */
    int32_t a32 = (int32_t)a;
    int64_t b64 = b;
    
    /* Conditional move via ternary */
    int64_t cond = (a32 > 0) ? b64 : -b64;
    
    /* Division with non-constant divisor */
    if (c != 0) {
        cond /= (c | 1);  /* Avoid division by zero */
    }
    
    /* Floating point to create FP ops */
    double d = (double)cond * 1.234567;
    
    /* Integer result with dependency chain */
    return (int64_t)d + a * b - c;
}

/* Pointer chasing pattern */
int64_t chase_pointer(int64_t *data, int size, int start) {
    int64_t sum = 0;
    int idx = start;
    
    for (int i = 0; i < size * 2; i++) {
        /* Data-dependent access with stride */
        idx = (idx * 13 + 7) % size;
        
        /* Complex addressing mode */
        int64_t val1 = data[idx];
        int64_t val2 = data[(idx + 1) % size];
        int64_t val3 = data[(idx + 3) % size];
        
        /* Carry dependency across iterations */
        sum = complex_op(sum, val1, val2);
        
        /* Conditional branch with computation in both paths */
        if (val3 > 0) {
            sum += val1 * val2 - val3;
            /* Inline assembly to create fixed RTL */
            asm volatile("" : : : "memory");
        } else {
            sum -= val1 / (val2 | 1) + val3;
            /* Another inline assembly barrier */
            asm volatile("# barrier" : : : "memory");
        }
    }
    
    return sum;
}

/* Matrix-vector like computation */
void matrix_style_compute(float *matrix, float *vector, float *result, int n) {
    volatile int bound = g_volatile_bound;
    
    for (int i = 0; i < n; i++) {
        float sum = 0.0f;
        
        /* Inner loop with unrolling hint */
        #pragma GCC unroll 4
        for (int j = 0; j < n; j++) {
            /* Strided access pattern */
            float m_val = matrix[i * n + j];
            float v_val = vector[j];
            
            /* Floating point operations */
            float prod = m_val * v_val;
            
            /* Conditional based on random-like value */
            if ((j & 3) == 0) {
                prod *= 1.5f;
                sum += prod;
            } else {
                prod /= 1.3f;
                sum -= prod;
            }
            
            /* Mixed precision */
            double dprod = (double)prod * 0.987654;
            sum += (float)dprod;
        }
        
        /* Store with potential aliasing */
        result[i] = sum;
        
        /* Volatile operation to prevent reordering */
        asm volatile("" : : "r"(sum) : "memory");
    }
}

/* Switch-based computation */
int64_t switch_computation(int64_t x, int mode) {
    int64_t result = x;
    
    switch (mode & 7) {
        case 0:
            result = (result * 3) / 2;
            result += (result >> 4) & 0x0F0F0F0F;
            break;
        case 1:
            result = (result << 2) | (result >> 62);
            result ^= 0xAAAAAAAAAAAAAAAA;
            break;
        case 2:
            result = (result + 0x12345678) * 0x9ABCDEF0;
            result = result % 1000000007;
            break;
        case 3:
            result = (result & 0xFFFFFFFF) * (result >> 32);
            result = result | (result << 32);
            break;
        case 4:
            result = (result ^ 0x55555555) * 7;
            result = result / ((result & 0xFF) + 1);
            break;
        default:
            result = ~result;
            result = result * result - result;
            break;
    }
    
    return result;
}

int main(void) {
    const int N = 512;
    const int M = 256;
    
    /* Initialize with pseudo-random data */
    int64_t *data1 = (int64_t*)malloc(N * sizeof(int64_t));
    float *matrix = (float*)malloc(N * N * sizeof(float));
    float *vector = (float*)malloc(N * sizeof(float));
    float *result = (float*)malloc(N * sizeof(float));
    
    unsigned int seed = time(NULL);
    
    for (int i = 0; i < N; i++) {
        data1[i] = rand_r(&seed) ^ ((int64_t)rand_r(&seed) << 32);
        vector[i] = (float)rand_r(&seed) / RAND_MAX;
        for (int j = 0; j < N; j++) {
            matrix[i * N + j] = (float)rand_r(&seed) / RAND_MAX;
        }
    }
    
    int64_t total_sum = 0;
    volatile int outer_bound = g_volatile_bound / 2;
    
    /* Outer loop with volatile bound */
    for (int outer = 0; outer < outer_bound; outer++) {
        /* Pointer chasing computation */
        int64_t chase_result = chase_pointer(data1, N, outer % N);
        
        /* Nested loop with data dependency */
        for (int inner = 0; inner < M; inner++) {
            /* Running product with cross-iteration dependency */
            static int64_t running_prod = 1;
            
            /* Complex addressing with modulo */
            int idx1 = (inner * 17 + outer * 23) % N;
            int idx2 = (inner * 13 + outer * 29) % N;
            
            int64_t val1 = data1[idx1];
            int64_t val2 = data1[idx2];
            
            /* Data-dependent computation */
            if (val1 > val2) {
                running_prod = running_prod * val1 - val2;
            } else {
                running_prod = running_prod / (val2 | 1) + val1;
            }
            
            /* Switch statement inside hot loop */
            int64_t switched = switch_computation(running_prod, inner);
            
            /* Update total with dependency */
            total_sum ^= switched + chase_result;
            
            /* Prevent dead code elimination */
            asm volatile("" : : "r"(running_prod), "r"(total_sum) : "memory");
        }
        
        /* Matrix computation every few iterations */
        if (outer % 10 == 0) {
            matrix_style_compute(matrix, vector, result, 64);
            
            /* Use result to prevent optimization */
            float check = 0.0f;
            for (int i = 0; i < 64; i++) {
                check += result[i];
            }
            total_sum += (int64_t)check;
        }
    }
    
    /* Final reduction */
    int64_t final_result = total_sum;
    for (int i = 0; i < N; i++) {
        final_result ^= data1[i];
    }
    
    printf("Result: %ld\n", (long)final_result);
    
    free(data1);
    free(matrix);
    free(vector);
    free(result);
    
    return 0;
}
