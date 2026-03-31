/* test_sel_sched.c - Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 2048
#define INNER_LOOP 100
#define MIDDLE_LOOP 50
#define OUTER_LOOP 20

/* Volatile variables to prevent optimization of control flow */
volatile int g_volatile_counter = 0;
volatile float g_volatile_float = 1.5f;

/* Mixed data type structure with non-contiguous access pattern */
struct MixedData {
    int id;
    float value;
    double weight;
    char tag;
    int data[3];
};

/* Function with complex loop nest and carried dependency */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
double complex_reduction(struct MixedData* arr, int n) {
    double acc = 0.0;
    volatile int v1 = g_volatile_counter;
    
    /* Outer loop */
    for (int i = 0; i < OUTER_LOOP; i++) {
        /* Middle loop with volatile condition */
        for (int j = 0; j < MIDDLE_LOOP; j++) {
            int cond = (v1 + j) % 7;
            
            /* Innermost loop with data-dependent control flow */
            for (int k = 0; k < INNER_LOOP; k++) {
                /* Volatile read to prevent optimization */
                volatile int v2 = g_volatile_counter + k;
                
                /* Data-dependent branch with mixed operations */
                if ((v2 & 3) == cond) {
                    /* Carried dependency across iterations */
                    if (k > 0) {
                        int idx = (j * 3 + k) % n;
                        int prev_idx = (j * 3 + k - 1) % n;
                        
                        /* Reduction with mixed types and memory access */
                        acc += arr[idx].value * arr[prev_idx].weight;
                        acc += arr[idx].data[k % 3] * 0.01;
                    }
                    
                    /* Floating point operation */
                    float temp = arr[k % n].value * g_volatile_float;
                    acc += (double)temp;
                } else {
                    /* Alternative path with integer operations */
                    int idx = (i * j + k) % n;
                    acc += arr[idx].id * 0.001;
                    
                    /* Conditional store */
                    if ((v2 % 5) == 0) {
                        arr[idx].data[0] = v2;
                    }
                }
                
                /* Non-linear memory access pattern */
                int stride_idx = (k * 3) % n;
                if (stride_idx < n) {
                    acc -= arr[stride_idx].weight * 0.5;
                }
            }
            
            /* Middle loop operation with volatile */
            v1 = (v1 * 1103515245 + 12345) & 0x7fffffff;
            if ((v1 % 11) < 5) {
                acc *= 1.0001;
            }
        }
        
        /* Outer loop operation */
        acc = acc / (i + 1.0);
    }
    
    return acc;
}

/* Function with deeply nested loops and non-contiguous access */
__attribute__((optimize("O3", "funroll-loops")))
float nested_mixed_access(struct MixedData* arr, int n) {
    float result = 0.0f;
    volatile float vf = g_volatile_float;
    
    /* Triple nested loop */
    for (int a = 0; a < 15; a++) {
        for (int b = 0; b < 25; b++) {
            volatile int vb = g_volatile_counter + b;
            
            for (int c = 0; c < 35; c++) {
                /* Complex condition with volatile */
                if ((vb + c * a) % (vf + 2) > 1.0f) {
                    /* Non-contiguous access (every 3rd element) */
                    int idx = (a * 100 + b * 10 + c) * 3 % n;
                    
                    if (idx >= 0 && idx < n) {
                        /* Mixed type operations */
                        result += arr[idx].value;
                        result -= (float)arr[idx].weight;
                        
                        /* Pointer arithmetic with different types */
                        char* ptr = (char*)&arr[idx];
                        for (int d = 0; d < 4; d++) {
                            result += ptr[d] * 0.01f;
                        }
                    }
                } else {
                    /* Alternative computation path */
                    int idx = (c * 7 + b * 3 + a) % n;
                    if (idx < n) {
                        result += arr[idx].data[c % 3] * 0.02f;
                    }
                }
                
                /* Additional volatile dependency */
                vf = vf * 1.01f + 0.5f;
            }
        }
        
        /* Outer loop reduction */
        result = result / (a + 1.0f);
    }
    
    return result;
}

/* Function with reduction pattern and complex dependencies */
__attribute__((hot, optimize("O2")))
double reduction_with_dependencies(int* data, int n) {
    double sum = 0.0;
    double prod = 1.0;
    volatile int seed = g_volatile_counter;
    
    /* Loop with carried dependency */
    for (int i = 1; i < n; i++) {
        /* Volatile condition to prevent optimization */
        volatile int cond = (seed + i) % 13;
        
        /* Critical carried dependency chain */
        double temp = data[i] * 0.01;
        temp += data[i - 1] * 0.005;
        
        /* Data-dependent branching */
        if (cond > 5) {
            sum += temp * prod;
            prod *= 1.00001 + temp * 0.000001;
        } else {
            sum -= temp / (prod + 0.1);
        }
        
        /* Additional computation to increase ILP opportunity */
        for (int j = 0; j < 3; j++) {
            int idx = (i * 7 + j * 3) % n;
            if (idx > 0) {
                sum += data[idx] * data[idx - 1] * 0.0001;
            }
        }
        
        /* Update volatile */
        seed = (seed * 1664525 + 1013904223) & 0x7fffffff;
    }
    
    return sum + prod;
}

/* Initialize array with pseudo-random data */
void init_array(struct MixedData* arr, int n) {
    unsigned int seed = 123456789;
    
    for (int i = 0; i < n; i++) {
        arr[i].id = i;
        arr[i].value = (float)(seed % 1000) / 100.0f;
        seed = seed * 1103515245 + 12345;
        
        arr[i].weight = (double)(seed % 2000) / 200.0;
        seed = seed * 1103515245 + 12345;
        
        arr[i].tag = 'A' + (seed % 26);
        
        for (int j = 0; j < 3; j++) {
            arr[i].data[j] = seed % 100;
            seed = seed * 1103515245 + 12345;
        }
    }
}

/* Initialize integer array */
void init_int_array(int* arr, int n) {
    unsigned int seed = 987654321;
    
    for (int i = 0; i < n; i++) {
        arr[i] = seed % 10000;
        seed = seed * 1664525 + 1013904223;
    }
}

int main() {
    /* Allocate and initialize data */
    struct MixedData* mixed_arr = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    int* int_arr = (int*)malloc(SIZE * sizeof(int));
    
    if (!mixed_arr || !int_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_array(mixed_arr, SIZE);
    init_int_array(int_arr, SIZE);
    
    /* Update volatile variables */
    g_volatile_counter = 42;
    g_volatile_float = 3.14159f;
    
    /* Call computation functions with complex patterns */
    double result1 = complex_reduction(mixed_arr, SIZE);
    float result2 = nested_mixed_access(mixed_arr, SIZE);
    double result3 = reduction_with_dependencies(int_arr, SIZE);
    
    /* Combine results to ensure all computations are used */
    double final_result = result1 + result2 + result3;
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %.10f\n", final_result);
    printf("Result components: %.6f, %.6f, %.6f\n", 
           result1, result2, result3);
    
    /* Free allocated memory */
    free(mixed_arr);
    free(int_arr);
    
    return 0;
}
