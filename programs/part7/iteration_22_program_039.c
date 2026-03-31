/* Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 2000
#define INNER_LOOP 50
#define MIDDLE_LOOP 30
#define OUTER_LOOP 20

/* Volatile variables to prevent optimization of control flow */
volatile int vol_cond1 = 0;
volatile int vol_cond2 = 1;
volatile float vol_float = 3.14f;

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
double reduction_with_dependency(double* data, int n) {
    double acc = 0.0;
    double prev = data[0];
    
    /* Complex loop with data-dependent control flow */
    for (int i = 1; i < n; i++) {
        /* Volatile condition prevents optimization */
        if (vol_cond1 || (rand() % 100) > 50) {
            /* Carried dependency: uses value from previous iteration */
            acc = acc + data[i] * prev;
            
            /* Mixed operations to create diverse RTL */
            double temp = data[i] * vol_float;
            acc += (temp > 100.0) ? temp * 0.5 : temp * 0.3;
        } else {
            /* Alternative path with different operations */
            acc = acc - data[i] / (prev + 1.0);
        }
        
        /* Update previous value for next iteration */
        prev = data[i];
        
        /* Additional volatile check */
        if (vol_cond2 && (i % 7 == 0)) {
            acc *= 1.01;
        }
    }
    
    return acc;
}

/* Function 2: Nested loops with mixed data types and non-contiguous access */
__attribute__((optimize("O3", "funroll-loops")))
float process_mixed_data(struct MixedData* data, int count) {
    float total = 0.0f;
    double weight_acc = 0.0;
    
    /* Triple nested loop structure */
    for (int outer = 0; outer < OUTER_LOOP; outer++) {
        /* Middle loop with volatile condition */
        for (int middle = 0; middle < MIDDLE_LOOP; middle++) {
            if (vol_cond1 || (rand() % 100) > 75) {
                /* Innermost loop with non-contiguous access (stride 3) */
                for (int inner = 0; inner < INNER_LOOP; inner++) {
                    int idx = (outer * MIDDLE_LOOP * INNER_LOOP + 
                              middle * INNER_LOOP + inner) % count;
                    
                    /* Process every 3rd element */
                    if (idx % 3 == 0) {
                        /* Mixed type operations */
                        total += data[idx].value * data[idx].weight;
                        
                        /* Conditional store based on volatile */
                        if (vol_cond2 || data[idx].counter > 100) {
                            data[idx].weight *= 1.05;
                        }
                        
                        /* Complex floating point operation */
                        weight_acc += (data[idx].id % 2 == 0) ? 
                                     data[idx].weight * 0.8 : 
                                     data[idx].weight * 1.2;
                    }
                }
            } else {
                /* Alternative path for scheduler to consider */
                for (int inner = 0; inner < INNER_LOOP / 2; inner++) {
                    int idx = (middle * INNER_LOOP + inner) % count;
                    total -= data[idx].value * 0.5f;
                }
            }
        }
    }
    
    return total + (float)weight_acc;
}

/* Function 3: Deeply nested loops with data-dependent branches */
__attribute__((optimize("O2", "fsel-sched-pipelining-outer-loops")))
int complex_nested_computation(int* arr, int n) {
    int result = 0;
    
    /* Four-level nested loop */
    for (int a = 0; a < 10; a++) {
        /* Volatile prevents loop invariant code motion */
        volatile int local_vol = a % 3;
        
        for (int b = 0; b < 15; b++) {
            for (int c = 0; c < 20; c++) {
                /* Innermost loop with reduction pattern */
                for (int d = 0; d < n; d++) {
                    /* Data-dependent branch with volatile */
                    if ((local_vol > 0) || (arr[d] % (b + 1)) > (c % 5)) {
                        /* Complex integer arithmetic with carried dependency */
                        result = result ^ (arr[d] * (result & 0xFF));
                        
                        /* Additional operation to create scheduling pressure */
                        result += (arr[d] > 0) ? arr[d] : -arr[d];
                    } else {
                        /* Alternative computation path */
                        result = result | (arr[d] << (d % 8));
                    }
                    
                    /* Periodic operation with volatile check */
                    if (vol_cond2 && (d % 13 == 0)) {
                        result = result * 1103515245 + 12345;
                    }
                }
            }
            
            /* Inter-loop operation */
            result = (result * 1664525 + 1013904223) & 0x7FFFFFFF;
        }
    }
    
    return result;
}

