/* test_sel_sched.c - Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization of control flow */
volatile int g_volatile_counter = 0;
volatile float g_volatile_float = 0.0f;

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
    volatile int cond = g_volatile_counter % 7;
    double acc = 0.0;
    double prev = data[0];
    
    /* Complex loop with data-dependent branching */
    for (int i = 1; i < size; i++) {
        /* Data-dependent condition using volatile */
        if ((cond + i) % 3 == 0) {
            acc += data[i] * prev;
        } else if ((cond + i) % 5 == 0) {
            acc -= data[i] * 0.5;
        } else {
            acc += data[i];
        }
        
        /* Carried dependency */
        prev = data[i] * 0.8 + prev * 0.2;
        
        /* Mixed operations to create diverse RTL */
        if (i % 100 == 0) {
            g_volatile_float += (float)acc * 0.1f;
        }
    }
    
    return acc;
}

/* Function 2: Nested loops with volatile conditionals (3+ levels) */
__attribute__((optimize("O2")))
int nested_loops_complex(struct MixedData* array, int rows, int cols, int depth) {
    volatile int outer_cond = g_volatile_counter;
    int total = 0;
    
    /* Level 1 */
    for (int i = 0; i < rows; i++) {
        /* Data-dependent condition */
        if ((outer_cond + i) % 4 == 0) {
            /* Level 2 */
            for (int j = 0; j < cols; j++) {
                volatile int mid_cond = g_volatile_float > 0.5f ? 1 : 0;
                
                /* Level 3 - innermost with complex operations */
                for (int k = 0; k < depth; k++) {
                    /* Non-contiguous access pattern */
                    int idx = (i * cols * depth + j * depth + k) % (rows * cols * depth);
                    
                    /* Mixed type operations */
                    if ((mid_cond + idx) % 2 == 0) {
                        array[idx].value = array[idx].weight * 2.0f;
                        total += array[idx].id;
                    } else {
                        array[idx].weight = array[idx].value * 0.5;
                        total -= array[idx].id;
                    }
                    
                    /* Additional floating point operation */
                    array[idx].timestamp += (int64_t)(array[idx].value * 1000.0);
                }
                
                /* Conditional store with volatile */
                if (g_volatile_counter++ % 11 == 0) {
                    array[j].tag = (char)(total % 256);
                }
            }
        }
    }
    
    return total;
}

/* Function 3: Mixed data type processing with stride */
__attribute__((optimize("O3")))
float mixed_type_stride_processing(struct MixedData* data, int size, int stride) {
    volatile float threshold = g_volatile_float;
    float sum = 0.0f;
    double prod_acc = 1.0;
    
    /* Process every 'stride'-th element */
    for (int i = 0; i < size; i += stride) {
        /* Complex conditional based on multiple factors */
        if ((i % 3 == 0) && (threshold > 0.3f) && (g_volatile_counter % 7 != 0)) {
            /* Floating point intensive path */
            data[i].value = (float)(data[i].weight * prod_acc);
            sum += data[i].value * 0.7f;
            prod_acc *= 0.99;
        } else {
            /* Integer intensive path */
            data[i].id = (data[i].id * 31 + i) % 1024;
            sum -= (float)data[i].id * 0.01f;
        }
        
        /* Additional nested conditional */
        if (i % 100 == 0 && stride > 1) {
            for (int j = 1; j < stride && (i + j) < size; j++) {
                data[i + j].weight = data[i].weight * 0.5;
                sum += (float)data[i + j].weight;
            }
        }
    }
    
    return sum;
}

/* Simple pseudo-random generator to avoid library dependencies */
static uint32_t lcg_state = 123456789;
uint32_t lcg_rand() {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Initialize array with pseudo-random data */
void initialize_data(double* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = (double)(lcg_rand() % 1000) / 100.0;
    }
}

/* Initialize mixed data array */
void initialize_mixed_data(struct MixedData* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i].id = lcg_rand() % 10000;
        data[i].value = (float)(lcg_rand() % 1000) / 10.0f;
        data[i].weight = (double)(lcg_rand() % 1000) / 100.0;
        data[i].tag = (char)(lcg_rand() % 128);
        data[i].timestamp = lcg_rand();
    }
}

int main() {
    const int DATA_SIZE = 5000;
    const int MIXED_SIZE = 3000;
    const int ROWS = 50, COLS = 20, DEPTH = 15;
    
    /* Allocate and initialize data */
    double* array1 = (double*)malloc(DATA_SIZE * sizeof(double));
    struct MixedData* array2 = (struct MixedData*)malloc(MIXED_SIZE * sizeof(struct MixedData));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    initialize_data(array1, DATA_SIZE);
    initialize_mixed_data(array2, MIXED_SIZE);
    
    double result1 = 0.0;
    int result2 = 0;
    float result3 = 0.0f;
    
    /* Perform multiple computations to increase scheduler activity */
    for (int iteration = 0; iteration < 5; iteration++) {
        g_volatile_counter = iteration;
        g_volatile_float = (float)iteration * 0.1f;
        
        /* Call function 1 - reduction with carried dependency */
        result1 += reduction_with_carried_dependency(array1, DATA_SIZE);
        
        /* Call function 2 - deeply nested loops */
        result2 += nested_loops_complex(array2, ROWS, COLS, DEPTH);
        
        /* Call function 3 - mixed type with stride */
        result3 += mixed_type_stride_processing(array2, MIXED_SIZE, 3 + (iteration % 5));
    }
    
    /* Combine and print results to prevent dead code elimination */
    printf("Results: reduction=%.6f, nested=%d, mixed=%.6f\n", 
           result1, result2, result3);
    
    /* Additional volatile operations to maintain control flow complexity */
    volatile int final_check = g_volatile_counter;
    if (final_check % 2 == 0) {
        printf("Even iteration path taken\n");
    } else {
        printf("Odd iteration path taken\n");
    }
    
    free(array1);
    free(array2);
    
    return 0;
}
