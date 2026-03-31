/* Complex loop patterns to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 10000
#define INNER_LOOP 50
#define MIDDLE_LOOP 20
#define OUTER_LOOP 10

/* Volatile variables to prevent optimization */
volatile int v_cond1 = 0;
volatile int v_cond2 = 1;
volatile float v_float = 3.14f;
volatile double v_double = 2.71828;

/* Mixed data type structure with non-contiguous access pattern */
struct MixedData {
    int id;
    float value;
    double weight;
    char tag;
    int counter;
};

/* Function 1: Reduction with carried dependency across iterations */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
double reduction_with_dependency(struct MixedData* data, int n) {
    double acc = 0.0;
    double prev = data[0].weight;
    
    /* Complex loop with data-dependent control flow */
    for (int i = 1; i < n; i += (v_cond1 ? 1 : 2)) {
        /* Volatile condition prevents optimization */
        if (v_cond2 || (rand() % 100) > 50) {
            /* Carried dependency: uses previous iteration's value */
            double current = data[i].weight;
            acc += prev * current * (double)data[i].value;
            prev = current;
            
            /* Mixed operations to generate diverse RTL */
            data[i].counter = (int)(acc * 0.01) + i;
            data[i].tag = (char)((int)acc % 26 + 'A');
        } else {
            /* Alternative path with different operations */
            acc -= data[i].value * 0.5f;
            data[i].id = (int)acc ^ i;
        }
        
        /* Additional volatile-dependent computation */
        if (v_float > 3.0f) {
            acc *= 1.0001;
        }
    }
    return acc;
}

/* Function 2: Nested loops with volatile conditions */
__attribute__((optimize("O3", "funroll-loops")))
float nested_loops_complex(int* arr, float* farr, int size) {
    float total = 0.0f;
    int counter = 0;
    
    /* Triple nested loop structure */
    for (int i = 0; i < OUTER_LOOP; i++) {
        /* Outer loop with volatile condition */
        if (v_cond1 || (i % 3) == 0) {
            for (int j = 0; j < MIDDLE_LOOP; j++) {
                /* Middle loop with data-dependent stride */
                int stride = (v_cond2 ? 1 : 2);
                for (int k = 0; k < INNER_LOOP; k += stride) {
                    /* Innermost loop with complex indexing */
                    int idx = (i * MIDDLE_LOOP * INNER_LOOP + 
                              j * INNER_LOOP + k) % size;
                    
                    /* Mixed integer/float operations */
                    if (arr[idx] % 7 == 0) {
                        total += farr[idx] * (float)arr[idx];
                        counter++;
                    } else if (arr[idx] % 5 == 0) {
                        total -= farr[idx] / 2.0f;
                        counter--;
                    }
                    
                    /* Volatile-dependent operation */
                    if (v_double > 2.0) {
                        farr[idx] = total * 0.99f;
                    }
                }
                
                /* Conditional store based on volatile */
                if (v_cond1 && (j % 4 == 0)) {
                    arr[j % size] = counter;
                }
            }
        }
    }
    return total;
}

/* Function 3: Non-contiguous access with mixed types */
__attribute__((optimize("O2", "fsel-sched-pipelining-outer-loops")))
double non_contiguous_access(struct MixedData* data, int n) {
    double sum_weights = 0.0;
    float sum_values = 0.0f;
    long long checksum = 0;
    
    /* Process every 3rd element with non-unit stride */
    for (int i = 0; i < n; i += 3) {
        /* Complex condition with volatile */
        if ((v_cond1 && data[i].value > 0) || 
            (!v_cond1 && data[i].id % 3 == 0)) {
            
            /* Mixed type operations */
            sum_weights += data[i].weight * (i % 10 + 1);
            sum_values += data[i].value * data[i].weight;
            
            /* Bit manipulation */
            checksum ^= (long long)data[i].id << 32;
            checksum ^= *(int*)&data[i].value;
            
            /* Conditional update based on volatile float */
            if (v_float < 4.0f) {
                data[i].counter = (int)sum_weights % 1000;
            }
        }
        
        /* Additional processing for adjacent elements */
        if (i + 1 < n && v_cond2) {
            data[i + 1].value = (float)sum_weights * 0.001f;
        }
    }
    
    return sum_weights + (double)sum_values + (double)checksum;
}

