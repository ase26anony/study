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
    double acc = 0.0;
    double prev = data[0];
    
    /* Complex loop with data-dependent branching */
    for (int i = 1; i < size; i++) {
        /* Volatile read to force re-evaluation */
        int volatile_cond = g_volatile_counter++;
        
        /* Carried dependency: uses result from previous iteration */
        double current = data[i];
        double product = prev * current;
        
        /* Data-dependent conditional */
        if (volatile_cond % 7 == 0) {
            acc = acc + product * 0.5;
        } else if (volatile_cond % 13 == 0) {
            acc = acc - product * 0.3;
        } else {
            acc = acc + product;
        }
        
        /* Non-linear update pattern */
        prev = current + (volatile_cond % 3) * 0.1;
        
        /* Additional floating point operations */
        acc = acc / (1.0 + (i % 100) * 0.01);
    }
    
    return acc;
}

/* Function 2: Process mixed data types with non-contiguous access */
__attribute__((optimize("O2")))
float process_mixed_types(struct MixedData* data, int count, int stride) {
    float total_weight = 0.0f;
    double value_accumulator = 0.0;
    int id_sum = 0;
    
    /* Triple nested loop with volatile conditions */
    for (int outer = 0; outer < 3; outer++) {
        volatile int outer_cond = g_volatile_counter + outer;
        
        for (int middle = 0; middle < 5; middle++) {
            /* Access with non-unit stride */
            for (int i = 0; i < count; i += stride) {
                /* Volatile function call simulation */
                int idx = i + (outer_cond % 3);
                if (idx >= count) continue;
                
                /* Mixed type operations */
                struct MixedData* item = &data[idx];
                
                /* Data-dependent branching */
                if (item->id % 2 == (g_volatile_counter % 2)) {
                    total_weight += item->weight * item->value;
                    value_accumulator += item->value * 0.75;
                } else {
                    total_weight -= item->weight * 0.5;
                    value_accumulator -= item->value * 0.25;
                }
                
                /* Integer operations with carried dependency */
                id_sum = id_sum * 13 + item->id;
                
                /* Conditional store based on volatile */
                if (g_volatile_float > 0.5f) {
                    item->tag = 'A' + (id_sum % 26);
                }
                
                /* Floating point intensive computation */
                double temp = item->weight * item->value;
                total_weight += temp / (1.0 + middle * 0.1);
            }
            
            /* Inner loop volatile update */
            g_volatile_float += 0.1f;
        }
    }
    
    return total_weight + (float)value_accumulator;
}

/* Function 3: Deeply nested loops with complex reduction */
__attribute__((optimize("O3")))
int64_t complex_nested_reduction(int* matrix, int rows, int cols) {
    int64_t grand_total = 0;
    
    /* Four-level nested loop */
    for (int r = 0; r < rows; r++) {
        volatile int row_seed = g_volatile_counter + r;
        
        for (int c = 0; c < cols; c++) {
            /* Skip some iterations based on volatile condition */
            if ((row_seed + c) % 11 == 0) continue;
            
            for (int d = 0; d < 4; d++) {
                int base_idx = (r * cols + c) * 4 + d;
                
                for (int t = 0; t < 3; t++) {
                    /* Complex indexing with mixed operations */
                    int idx = base_idx + t;
                    if (idx >= rows * cols * 4) break;
                    
                    /* Data-dependent computation */
                    int val = matrix[idx];
                    int modified_val;
                    
                    /* Multiple conditional paths */
                    if (val % 3 == 0) {
                        modified_val = val * 2 + d;
                    } else if (val % 5 == 0) {
                        modified_val = val / 2 - t;
                    } else {
                        modified_val = val + r - c;
                    }
                    
                    /* Reduction with feedback */
                    grand_total = grand_total * 17 + modified_val;
                    
                    /* Conditional update with volatile check */
                    if (g_volatile_counter % (idx + 1) == 0) {
                        matrix[idx] = modified_val % 256;
                    }
                }
            }
            
            /* Inter-loop dependency */
            g_volatile_counter += (r * c) % 7;
        }
    }
    
    return grand_total;
}

/* Simple pseudo-random generator to avoid library dependencies */
static uint32_t lcg_state = 123456789;
uint32_t lcg_rand() {
    lcg_state = lcg_state * 1103515245 + 12345;
    return lcg_state;
}

/* Initialize data with pseudo-random values */
void initialize_data(double* array, int size, struct MixedData* mixed, int mixed_count) {
    for (int i = 0; i < size; i++) {
        array[i] = (lcg_rand() % 1000) / 100.0;
    }
    
    for (int i = 0; i < mixed_count; i++) {
        mixed[i].id = lcg_rand() % 10000;
        mixed[i].value = (lcg_rand() % 1000) / 10.0f;
        mixed[i].weight = (lcg_rand() % 1000) / 100.0;
        mixed[i].tag = 'A' + (lcg_rand() % 26);
        mixed[i].timestamp = lcg_rand();
    }
}

int main() {
    const int DATA_SIZE = 5000;
    const int MIXED_SIZE = 3000;
    const int MATRIX_ROWS = 64;
    const int MATRIX_COLS = 64;
    
    /* Allocate and initialize data */
    double* data_array = (double*)malloc(DATA_SIZE * sizeof(double));
    struct MixedData* mixed_array = (struct MixedData*)malloc(MIXED_SIZE * sizeof(struct MixedData));
    int* matrix_data = (int*)malloc(MATRIX_ROWS * MATRIX_COLS * 4 * sizeof(int));
    
    if (!data_array || !mixed_array || !matrix_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    initialize_data(data_array, DATA_SIZE, mixed_array, MIXED_SIZE);
    
    for (int i = 0; i < MATRIX_ROWS * MATRIX_COLS * 4; i++) {
        matrix_data[i] = lcg_rand() % 1000;
    }
    
    /* Execute all computation patterns to trigger various scheduler paths */
    double result1 = reduction_with_carried_dependency(data_array, DATA_SIZE);
    printf("Result 1 (carried dependency): %f\n", result1);
    
    float result2 = process_mixed_types(mixed_array, MIXED_SIZE, 3); /* stride = 3 */
    printf("Result 2 (mixed types): %f\n", result2);
    
    int64_t result3 = complex_nested_reduction(matrix_data, MATRIX_ROWS, MATRIX_COLS);
    printf("Result 3 (nested reduction): %lld\n", (long long)result3);
    
    /* Combine results to ensure all computations are live */
    double final_result = result1 + result2 + result3;
    printf("Final combined result: %f\n", final_result);
    
    /* Cleanup */
    free(data_array);
    free(mixed_array);
    free(matrix_data);
    
    return 0;
}
