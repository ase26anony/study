/* Complex program to trigger selective scheduler debug dumping */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 2048
#define INNER_LOOP 128
#define MIDDLE_LOOP 64
#define OUTER_LOOP 32

/* Volatile variables to prevent optimization */
volatile int vol_cond1 = 0;
volatile int vol_cond2 = 1;
volatile float vol_float = 3.14f;
volatile double vol_double = 2.71828;

/* Mixed data type structure */
struct MixedData {
    int id;
    float value;
    double weight;
    char tag;
    int counter;
};

/* Function with complex loop nest and carried dependency */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
double complex_reduction(struct MixedData* data, int n) {
    double acc = 0.0;
    volatile int v = vol_cond1;
    
    /* Outer loop */
    for (int i = 0; i < OUTER_LOOP; i++) {
        /* Middle loop with volatile condition */
        for (int j = (vol_cond2 ? 1 : 0); j < MIDDLE_LOOP; j++) {
            /* Innermost loop with data-dependent control flow */
            for (int k = 0; k < INNER_LOOP; k++) {
                /* Volatile read to prevent optimization */
                v = vol_cond1 + rand() % 10;
                
                /* Complex carried dependency across iterations */
                if (k > 0) {
                    /* Reduction with mixed operations */
                    acc += data[(i * MIDDLE_LOOP * INNER_LOOP + 
                                j * INNER_LOOP + k) % n].value *
                           data[(i * MIDDLE_LOOP * INNER_LOOP + 
                                j * INNER_LOOP + (k-1)) % n].weight;
                }
                
                /* Additional floating point operations */
                acc += (data[(i * MIDDLE_LOOP * INNER_LOOP + 
                            j * INNER_LOOP + k) % n].id % 7) * 0.5;
                
                /* Conditional store based on volatile */
                if (v > 5) {
                    data[(i * MIDDLE_LOOP * INNER_LOOP + 
                         j * INNER_LOOP + k) % n].counter++;
                }
            }
            
            /* Non-unit stride access */
            if (j % 3 == 0) {
                acc -= data[(j * 7) % n].weight * 0.3;
            }
        }
        
        /* Volatile write */
        vol_float = acc * 0.01f;
    }
    
    return acc;
}

/* Function with deeply nested loops and mixed operations */
__attribute__((optimize("O3", "funroll-loops")))
float nested_mixed_operations(int* arr_int, float* arr_float, 
                              double* arr_double, int n) {
    float result = 0.0f;
    volatile int cond = vol_cond2;
    
    /* Triple nested loop with volatile conditions */
    for (int a = 0; a < 16; a++) {
        for (int b = cond ? 2 : 1; b < 24; b++) {
            for (int c = 0; c < 32; c++) {
                /* Read volatile to create memory barrier */
                cond = vol_cond1;
                
                /* Mixed type operations */
                int idx = (a * 24 * 32 + b * 32 + c) % n;
                float temp = arr_float[idx];
                
                /* Data-dependent branching */
                if (arr_int[idx] % 11 > 5) {
                    temp *= arr_double[idx % (n/2)];
                    result += temp;
                } else {
                    temp /= (arr_double[(idx + 7) % n] + 1.0);
                    result -= temp;
                }
                
                /* Complex addressing with stride */
                if (c % 4 == 0) {
                    arr_int[(idx * 3) % n] = (int)(temp * 100);
                }
                
                /* Additional volatile dependency */
                if (cond) {
                    vol_double = arr_double[idx] * 0.5;
                }
            }
            
            /* Reduction across middle loop */
            result += arr_float[(b * 13) % n] * 0.7f;
        }
    }
    
    return result;
}

/* Function with pointer chasing and complex control flow */
__attribute__((hot, optimize("O2")))
int pointer_chasing_reduction(struct MixedData* data, int n) {
    int sum = 0;
    struct MixedData* current = &data[0];
    volatile int skip = vol_cond1;
    
    /* Loop with pointer chasing pattern */
    for (int i = 0; i < n * 2; i++) {
        /* Volatile read in condition */
        if (skip || (rand() % 100) > 50) {
            current = &data[(current->id + i) % n];
            skip = vol_cond2;
        }
        
        /* Mixed operations with carried dependency */
        sum += current->id;
        sum -= (int)(current->value * current->weight);
        
        /* Conditional update with complex expression */
        if (current->tag == 'A' || current->value > vol_float) {
            current->weight = (sum % 100) * 0.01;
            vol_float = current->value;
        }
        
        /* Non-linear pointer advancement */
        current = &data[(current->id * 17 + i) % n];
        
        /* Additional reduction */
        sum += current->counter * 3;
    }
    
    return sum;
}

/* Main driver function */
int main() {
    /* Initialize data arrays */
    struct MixedData* mixed_array = 
        (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    float* float_array = (float*)malloc(SIZE * sizeof(float));
    double* double_array = (double*)malloc(SIZE * sizeof(double));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        mixed_array[i].id = i;
        mixed_array[i].value = (i * 1.7f) / (i + 1);
        mixed_array[i].weight = (i * 2.3) / (i + 2);
        mixed_array[i].tag = (i % 2) ? 'A' : 'B';
        mixed_array[i].counter = i % 100;
        
        int_array[i] = (i * 13) % 97;
        float_array[i] = (i * 1.1f) / (i % 7 + 1);
        double_array[i] = (i * 2.7) / (i % 11 + 1);
    }
    
    /* Call computation functions to trigger scheduler */
    double result1 = complex_reduction(mixed_array, SIZE);
    float result2 = nested_mixed_operations(int_array, float_array, 
                                           double_array, SIZE);
    int result3 = pointer_chasing_reduction(mixed_array, SIZE);
    
    /* Combine results to prevent dead code elimination */
    double final_result = result1 + result2 + result3;
    
    /* Print result to ensure code is live */
    printf("Final result: %f\n", final_result);
    
    /* Cleanup */
    free(mixed_array);
    free(int_array);
    free(float_array);
    free(double_array);
    
    return 0;
}
