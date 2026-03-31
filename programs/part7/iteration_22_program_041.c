/* Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 10000
#define INNER_LOOP 50
#define MIDDLE_LOOP 20
#define OUTER_LOOP 10

/* Mixed data type structure with non-contiguous access pattern */
struct MixedData {
    int id;
    float value;
    double weight;
    char tag;
    volatile int flag; /* volatile to prevent optimization */
};

/* Global volatile variables to create data-dependent control flow */
volatile int global_counter = 0;
volatile int global_seed = 12345;

/* Simple PRNG to avoid library dependencies */
static unsigned int prng_state = 123456789;
static inline unsigned int fast_rand(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

/* Function 1: Reduction with carried dependency across iterations */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
double reduction_with_carry(struct MixedData* data, int count) {
    double acc = 0.0;
    double prev = data[0].weight;
    
    /* Complex loop with carried dependency */
    for (int i = 1; i < count; i += 3) { /* Non-unit stride of 3 */
        double current = data[i].weight;
        
        /* Carried dependency: uses value from previous iteration */
        acc = acc + prev * current;
        
        /* Data-dependent conditional with volatile */
        if (global_counter++ % 7 == 0) {
            acc += data[i].value * 0.5;
        }
        
        /* Mixed operations */
        prev = current + data[i].value;
        
        /* More complex dependency chain */
        if (data[i].flag & 1) {
            acc -= data[i-1].weight * 0.25;
        }
    }
    
    return acc;
}

/* Function 2: Deeply nested loops with volatile conditionals */
__attribute__((optimize("O3", "funroll-loops")))
float nested_loops_complex(int* array, int size) {
    float result = 0.0f;
    volatile int v1 = global_seed;
    volatile int v2 = global_seed * 2;
    
    /* Triple nested loop */
    for (int i = 0; i < OUTER_LOOP; i++) {
        for (int j = 0; j < MIDDLE_LOOP; j++) {
            for (int k = 0; k < INNER_LOOP; k++) {
                /* Data-dependent branch with volatile */
                if ((v1++ & 3) == 0) {
                    result += array[(i * MIDDLE_LOOP * INNER_LOOP + 
                                    j * INNER_LOOP + k) % size] * 1.5f;
                } else {
                    result -= array[(j * INNER_LOOP + k) % size] * 0.75f;
                }
                
                /* More volatile-dependent operations */
                if (v2-- % 5 == 0) {
                    result *= 1.1f;
                }
                
                /* Mixed integer/float operations */
                int idx = (fast_rand() % size);
                result += (array[idx] % 100) * 0.01f;
            }
            
            /* Middle loop complexity */
            if (global_counter & (1 << (j % 8))) {
                result = result / 1.5f;
            }
        }
        
        /* Outer loop with side effect */
        global_seed ^= (i * 137);
    }
    
    return result;
}

/* Function 3: Mixed data type processing with conditional stores */
__attribute__((optimize("O2", "fsel-sched-pipelining-outer-loops")))
void process_mixed_types(struct MixedData* data, int count, double* output) {
    double temp_acc = 0.0;
    int int_acc = 0;
    
    for (int i = 0; i < count; i += 2) { /* Stride 2 for non-contiguous access */
        /* Mixed type operations */
        double dbl_op = data[i].weight * 2.0;
        float flt_op = data[i].value * 3.0f;
        
        /* Conditional store based on volatile */
        if (data[i].flag || (global_seed++ % 11 == 0)) {
            data[i].value = flt_op + (i % 100) * 0.1f;
            temp_acc += dbl_op;
        }
        
        /* Integer operations with dependency */
        int_acc += data[i].id;
        if (int_acc > 1000) {
            int_acc -= 500;
            temp_acc *= 0.9;
        }
        
        /* More complex dependency chain */
        for (int j = 0; j < 3; j++) {
            if ((i + j) < count) {
                data[i+j].tag = (char)((int_acc + j) % 256);
                temp_acc += data[i+j].weight * j;
            }
        }
    }
    
    *output = temp_acc + int_acc;
}

/* Function 4: Reduction with multiple accumulators for ILP */
__attribute__((optimize("O3")))
double multi_acc_reduction(double* array, int size) {
    double acc1 = 0.0, acc2 = 0.0, acc3 = 0.0, acc4 = 0.0;
    volatile int mod = global_seed;
    
    for (int i = 0; i < size; i++) {
        /* Multiple independent accumulators for instruction-level parallelism */
        double val = array[i];
        
        acc1 += val * 1.1;
        acc2 += val * 0.9;
        
        /* Data-dependent operation */
        if (mod++ % 13 == 0) {
            acc3 += val * 1.5;
        } else {
            acc4 += val * 0.5;
        }
        
        /* Cross-accumulator dependency every 8 iterations */
        if (i % 8 == 0) {
            acc1 = acc1 * 0.99 + acc2 * 0.01;
            acc3 = acc3 * 0.95 + acc4 * 0.05;
        }
    }
    
    /* Final reduction */
    return acc1 + acc2 + acc3 + acc4;
}

/* Main driver with varied workload */
int main(void) {
    /* Initialize data arrays */
    struct MixedData* mixed_array = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    double* double_array = (double*)malloc(SIZE * sizeof(double));
    
    if (!mixed_array || !int_array || !double_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        mixed_array[i].id = fast_rand() % 1000;
        mixed_array[i].value = (fast_rand() % 10000) * 0.001f;
        mixed_array[i].weight = (fast_rand() % 10000) * 0.0001;
        mixed_array[i].tag = (char)(fast_rand() % 256);
        mixed_array[i].flag = fast_rand() % 2;
        
        int_array[i] = fast_rand() % 10000;
        double_array[i] = (fast_rand() % 10000) * 0.0001;
    }
    
    double total_result = 0.0;
    
    /* Call different computation patterns to trigger various scheduler behaviors */
    total_result += reduction_with_carry(mixed_array, SIZE);
    
    float nested_result = nested_loops_complex(int_array, SIZE);
    total_result += nested_result;
    
    double mixed_output;
    process_mixed_types(mixed_array, SIZE, &mixed_output);
    total_result += mixed_output;
    
    total_result += multi_acc_reduction(double_array, SIZE);
    
    /* Additional complex pattern: combination of all */
    for (int iter = 0; iter < 5; iter++) {
        global_seed = fast_rand() % 100;
        
        /* Interleave calls to create complex call graph */
        total_result += reduction_with_carry(mixed_array, SIZE / (iter + 1));
        
        if (iter % 2 == 0) {
            nested_result = nested_loops_complex(int_array, SIZE / 2);
            total_result += nested_result;
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %f\n", total_result);
    printf("Global counter: %d\n", global_counter);
    printf("Global seed: %d\n", global_seed);
    
    /* Cleanup */
    free(mixed_array);
    free(int_array);
    free(double_array);
    
    return 0;
}
