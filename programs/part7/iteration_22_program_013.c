/* Complex loop patterns to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 2000
#define INNER_ITERS 50

/* Volatile variables to prevent optimization of control flow */
volatile int v_cond1 = 0;
volatile int v_cond2 = 1;
volatile float v_float_cond = 0.5f;

/* Mixed data type structure with non-contiguous access pattern */
struct MixedData {
    int id;
    float value;
    double weight;
    char tag;
    int counter;
};

/* Function with complex reduction and carried dependency */
__attribute__((optimize("O2")))
double complex_reduction(const double* data, int n) {
    double acc = 0.0;
    volatile int seed = 7;  /* Prevent optimization */
    
    /* Outer loop with data-dependent condition */
    for (int i = 1; i < n; i++) {
        /* Middle loop with volatile condition */
        for (int j = 0; j < INNER_ITERS; j++) {
            if (v_cond1 || (seed % 3 == 0)) {
                /* Innermost loop with reduction pattern */
                double temp = 0.0;
                for (int k = 0; k < 5; k++) {
                    /* Carried dependency across iterations */
                    temp = temp + data[i - 1] * data[i] * (k + 1);
                    /* Volatile read to prevent optimization */
                    if (v_cond2) {
                        temp *= 0.99;
                    }
                }
                acc += temp;
            }
            /* Modify seed to create varying control flow */
            seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        }
        
        /* Additional floating-point operation with volatile */
        if (v_float_cond > 0.3f) {
            acc += data[i] * 0.5;
        } else {
            acc -= data[i] * 0.2;
        }
    }
    
    return acc;
}

/* Function processing mixed data types with non-contiguous access */
__attribute__((optimize("O3")))
float process_mixed_types(struct MixedData* data, int n) {
    float total = 0.0f;
    volatile int skip = 2;  /* Non-unit stride */
    
    /* Triple nested loop with mixed operations */
    for (int outer = 0; outer < 3; outer++) {
        for (int i = 0; i < n; i += (skip + outer)) {
            /* Data-dependent conditional store */
            if (data[i].value > v_float_cond || v_cond1) {
                /* Mixed type operations */
                double weighted = data[i].weight * data[i].value;
                
                /* Innermost computation loop */
                for (int k = 0; k < 4; k++) {
                    /* Conditional based on volatile */
                    if (v_cond2 || (k % 2 == 0)) {
                        weighted *= 1.01 + (k * 0.001);
                        data[i].counter++;
                    }
                }
                
                total += (float)weighted;
                
                /* Conditional store with volatile dependency */
                if (v_float_cond < 0.8f) {
                    data[i].tag = (char)((int)data[i].tag + 1);
                }
            }
        }
        /* Modify volatile to change control flow */
        skip = (skip * 3) % 5;
    }
    
    return total;
}

/* Function with deeply nested loops and complex conditions */
__attribute__((optimize("O2")))
int nested_conditional_compute(int* array, int n) {
    int result = 0;
    volatile int mod_base = 3;
    
    /* Deeply nested loop structure */
    for (int a = 0; a < n / 100; a++) {
        for (int b = a; b < n / 50; b++) {
            /* Volatile-dependent condition */
            if ((a + b) % (mod_base + 1) == 0 || v_cond1) {
                for (int c = 0; c < 10; c++) {
                    /* Complex addressing with mixed operations */
                    int idx = (a * 17 + b * 13 + c) % n;
                    
                    /* Reduction with carried dependency */
                    int temp = array[idx];
                    for (int d = 0; d < 5; d++) {
                        /* Data-dependent computation */
                        if (v_cond2 || (temp % 2 == 0)) {
                            temp = temp * 1103515245 + 12345;
                            result ^= temp;
                        } else {
                            temp = temp / 3;
                            result += temp;
                        }
                        
                        /* Memory access with stride */
                        if (d % 2 == 0) {
                            array[(idx + d) % n] = temp % 100;
                        }
                    }
                }
            }
        }
        /* Update volatile to affect next iteration */
        mod_base = (mod_base + a) % 7;
    }
    
    return result;
}

/* Initialize data with pseudo-random values */
void initialize_data(double* double_data, struct MixedData* mixed_data, int* int_data, int n) {
    unsigned int seed = 123456789;
    
    for (int i = 0; i < n; i++) {
        /* Simple LCG for reproducibility */
        seed = seed * 1103515245 + 12345;
        
        double_data[i] = (double)(seed % 1000) / 100.0;
        int_data[i] = seed % 10000;
        
        mixed_data[i].id = i;
        mixed_data[i].value = (float)(seed % 500) / 50.0f;
        mixed_data[i].weight = (double)(seed % 200) / 25.0;
        mixed_data[i].tag = (char)((seed % 26) + 'A');
        mixed_data[i].counter = 0;
    }
}

int main() {
    /* Allocate and initialize data */
    double* double_data = (double*)malloc(SIZE * sizeof(double));
    struct MixedData* mixed_data = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    int* int_data = (int*)malloc(SIZE * sizeof(int));
    
    if (!double_data || !mixed_data || !int_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    initialize_data(double_data, mixed_data, int_data, SIZE);
    
    printf("Starting complex computations...\n");
    
    /* Call all computation functions to exercise different scheduler patterns */
    double result1 = complex_reduction(double_data, SIZE);
    printf("Result 1: %f\n", result1);
    
    float result2 = process_mixed_types(mixed_data, SIZE);
    printf("Result 2: %f\n", result2);
    
    int result3 = nested_conditional_compute(int_data, SIZE);
    printf("Result 3: %d\n", result3);
    
    /* Combine results to ensure all computations are used */
    double final_result = result1 + result2 + result3;
    printf("Final combined result: %f\n", final_result);
    
    /* Cleanup */
    free(double_data);
    free(mixed_data);
    free(int_data);
    
    return 0;
}
