/* Complex test program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 2048
#define INNER_ITERS 100
#define MIDDLE_ITERS 50
#define OUTER_ITERS 20

/* Volatile variables to prevent optimization of control flow */
volatile int g_volatile_counter = 0;
volatile float g_volatile_float = 1.5f;

/* Mixed data type structure for complex access patterns */
struct MixedData {
    int id;
    float value;
    double weight;
    char tag;
    int data[3];
};

/* Function with complex loop nest and carried dependency */
__attribute__((optimize("O2")))
double complex_reduction_with_dependency(struct MixedData* arr, int n) {
    double acc = 0.0;
    volatile int v = g_volatile_counter;
    
    /* Outer loop */
    for (int i = 1; i < OUTER_ITERS; i++) {
        /* Middle loop with volatile condition */
        for (int j = (v % 3) + 1; j < MIDDLE_ITERS; j += (rand() % 2) + 1) {
            /* Innermost loop with data-dependent carried dependency */
            for (int k = 2; k < INNER_ITERS; k++) {
                /* Complex reduction with carried dependency across iterations */
                int idx = (i * j + k) % n;
                int prev_idx = (i * j + k - 1) % n;
                
                /* Mixed type operations */
                double temp = arr[idx].weight * arr[prev_idx].value;
                acc = acc + temp;
                
                /* Conditional store based on volatile */
                if (g_volatile_float > 1.0f) {
                    arr[idx].data[k % 3] = (int)(acc * 100.0);
                }
                
                /* More arithmetic to create scheduling pressure */
                arr[idx].value = (float)(arr[idx].value * 1.01f + 
                                        arr[prev_idx].weight * 0.5);
            }
            
            /* Volatile-dependent branch in middle loop */
            if (v++ % 7 == 0) {
                acc *= 0.99;
            }
        }
        
        /* Non-contiguous memory access pattern */
        for (int stride = i; stride < n; stride += 13) {
            arr[stride].weight = arr[stride].weight * 0.95 + 
                                arr[(stride + 7) % n].value;
        }
    }
    
    return acc;
}

/* Function with deeply nested loops and mixed operations */
__attribute__((optimize("O2")))
float nested_mixed_operations(int* int_arr, float* float_arr, double* double_arr, int n) {
    float result = 0.0f;
    volatile int cond = g_volatile_counter;
    
    /* Triple nested loop with data-dependent conditions */
    for (int a = 0; a < 15; a++) {
        int a_mod = (cond + a) % 5;
        
        for (int b = a_mod; b < 25; b += (rand() % 3) + 1) {
            float b_scale = (float)(b) * 0.1f;
            
            for (int c = 0; c < 35; c++) {
                int idx = (a * b + c) % n;
                
                /* Mixed type computations */
                double dbl_val = double_arr[idx] * (1.0 + b_scale);
                float flt_val = float_arr[idx] * (float)dbl_val;
                int int_val = int_arr[idx] * (int)(flt_val * 10.0f);
                
                /* Conditional accumulation */
                if ((cond + idx) % 11 == 0) {
                    result += flt_val;
                } else if ((cond + idx) % 13 == 0) {
                    result -= flt_val * 0.5f;
                }
                
                /* Store results with stride */
                if (c % 4 == 0) {
                    int_arr[(idx + 7) % n] = int_val % 1000;
                    float_arr[(idx + 11) % n] = flt_val * 0.9f;
                }
            }
            
            /* Volatile check in middle loop */
            if (g_volatile_float > 0.0f) {
                result *= 0.999f;
            }
        }
    }
    
    return result;
}

/* Function with pointer chasing and complex control flow */
__attribute__((optimize("O3")))
int pointer_chasing_reduction(struct MixedData* arr, int n) {
    int sum = 0;
    volatile int start = g_volatile_counter % n;
    
    /* Outer loop with pointer chasing */
    for (int iter = 0; iter < 100; iter++) {
        struct MixedData* current = &arr[start];
        int steps = 0;
        
        /* Inner loop with data-dependent pointer chasing */
        while (steps < 50 && current != NULL) {
            /* Complex address calculation */
            int next_idx = (current->id + iter + steps) % n;
            
            /* Reduction with mixed types */
            sum += current->data[steps % 3];
            sum += (int)(current->value * 100.0f);
            
            /* Conditional pointer update */
            if ((sum + iter) % 3 == 0) {
                current = &arr[next_idx];
            } else if ((sum + iter) % 5 == 0) {
                current = &arr[(next_idx + 17) % n];
            } else {
                current = &arr[(next_idx + 31) % n];
            }
            
            steps++;
            
            /* Volatile-dependent operation */
            if (g_volatile_counter++ % 19 == 0) {
                sum -= 100;
            }
        }
        
        /* Update start for next iteration */
        start = (start * 13 + 7) % n;
    }
    
    return sum;
}

/* Simple PRNG for initialization (avoiding library dependencies) */
static unsigned int prng_state = 123456789;
unsigned int simple_rand() {
    prng_state = prng_state * 1103515245 + 12345;
    return (unsigned int)(prng_state / 65536) % 32768;
}

/* Initialize arrays with pseudo-random data */
void initialize_data(struct MixedData* arr, int* int_arr, float* float_arr, 
                     double* double_arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i].id = i;
        arr[i].value = (float)(simple_rand() % 1000) / 10.0f;
        arr[i].weight = (double)(simple_rand() % 10000) / 100.0;
        arr[i].tag = 'A' + (i % 26);
        
        for (int j = 0; j < 3; j++) {
            arr[i].data[j] = simple_rand() % 100;
        }
        
        int_arr[i] = simple_rand() % 1000;
        float_arr[i] = (float)(simple_rand() % 10000) / 100.0f;
        double_arr[i] = (double)(simple_rand() % 100000) / 1000.0;
    }
}

int main() {
    const int data_size = SIZE;
    
    /* Allocate and initialize data */
    struct MixedData* mixed_arr = (struct MixedData*)malloc(data_size * sizeof(struct MixedData));
    int* int_arr = (int*)malloc(data_size * sizeof(int));
    float* float_arr = (float*)malloc(data_size * sizeof(float));
    double* double_arr = (double*)malloc(data_size * sizeof(double));
    
    if (!mixed_arr || !int_arr || !float_arr || !double_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    initialize_data(mixed_arr, int_arr, float_arr, double_arr, data_size);
    
    /* Update volatile to affect control flow */
    g_volatile_counter = simple_rand() % 100;
    g_volatile_float = (float)(simple_rand() % 100) / 50.0f;
    
    /* Call complex functions to trigger scheduler activity */
    double result1 = complex_reduction_with_dependency(mixed_arr, data_size);
    
    g_volatile_counter += 17;
    g_volatile_float += 0.3f;
    
    float result2 = nested_mixed_operations(int_arr, float_arr, double_arr, data_size);
    
    g_volatile_counter = (g_volatile_counter * 3) % 100;
    
    int result3 = pointer_chasing_reduction(mixed_arr, data_size);
    
    /* Combine results to ensure all computations are used */
    double final_result = result1 + result2 + result3;
    
    /* Print result to prevent dead code elimination */
    printf("Final combined result: %f\n", final_result);
    
    /* Cleanup */
    free(mixed_arr);
    free(int_arr);
    free(float_arr);
    free(double_arr);
    
    return 0;
}
