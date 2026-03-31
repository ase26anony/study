/* test_sel_sched.c - Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 2048
#define INNER_LOOP 100
#define MIDDLE_LOOP 50
#define OUTER_LOOP 20

/* Mixed data type structure with non-contiguous access pattern */
struct MixedData {
    int id;
    float value;
    double weight;
    char tag;
    int32_t counter;
};

/* Volatile variables to prevent optimization of control flow */
volatile int g_volatile_seed = 42;
volatile int g_volatile_mod = 7;

/* Simple PRNG to avoid library dependencies */
static inline int simple_rand(int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

/* Function 1: Reduction with carried dependency across iterations */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
double reduction_with_carry(struct MixedData *data, int count) {
    double acc = 0.0;
    volatile int v_cond = g_volatile_seed;
    
    /* Complex loop with data-dependent branching */
    for (int i = 1; i < count; i++) {
        /* Volatile condition prevents dead code elimination */
        if ((v_cond % g_volatile_mod) > 3) {
            /* Carried dependency: uses previous iteration's value */
            acc += data[i].weight * data[i-1].value;
            
            /* Mixed operations */
            data[i].counter = (int)(acc * 1000.0);
            data[i].value = (float)(data[i].weight * 0.5);
        } else {
            /* Alternative path with different operations */
            acc -= data[i].weight * 0.25;
            data[i].tag = (char)((int)acc & 0xFF);
        }
        
        /* More complex data-dependent computation */
        if ((simple_rand(&g_volatile_seed) % 5) == 0) {
            acc *= 1.01;
            data[i].id = i * 2;
        }
        
        /* Non-contiguous memory access pattern */
        if (i % 3 == 0) {
            data[i].weight = data[i-2].weight * 0.9;
        }
    }
    
    return acc;
}

/* Function 2: Deeply nested loops with volatile conditionals */
__attribute__((optimize("O3", "funroll-loops")))
float nested_loops_complex(int *array, int size) {
    float result = 0.0f;
    volatile int outer_cond = g_volatile_mod;
    
    /* Triple nested loop structure */
    for (int i = 0; i < OUTER_LOOP; i++) {
        int local_seed = g_volatile_seed + i;
        
        for (int j = 0; j < MIDDLE_LOOP; j++) {
            /* Volatile condition in middle loop */
            if ((outer_cond % (j + 2)) > (i % 3)) {
                for (int k = 0; k < INNER_LOOP; k++) {
                    /* Data-dependent computation in innermost loop */
                    int idx = (i * MIDDLE_LOOP * INNER_LOOP + 
                              j * INNER_LOOP + k) % size;
                    
                    /* Mixed integer/float operations */
                    if ((simple_rand(&local_seed) % 4) == 0) {
                        result += array[idx] * 0.5f;
                        array[idx] = (int)(result * 100.0f);
                    } else {
                        result -= array[idx] * 0.25f;
                        
                        /* Conditional store with stride */
                        if (k % 4 == 0) {
                            array[(idx + 1) % size] = 
                                array[idx] + (int)result;
                        }
                    }
                    
                    /* Additional floating point operation */
                    result = result * 0.999f + 0.001f;
                }
            }
        }
        
        /* Modify volatile between outer iterations */
        outer_cond = (outer_cond * 3 + 1) % 13;
    }
    
    return result;
}

/* Function 3: Mixed data type processing with pointer arithmetic */
__attribute__((optimize("O2", "fsel-sched-pipelining-outer-loops")))
void process_mixed_types(struct MixedData *data, int count, double *output) {
    double temp_acc = 0.0;
    volatile int stride_cond = g_volatile_seed;
    
    /* Process with non-unit stride */
    for (int i = 0; i < count; i += 2) {
        /* Complex condition with volatile */
        int stride = (stride_cond % 3) + 1;
        int next_idx = (i + stride) % count;
        
        /* Mixed type operations */
        double product = data[i].weight * data[next_idx].value;
        
        if ((stride_cond % 5) > 2) {
            temp_acc += product * data[i].counter;
            data[i].tag = 'A' + (i % 26);
        } else {
            temp_acc -= product * 0.75;
            data[i].tag = 'a' + (i % 26);
        }
        
        /* Pointer arithmetic with type casting */
        char *byte_ptr = (char *)&data[i];
        for (int b = 0; b < sizeof(struct MixedData) / 2; b++) {
            byte_ptr[b] ^= (i + b) & 0xFF;
        }
        
        /* Update volatile condition */
        stride_cond = (stride_cond * 7 + 1) % 17;
    }
    
    *output = temp_acc;
}

/* Function 4: Reduction with multiple accumulators */
__attribute__((optimize("O3")))
void multi_reduction(struct MixedData *data, int count, 
                     double *sum1, float *sum2, int64_t *sum3) {
    double d_acc = 0.0;
    float f_acc = 0.0f;
    int64_t i_acc = 0;
    
    volatile int v_switch = g_volatile_mod;
    
    for (int i = 0; i < count; i++) {
        /* Multiple independent but interleaved reductions */
        d_acc += data[i].weight;
        
        if ((v_switch % 3) == 0) {
            f_acc += data[i].value * 2.0f;
            v_switch = (v_switch + 1) % 7;
        } else {
            i_acc += data[i].counter;
            v_switch = (v_switch * 2) % 7;
        }
        
        /* Complex conditional update */
        if (i % 4 == 0) {
            data[i].weight = d_acc * 0.01;
            data[i].value = f_acc * 0.5f;
        }
        
        /* Additional dependency chain */
        if (i > 0) {
            d_acc += data[i-1].weight * 0.1;
        }
    }
    
    *sum1 = d_acc;
    *sum2 = f_acc;
    *sum3 = i_acc;
}

/* Main driver with varied workload */
int main(void) {
    /* Allocate and initialize data */
    struct MixedData *data = (struct MixedData *)malloc(SIZE * sizeof(struct MixedData));
    int *int_array = (int *)malloc(SIZE * sizeof(int));
    
    if (!data || !int_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    int seed = 123456789;
    for (int i = 0; i < SIZE; i++) {
        data[i].id = i;
        data[i].value = (float)(simple_rand(&seed) % 1000) / 10.0f;
        data[i].weight = (double)(simple_rand(&seed) % 2000) / 20.0;
        data[i].tag = (char)('A' + (i % 26));
        data[i].counter = simple_rand(&seed) % 10000;
        
        int_array[i] = simple_rand(&seed) % 5000;
    }
    
    double result1, result3, sum1;
    float result2, sum2;
    int64_t sum3;
    
    /* Call all computation functions to create varied scheduling workload */
    result1 = reduction_with_carry(data, SIZE);
    printf("Result 1 (reduction with carry): %.6f\n", result1);
    
    result2 = nested_loops_complex(int_array, SIZE);
    printf("Result 2 (nested loops): %.6f\n", result2);
    
    process_mixed_types(data, SIZE, &result3);
    printf("Result 3 (mixed types): %.6f\n", result3);
    
    multi_reduction(data, SIZE, &sum1, &sum2, &sum3);
    printf("Result 4 (multi reduction): %.6f, %.6f, %ld\n", 
           sum1, (double)sum2, (long)sum3);
    
    /* Final combination to ensure all results are used */
    double final_result = result1 + result2 + result3 + sum1 + sum2;
    printf("Final combined result: %.6f\n", final_result);
    
    /* Cleanup */
    free(data);
    free(int_array);
    
    return 0;
}
