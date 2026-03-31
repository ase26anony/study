/* test_sel_sched.c - Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* Volatile variables to prevent optimization of control flow */
volatile int g_volatile_counter = 0;
volatile float g_volatile_float = 1.5f;

/* Mixed data type structure with non-contiguous access pattern */
struct MixedData {
    int id;
    float value;
    double weight;
    char tag;
    int64_t timestamp;
};

/* Function 1: Reduction with carried dependency across iterations */
__attribute__((optimize("O2")))
double reduction_with_carried_dependency(const double* data, int size) {
    double acc = 0.0;
    double prev = data[0];
    
    /* Complex loop with data-dependent control flow */
    for (int i = 1; i < size; i++) {
        /* Volatile read to prevent optimization */
        int volatile_cond = g_volatile_counter++;
        
        /* Carried dependency: uses value from previous iteration */
        double current = data[i];
        double product = prev * current;
        
        /* Data-dependent branch */
        if (volatile_cond & 0x1) {
            acc += product * g_volatile_float;
        } else {
            acc -= product / (g_volatile_float + 1.0f);
        }
        
        /* Additional arithmetic to create more RTL instructions */
        acc = fmod(acc, 1000.0);
        prev = current + sin(acc * 0.001);
        
        /* Nested conditional with side effect */
        if ((i & 0xF) == 0) {
            g_volatile_float *= 0.99f;
        }
    }
    
    return acc;
}

/* Function 2: Mixed data types with non-contiguous access */
__attribute__((optimize("O3")))
float process_mixed_data(struct MixedData* data, int count, int stride) {
    float total_weight = 0.0f;
    double weighted_sum = 0.0;
    
    /* Outer loop */
    for (int outer = 0; outer < 3; outer++) {
        /* Middle loop with volatile condition */
        int mid_limit = g_volatile_counter % 10 + 5;
        for (int middle = 0; middle < mid_limit; middle++) {
            /* Innermost loop with non-contiguous access */
            for (int i = 0; i < count; i += stride) {
                /* Access every 'stride'-th element */
                int idx = (i + outer * stride) % count;
                
                /* Mixed type operations */
                double temp = data[idx].weight * data[idx].value;
                
                /* Conditional store based on volatile */
                if (g_volatile_counter++ & 0x2) {
                    data[idx].value = (float)temp * 0.5f;
                    weighted_sum += temp * data[idx].timestamp;
                } else {
                    data[idx].value = (float)temp * 1.5f;
                    weighted_sum -= temp / (data[idx].timestamp + 1);
                }
                
                /* Reduction operation */
                total_weight += data[idx].value;
                
                /* Complex conditional with floating point comparison */
                if (total_weight > 1000.0f && (idx % 7) == 0) {
                    total_weight *= 0.9f;
                    g_volatile_float = total_weight * 0.01f;
                }
            }
            
            /* Additional operation between middle loop iterations */
            weighted_sum = fabs(weighted_sum);
        }
    }
    
    return total_weight + (float)weighted_sum;
}

/* Function 3: Deeply nested loops with data-dependent branches */
__attribute__((optimize("O2")))
int64_t complex_nested_loops(int* matrix, int dim) {
    int64_t result = 0;
    volatile int seed = g_volatile_counter;
    
    /* 3-level nested loop */
    for (int i = 0; i < dim; i++) {
        /* Data-dependent loop bound */
        int j_limit = (seed + i) % dim + 1;
        for (int j = 0; j < j_limit; j++) {
            /* Another data-dependent bound */
            int k_limit = (seed * j + i) % 5 + 2;
            for (int k = 0; k < k_limit; k++) {
                /* Complex index calculation */
                int idx = (i * dim + j * k) % (dim * dim);
                
                /* Data-dependent branch with side effects */
                if ((seed + idx) & 0x1) {
                    matrix[idx] += (i * j - k) * g_volatile_counter;
                    result += matrix[idx] * 2;
                } else {
                    matrix[idx] -= (j * k + i) / (g_volatile_counter + 1);
                    result -= matrix[idx] / 3;
                }
                
                /* Additional floating point operation */
                float temp = sinf(matrix[idx] * 0.01f);
                if (temp > 0.5f) {
                    result += (int64_t)(temp * 1000);
                }
                
                /* Modify volatile to affect subsequent iterations */
                seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
            }
            
            /* Reduction across middle loop */
            result = result % 1000000;
        }
        
        /* Outer loop operation with memory access pattern */
        if (i % 4 == 0) {
            for (int t = 0; t < dim; t += 8) {
                matrix[t] ^= result;
            }
        }
    }
    
    return result;
}

