/* Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 10000
#define INNER_LOOP 50
#define MIDDLE_LOOP 20
#define OUTER_LOOP 10

/* Mixed data type structure */
struct MixedData {
    int id;
    float value;
    double precision;
    char tag;
    volatile int flag; /* volatile to prevent optimization */
};

/* Global volatile variables to create data-dependent control flow */
volatile int global_counter = 0;
volatile int global_seed = 12345;

/* Simple PRNG to avoid library dependencies */
static inline int simple_rand(int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

/* Function 1: Reduction with carried dependency across iterations */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
double reduction_with_carried_dep(double *data, int n) {
    double acc = 0.0;
    double prev = data[0];
    
    /* Complex loop with carried dependency */
    for (int i = 1; i < n; i++) {
        /* Data-dependent condition using volatile */
        if (global_counter++ % 7 == 0) {
            acc += data[i] * prev;
        } else {
            acc += data[i] / (prev + 1.0);
        }
        
        /* Non-trivial arithmetic creating scheduling pressure */
        double temp = data[i] * 1.2345;
        temp = temp / (i + 1);
        
        /* Conditional store based on volatile */
        if (global_counter % 13 == 0) {
            data[i] = temp;
        }
        
        prev = data[i];
        
        /* Inner loop to create more scheduling complexity */
        for (int j = 0; j < INNER_LOOP; j++) {
            acc += (j % 2 == 0) ? 0.001 : -0.001;
        }
    }
    
    return acc;
}

/* Function 2: Mixed data types with non-contiguous access */
__attribute__((optimize("O3", "funroll-loops")))
float process_mixed_data(struct MixedData *data, int n) {
    float total = 0.0f;
    int local_seed = global_seed;
    
    /* Process every 3rd element with non-unit stride */
    for (int i = 0; i < n; i += 3) {
        /* Volatile condition prevents optimization */
        if (simple_rand(&local_seed) % 5 == 0) {
            total += data[i].value * 2.0f;
        } else {
            total -= data[i].value / 2.0f;
        }
        
        /* Mixed type operations */
        data[i].precision = (double)data[i].value * 1.5;
        data[i].id = i * simple_rand(&local_seed);
        
        /* Middle loop with volatile condition */
        for (int j = 0; j < MIDDLE_LOOP; j++) {
            volatile int cond = simple_rand(&local_seed) % 11;
            if (cond < 3) {
                total += j * 0.01f;
            } else if (cond < 7) {
                total -= j * 0.005f;
            }
            
            /* Additional arithmetic pressure */
            data[i].tag = (char)((data[i].id + j) % 256);
        }
        
        /* Outer loop in the same function */
        for (int k = 0; k < OUTER_LOOP; k++) {
            if (k % 4 == 0) {
                data[i].flag = k;
            }
        }
    }
    
    global_seed = local_seed;
    return total;
}

/* Function 3: Deeply nested loops with complex control flow */
__attribute__((hot, optimize("O2")))
int nested_loops_complex(int *arr, int n) {
    int result = 0;
    volatile int v1 = 1, v2 = 2, v3 = 3;
    
    /* Triple nested loop */
    for (int i = 0; i < n; i++) {
        /* Data-dependent condition using volatile */
        if ((v1++ % 17) == 0) {
            for (int j = 0; j < MIDDLE_LOOP; j++) {
                /* Another volatile condition */
                if ((v2++ % 23) < 11) {
                    for (int k = 0; k < INNER_LOOP; k++) {
                        /* Complex conditional with mixed operations */
                        if ((v3++ % 29) == 0) {
                            result += arr[i] * j * k;
                        } else {
                            result -= arr[i] / (j + k + 1);
                        }
                        
                        /* Floating point in integer loop */
                        float ftemp = (float)result / 1000.0f;
                        if (ftemp > 100.0f) {
                            result = (int)(ftemp);
                        }
                    }
                } else {
                    /* Alternative path with different operations */
                    result ^= (arr[i] << (j % 16));
                }
            }
        } else {
            /* Different computation pattern */
            result |= arr[i];
        }
        
        /* Memory access pattern that's hard to predict */
        if (i % 7 == 0) {
            arr[i] = result % 1000;
        } else if (i % 13 == 0) {
            arr[i] = -result;
        }
    }
    
    return result;
}

/* Function 4: Matrix-like operations with reductions */
__attribute__((optimize("O3", "fsel-sched-pipelining-outer-loops")))
double matrix_reduction(double *mat, int rows, int cols) {
    double total = 0.0;
    int seed = 4567;
    
    for (int i = 0; i < rows; i++) {
        double row_sum = 0.0;
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            
            /* Data-dependent computation */
            if (simple_rand(&seed) % 3 == 0) {
                row_sum += mat[idx] * 1.5;
            } else {
                row_sum -= mat[idx] * 0.5;
            }
            
            /* Create loop-carried dependency */
            if (j > 0) {
                mat[idx] += mat[idx - 1] * 0.1;
            }
            
            /* Conditional store */
            if (simple_rand(&seed) % 7 == 0) {
                mat[idx] = row_sum / (j + 1);
            }
        }
        
        total += row_sum;
        
        /* Innermost reduction loop */
        for (int k = 0; k < 5; k++) {
            total = total * 0.99 + k * 0.01;
        }
    }
    
    return total;
}

int main() {
    /* Allocate and initialize data */
    double *data1 = (double*)malloc(SIZE * sizeof(double));
    struct MixedData *data2 = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    int *data3 = (int*)malloc(SIZE * sizeof(int));
    double *matrix = (double*)malloc(100 * 100 * sizeof(double));
    
    /* Initialize with pseudo-random values */
    int seed = 42;
    for (int i = 0; i < SIZE; i++) {
        data1[i] = (double)simple_rand(&seed) / 1000.0;
        data2[i].value = (float)simple_rand(&seed) / 500.0f;
        data2[i].id = i;
        data2[i].precision = data1[i];
        data2[i].tag = (char)(i % 128);
        data2[i].flag = 0;
        data3[i] = simple_rand(&seed) % 1000;
    }
    
    for (int i = 0; i < 100 * 100; i++) {
        matrix[i] = (double)simple_rand(&seed) / 800.0;
    }
    
    /* Call all computation functions to create scheduling pressure */
    double result1 = reduction_with_carried_dep(data1, SIZE);
    float result2 = process_mixed_data(data2, SIZE);
    int result3 = nested_loops_complex(data3, SIZE / 10);
    double result4 = matrix_reduction(matrix, 100, 100);
    
    /* Combine results to ensure they're used */
    double final_result = result1 + result2 + result3 + result4;
    
    printf("Final combined result: %f\n", final_result);
    printf("Global counter: %d\n", global_counter);
    printf("Global seed: %d\n", global_seed);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    free(matrix);
    
    return 0;
}
