/* Complex loop patterns to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 10000
#define INNER_LOOP 100
#define MIDDLE_LOOP 50
#define OUTER_LOOP 20

/* Volatile variables to prevent optimization of control flow */
volatile int g_volatile_counter = 0;
volatile float g_volatile_float = 0.5f;

/* Mixed data type structure with non-contiguous access pattern */
struct MixedData {
    int id;
    float value;
    double weight;
    char tag;
    int data[3];
};

/* Function with carried dependency reduction pattern */
__attribute__((optimize("O2")))
double reduction_with_carry(struct MixedData* arr, int n) {
    double acc = 0.0;
    volatile int v = g_volatile_counter;
    
    /* Loop with carried dependency - scheduler needs to pipeline this */
    for (int i = 1; i < n; i++) {
        /* Complex data-dependent operation with dependency chain */
        double temp = arr[i].value * arr[i-1].weight;
        
        /* Conditional operation based on volatile */
        if (v > 100) {
            temp *= 1.1;
        }
        
        /* Main reduction with dependency across iterations */
        acc = acc + temp;
        
        /* Additional floating point operations */
        arr[i].weight = arr[i].weight * 0.99 + 0.01;
    }
    
    return acc;
}

/* Function with deeply nested loops and volatile conditionals */
__attribute__((optimize("O3")))
int nested_loops_complex(int* data, int size) {
    int result = 0;
    volatile int cond1 = g_volatile_counter % 7;
    volatile float cond2 = g_volatile_float;
    
    /* Triple nested loop with data-dependent conditions */
    for (int i = 0; i < OUTER_LOOP; i++) {
        /* Outer loop with volatile check */
        if (cond1 > i) {
            for (int j = 0; j < MIDDLE_LOOP; j++) {
                /* Middle loop with floating point condition */
                float fcond = cond2 * j;
                if (fcond > 10.0f) {
                    for (int k = 0; k < INNER_LOOP; k++) {
                        /* Innermost loop with mixed operations */
                        int idx = (i * 100 + j * 2 + k * 3) % size;
                        
                        /* Data-dependent branching */
                        if (data[idx] % 2 == 0) {
                            result += data[idx] * k;
                        } else {
                            result -= data[idx] / (k + 1);
                        }
                        
                        /* Floating point operation in integer loop */
                        g_volatile_float += 0.001f * k;
                    }
                }
                /* Update volatile in middle loop */
                cond1 = (cond1 + j) % 13;
            }
        }
        /* Non-contiguous memory access */
        data[(i * 7) % size] = result % 1000;
    }
    
    return result;
}

/* Function with mixed data types and non-unit stride access */
__attribute__((optimize("O2")))
float mixed_type_processing(struct MixedData* arr, int n) {
    float sum = 0.0f;
    volatile int stride_cond = g_volatile_counter % 5;
    
    /* Process with non-contiguous stride */
    for (int i = 0; i < n; i += (stride_cond + 2)) {
        /* Mixed type operations */
        double dval = arr[i].weight;
        float fval = arr[i].value;
        
        /* Conditional store based on complex condition */
        if ((arr[i].id % 3 == 0) && (dval > fval)) {
            arr[i].value = (float)dval * 0.8f;
            sum += arr[i].value;
        } else {
            arr[i].weight = dval * 0.9;
            sum -= fval;
        }
        
        /* Access structure array with offset */
        if (i + 1 < n) {
            arr[i].data[0] = (int)(sum * 100);
            arr[i + 1].data[1] = arr[i].data[0] + 1;
        }
    }
    
    return sum;
}

/* Function with software pipelining candidate */
__attribute__((optimize("O3")))
double pipeline_candidate(double* a, double* b, int n) {
    double dot = 0.0;
    volatile int mod = g_volatile_counter;
    
    /* Perfect candidate for software pipelining */
    for (int i = 0; i < n; i++) {
        /* Long dependency chain */
        double ai = a[i];
        double bi = b[i];
        
        /* Multiple dependent operations */
        ai = ai * 1.01 + 0.5;
        bi = bi * 0.99 - 0.1;
        
        /* Main reduction with dependency */
        dot += ai * bi;
        
        /* Conditional update with volatile */
        if (mod % 4 == 0) {
            a[i] = ai * 0.95;
            b[i] = bi * 1.05;
        }
        
        /* Update volatile */
        mod = (mod * 1103515245 + 12345) & 0x7fffffff;
    }
    
    return dot;
}

/* Initialize data with pseudo-random values */
void initialize_data(struct MixedData* arr, int n, int* int_data) {
    unsigned int seed = 123456789;
    
    for (int i = 0; i < n; i++) {
        /* Simple LCG for reproducibility */
        seed = seed * 1103515245 + 12345;
        
        arr[i].id = i;
        arr[i].value = (float)(seed % 1000) / 100.0f;
        arr[i].weight = (double)(seed % 2000) / 200.0;
        arr[i].tag = 'A' + (seed % 26);
        
        for (int j = 0; j < 3; j++) {
            seed = seed * 1103515245 + 12345;
            arr[i].data[j] = seed % 100;
        }
        
        if (i < SIZE) {
            int_data[i] = seed % 10000;
        }
    }
}

int main() {
    /* Allocate and initialize data */
    struct MixedData* mixed_arr = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    int* int_data = (int*)malloc(SIZE * sizeof(int));
    double* vec_a = (double*)malloc(SIZE * sizeof(double));
    double* vec_b = (double*)malloc(SIZE * sizeof(double));
    
    if (!mixed_arr || !int_data || !vec_a || !vec_b) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    initialize_data(mixed_arr, SIZE, int_data);
    
    /* Initialize double arrays */
    for (int i = 0; i < SIZE; i++) {
        vec_a[i] = (i % 100) * 0.1;
        vec_b[i] = (i % 50) * 0.2;
    }
    
    double total_result = 0.0;
    
    /* Call all computation functions to exercise different scheduler patterns */
    printf("Starting complex computations...\n");
    
    /* 1. Reduction with carried dependency */
    double reduction_result = reduction_with_carry(mixed_arr, SIZE);
    total_result += reduction_result;
    printf("Reduction result: %f\n", reduction_result);
    
    /* 2. Deeply nested loops with volatile conditionals */
    int nested_result = nested_loops_complex(int_data, SIZE);
    total_result += nested_result;
    printf("Nested loops result: %d\n", nested_result);
    
    /* 3. Mixed data types with non-contiguous access */
    float mixed_result = mixed_type_processing(mixed_arr, SIZE);
    total_result += mixed_result;
    printf("Mixed type processing result: %f\n", mixed_result);
    
    /* 4. Software pipelining candidate */
    double pipeline_result = pipeline_candidate(vec_a, vec_b, SIZE);
    total_result += pipeline_result;
    printf("Pipeline candidate result: %f\n", pipeline_result);
    
    /* Additional complex pattern: combination of all */
    volatile int toggle = 0;
    for (int i = 0; i < 100; i++) {
        if (toggle) {
            reduction_result += reduction_with_carry(mixed_arr, SIZE / 10);
        } else {
            mixed_result += mixed_type_processing(mixed_arr, SIZE / 10);
        }
        toggle = !toggle;
        
        /* Update volatile to affect scheduling */
        g_volatile_counter = (g_volatile_counter * 1664525 + 1013904223) & 0x7fffffff;
        g_volatile_float = (g_volatile_counter % 100) / 100.0f;
    }
    
    printf("Final combined result: %f\n", total_result + reduction_result + mixed_result);
    
    /* Cleanup */
    free(mixed_arr);
    free(int_data);
    free(vec_a);
    free(vec_b);
    
    return 0;
}
