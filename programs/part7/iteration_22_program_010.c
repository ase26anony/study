/* test_sel_sched.c - Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Mixed data type structure with non-contiguous access patterns */
struct mixed_data {
    int index;
    float f_value;
    double d_value;
    volatile int flag;  /* volatile to prevent optimization */
};

/* Global volatile variables to create data-dependent control flow */
volatile int g_volatile_counter = 0;
volatile int g_volatile_threshold = 100;

/* Function with complex loop nest and reduction pattern */
__attribute__((optimize("O2")))
double complex_reduction(struct mixed_data* data, int size) {
    double acc = 0.0;
    int i, j, k;
    
    /* Outer loop - creates scheduling region */
    for (i = 0; i < size - 1; i++) {
        /* Middle loop with volatile condition */
        for (j = 0; j < 3; j++) {
            /* Innermost loop with data-dependent branch */
            for (k = 0; k < 2; k++) {
                /* Volatile read to prevent optimization */
                int volatile_flag = g_volatile_counter;
                
                /* Data-dependent conditional with side effect */
                if (volatile_flag % (i + j + k + 2) == 0) {
                    /* Carried dependency across iterations */
                    acc += data[i].d_value * data[i + 1].f_value;
                    
                    /* Mixed type operations */
                    data[i].index = (int)(acc * 0.1);
                    
                    /* Non-contiguous memory access */
                    if (i % 3 == 0) {
                        acc += data[i * 2 % size].d_value;
                    }
                } else {
                    /* Alternative computation path */
                    acc -= data[i].f_value * 0.5;
                }
                
                /* Complex conditional store */
                if (acc > 1000.0 && volatile_flag < g_volatile_threshold) {
                    data[i].flag = 1;
                }
            }
            
            /* Additional computation to increase basic block size */
            acc = acc * 0.99 + (j * 0.01);
        }
        
        /* Cross-iteration dependency with varying stride */
        if (i > 0) {
            acc += data[i - 1].d_value * 0.1;
        }
        
        /* Periodic volatile update */
        if (i % 7 == 0) {
            g_volatile_counter++;
        }
    }
    
    return acc;
}

/* Function with deeply nested loops and mixed operations */
__attribute__((optimize("O2")))
float nested_mixed_ops(int* int_arr, float* float_arr, int size) {
    float result = 0.0f;
    int i, j, k;
    
    /* Triple nested loop with volatile conditions */
    for (i = 0; i < size / 4; i++) {
        volatile int v1 = g_volatile_counter;
        
        for (j = 0; j < 4; j++) {
            volatile int v2 = g_volatile_threshold;
            
            for (k = 0; k < 3; k++) {
                /* Data-dependent branch with side effects */
                if ((v1 + v2 + i + j + k) % 5 == 0) {
                    /* Mixed integer/float operations */
                    result += int_arr[i * 4 + j] * float_arr[k];
                    
                    /* Conditional store with non-unit stride */
                    if (result > 0) {
                        int_arr[(i * 3 + k) % size] = (int)result;
                    }
                } else {
                    /* Alternative computation path */
                    result -= float_arr[j] / (k + 1);
                }
                
                /* Complex expression with multiple operations */
                result = result * 0.9f + float_arr[(i + j + k) % size] * 0.1f;
            }
            
            /* Loop-carried dependency */
            if (j > 0) {
                result += float_arr[j - 1] * 0.01f;
            }
        }
        
        /* Update volatile variable */
        if (i % 11 == 0) {
            g_volatile_threshold += 2;
        }
    }
    
    return result;
}

/* Function with pointer chasing and complex dependencies */
__attribute__((optimize("O3")))
double pointer_chasing_reduction(struct mixed_data* data, int size) {
    double sum = 0.0;
    int i;
    
    /* Loop with pointer arithmetic and complex indexing */
    for (i = 0; i < size * 2; i += 2) {
        int idx = (i * 3) % size;
        volatile int cond = g_volatile_counter;
        
        /* Multiple dependent operations */
        double temp = data[idx].d_value;
        
        if (cond % 3 == 0) {
            temp *= data[(idx + 1) % size].f_value;
            sum += temp * 1.5;
        } else if (cond % 3 == 1) {
            temp /= data[(idx + 2) % size].f_value + 1.0;
            sum += temp * 0.5;
        } else {
            temp = temp - data[(idx + 3) % size].d_value;
            sum += temp;
        }
        
        /* Conditional store with data-dependent address */
        if (sum > 500.0) {
            int store_idx = (i * 5) % size;
            data[store_idx].index = (int)sum;
            data[store_idx].flag = cond;
        }
        
        /* Update volatile */
        if (i % 13 == 0) {
            g_volatile_counter = (g_volatile_counter + 1) % 100;
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

/* Initialize data with pseudo-random values */
void init_data(struct mixed_data* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i].index = lcg_rand() % 1000;
        data[i].f_value = (lcg_rand() % 10000) / 100.0f;
        data[i].d_value = (lcg_rand() % 20000) / 100.0;
        data[i].flag = lcg_rand() % 2;
    }
}

void init_arrays(int* int_arr, float* float_arr, int size) {
    for (int i = 0; i < size; i++) {
        int_arr[i] = lcg_rand() % 1000;
        float_arr[i] = (lcg_rand() % 1000) / 10.0f;
    }
}

int main() {
    const int DATA_SIZE = 2048;  /* Large enough for non-trivial scheduling */
    
    /* Allocate and initialize data */
    struct mixed_data* data = malloc(DATA_SIZE * sizeof(struct mixed_data));
    int* int_arr = malloc(DATA_SIZE * sizeof(int));
    float* float_arr = malloc(DATA_SIZE * sizeof(float));
    
    if (!data || !int_arr || !float_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_data(data, DATA_SIZE);
    init_arrays(int_arr, float_arr, DATA_SIZE);
    
    /* Reset volatile variables */
    g_volatile_counter = 0;
    g_volatile_threshold = 100;
    
    /* Call multiple computation functions to increase scheduling opportunities */
    double result1 = complex_reduction(data, DATA_SIZE);
    float result2 = nested_mixed_ops(int_arr, float_arr, DATA_SIZE);
    double result3 = pointer_chasing_reduction(data, DATA_SIZE);
    
    /* Combine results to ensure all computations are live */
    double final_result = result1 + result2 + result3;
    
    /* Use results to prevent dead code elimination */
    printf("Final result: %f\n", final_result);
    printf("Volatile counter: %d, threshold: %d\n", 
           g_volatile_counter, g_volatile_threshold);
    
    /* Cleanup */
    free(data);
    free(int_arr);
    free(float_arr);
    
    return 0;
}