/* Function 4: Software pipelining candidate with reduction */
__attribute__((optimize("O3")))
double pipeline_candidate(const float* a, const float* b, int n) {
    double sum = 0.0;
    double sum_sq = 0.0;
    double cross = 0.0;
    
    /* Loop designed to benefit from software pipelining */
    for (int i = 0; i < n; i++) {
        /* Multiple accumulators to create parallel operations */
        float ai = a[i];
        float bi = b[i];
        
        /* Independent operations that can be pipelined */
        double prod = (double)ai * bi;
        double diff = (double)ai - bi;
        
        /* Reduction operations with carried dependencies */
        sum += prod;
        sum_sq += diff * diff;
        
        /* Cross-term with staggered dependency */
        if (i > 0) {
            cross += prod * (double)a[i-1];
        }
        
        /* Data-dependent operation to prevent vectorization */
        if (g_volatile_counter++ & 0x4) {
            sum = fmod(sum, 500.0);
        }
    }
    
    /* Final combination */
    return sum + sum_sq * 0.5 + cross;
}

/* Simple pseudo-random generator to avoid library dependencies */
static uint32_t lcg_state = 1;
static inline uint32_t lcg_rand() {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Initialize array with pseudo-random data */
void init_data(double* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = (double)(lcg_rand() % 1000) / 100.0;
    }
}

/* Initialize mixed data array */
void init_mixed_data(struct MixedData* data, int count) {
    for (int i = 0; i < count; i++) {
        data[i].id = i;
        data[i].value = (float)(lcg_rand() % 100) / 10.0f;
        data[i].weight = (double)(lcg_rand() % 1000) / 1000.0;
        data[i].tag = 'A' + (i % 26);
        data[i].timestamp = lcg_rand();
    }
}

int main() {
    const int DATA_SIZE = 5000;
    const int MATRIX_DIM = 100;
    const int MIXED_SIZE = 3000;
    
    /* Allocate and initialize data */
    double* data = (double*)malloc(DATA_SIZE * sizeof(double));
    struct MixedData* mixed = (struct MixedData*)malloc(MIXED_SIZE * sizeof(struct MixedData));
    int* matrix = (int*)malloc(MATRIX_DIM * MATRIX_DIM * sizeof(int));
    float* array_a = (float*)malloc(DATA_SIZE * sizeof(float));
    float* array_b = (float*)malloc(DATA_SIZE * sizeof(float));
    
    if (!data || !mixed || !matrix || !array_a || !array_b) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize all arrays */
    init_data(data, DATA_SIZE);
    init_mixed_data(mixed, MIXED_SIZE);
    
    for (int i = 0; i < DATA_SIZE; i++) {
        array_a[i] = (float)data[i];
        array_b[i] = (float)(lcg_rand() % 100) / 50.0f;
    }
    
    for (int i = 0; i < MATRIX_DIM * MATRIX_DIM; i++) {
        matrix[i] = lcg_rand() % 1000;
    }
    
    /* Call all computation functions to trigger scheduler activity */
    double result1 = reduction_with_carried_dependency(data, DATA_SIZE);
    float result2 = process_mixed_data(mixed, MIXED_SIZE, 3);
    int64_t result3 = complex_nested_loops(matrix, MATRIX_DIM);
    double result4 = pipeline_candidate(array_a, array_b, DATA_SIZE);
    
    /* Combine results to ensure all computations are live */
    double final_result = result1 + result2 + result3 + result4;
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %f\n", final_result);
    printf("Volatile counter: %d, volatile float: %f\n", 
           g_volatile_counter, g_volatile_float);
    
    /* Cleanup */
    free(data);
    free(mixed);
    free(matrix);
    free(array_a);
    free(array_b);
    
    return 0;
}