/* Function 4: Matrix-like operations with software pipelining potential */
__attribute__((optimize("O3")))
void matrix_style_computation(float* src, float* dst, int rows, int cols) {
    /* Two-dimensional processing with carried dependencies */
    for (int i = 1; i < rows - 1; i++) {
        for (int j = 1; j < cols - 1; j++) {
            int idx = i * cols + j;
            
            /* Stencil computation with multiple dependencies */
            dst[idx] = (src[idx - cols] +    /* above */
                       src[idx - 1] +        /* left */
                       src[idx] * 4.0f +     /* center */
                       src[idx + 1] +        /* right */
                       src[idx + cols]) / 8.0f; /* below */
            
            /* Conditional operation based on volatile */
            if (vol_cond1 || dst[idx] > 0.5f) {
                dst[idx] = dst[idx] * dst[idx - cols] * vol_float;
            }
        }
    }
}

/* Simple pseudo-random generator to avoid library dependencies */
unsigned int lcg_seed = 1;
unsigned int lcg_rand() {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return (unsigned int)(lcg_seed / 65536) % 32768;
}

/* Initialize arrays with pseudo-random data */
void initialize_data(double* dbl_arr, int* int_arr, struct MixedData* mixed_arr, int n) {
    for (int i = 0; i < n; i++) {
        dbl_arr[i] = (lcg_rand() % 1000) / 100.0;
        int_arr[i] = lcg_rand() % 10000;
        
        mixed_arr[i].id = i;
        mixed_arr[i].value = (lcg_rand() % 1000) / 10.0f;
        mixed_arr[i].weight = (lcg_rand() % 500) / 100.0;
        mixed_arr[i].tag = 'A' + (i % 26);
        mixed_arr[i].counter = lcg_rand() % 200;
    }
}

int main() {
    /* Allocate and initialize data */
    double* double_data = (double*)malloc(SIZE * sizeof(double));
    int* int_data = (int*)malloc(SIZE * sizeof(int));
    struct MixedData* mixed_data = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    float* src_matrix = (float*)malloc(SIZE * sizeof(float));
    float* dst_matrix = (float*)malloc(SIZE * sizeof(float));
    
    if (!double_data || !int_data || !mixed_data || !src_matrix || !dst_matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    initialize_data(double_data, int_data, mixed_data, SIZE);
    for (int i = 0; i < SIZE; i++) {
        src_matrix[i] = (lcg_rand() % 1000) / 100.0f;
    }
    
    /* Call all computation functions to exercise different scheduler paths */
    double result1 = reduction_with_dependency(double_data, SIZE);
    printf("Result 1 (reduction): %f\n", result1);
    
    float result2 = process_mixed_data(mixed_data, SIZE);
    printf("Result 2 (mixed data): %f\n", result2);
    
    int result3 = complex_nested_computation(int_data, SIZE);
    printf("Result 3 (nested): %d\n", result3);
    
    matrix_style_computation(src_matrix, dst_matrix, 50, 40);
    printf("Matrix computation completed\n");
    
    /* Combine results to ensure all computations are live */
    double final_result = result1 + result2 + result3 + dst_matrix[SIZE/2];
    printf("Final combined result: %f\n", final_result);
    
    /* Cleanup */
    free(double_data);
    free(int_data);
    free(mixed_data);
    free(src_matrix);
    free(dst_matrix);
    
    return 0;
}