/* Function 4: Deeply nested with reduction pattern */
__attribute__((hot, optimize("O3")))
int complex_reduction(int* matrix, int rows, int cols) {
    int total = 0;
    volatile int v_local = rows % 7;
    
    for (int r = 0; r < rows; r++) {
        /* Row loop with volatile-dependent bound */
        int limit = cols - (v_local % 3);
        for (int c = 0; c < limit; c++) {
            /* Column loop with complex indexing */
            int idx = r * cols + c;
            
            /* Reduction with multiple dependencies */
            int val = matrix[idx];
            if (val > 0) {
                total += val * (r + 1);
                
                /* Nested conditional with volatile */
                if (v_cond1 || val % 11 == 0) {
                    for (int k = 0; k < 3; k++) {
                        /* Innermost tiny loop */
                        total -= (val >> k) & 1;
                        
                        /* Volatile access in innermost loop */
                        if (v_cond2 && (k == 1)) {
                            matrix[idx] = total % 256;
                        }
                    }
                }
            } else {
                total -= (-val) * (c + 1);
            }
            
            /* Periodic update based on volatile */
            if (v_float > 2.5f && c % 13 == 0) {
                matrix[idx] = total % 1000;
            }
        }
    }
    return total;
}

/* Simple pseudo-random generator to avoid library dependencies */
static unsigned int seed = 123456789;
unsigned int simple_rand() {
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
}

/* Initialize data with pseudo-random values */
void initialize_data(struct MixedData* data, int* int_arr, float* float_arr, int size) {
    for (int i = 0; i < size; i++) {
        data[i].id = simple_rand() % 1000;
        data[i].value = (float)(simple_rand() % 1000) / 10.0f;
        data[i].weight = (double)(simple_rand() % 1000) / 100.0;
        data[i].tag = (char)('A' + (simple_rand() % 26));
        data[i].counter = 0;
        
        int_arr[i] = (int)simple_rand() % 1000 - 500;
        float_arr[i] = (float)(simple_rand() % 1000) / 100.0f - 5.0f;
    }
}

int main() {
    /* Allocate and initialize data */
    struct MixedData* mixed_data = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    float* float_array = (float*)malloc(SIZE * sizeof(float));
    int* matrix = (int*)malloc(100 * 100 * sizeof(int));
    
    if (!mixed_data || !int_array || !float_array || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    initialize_data(mixed_data, int_array, float_array, SIZE);
    
    /* Initialize matrix for reduction */
    for (int i = 0; i < 100 * 100; i++) {
        matrix[i] = simple_rand() % 200 - 100;
    }
    
    /* Call all computation functions to trigger various scheduling scenarios */
    double result1 = reduction_with_dependency(mixed_data, SIZE);
    printf("Result 1 (reduction with dependency): %.6f\n", result1);
    
    float result2 = nested_loops_complex(int_array, float_array, SIZE);
    printf("Result 2 (nested loops): %.6f\n", result2);
    
    double result3 = non_contiguous_access(mixed_data, SIZE);
    printf("Result 3 (non-contiguous access): %.6f\n", result3);
    
    int result4 = complex_reduction(matrix, 100, 100);
    printf("Result 4 (complex reduction): %d\n", result4);
    
    /* Combine results to ensure all computations are live */
    double final_result = result1 + result2 + result3 + result4;
    printf("Final combined result: %.6f\n", final_result);
    
    /* Cleanup */
    free(mixed_data);
    free(int_array);
    free(float_array);
    free(matrix);
    
    return 0;
}
