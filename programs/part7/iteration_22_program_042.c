/* Complex program to trigger selective scheduler debug dumping */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 2048
#define INNER_LOOP 100
#define MIDDLE_LOOP 50
#define OUTER_LOOP 20

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile float g_volatile_float = 1.5f;

/* Mixed data type structure */
struct MixedData {
    int id;
    float value;
    double precision;
    char tag;
    int data[3];
};

/* Function with complex loop nest and carried dependency */
__attribute__((optimize("O2,unroll-loops")))
double complex_reduction(struct MixedData* arr, int n) {
    double acc = 0.0;
    volatile int cond = g_volatile_counter;
    
    /* Outer loop */
    for (int i = 0; i < OUTER_LOOP; i++) {
        /* Middle loop with volatile condition */
        for (int j = 0; j < MIDDLE_LOOP; j++) {
            if (cond++ % 3 == 0) {
                /* Innermost loop with carried dependency */
                for (int k = 1; k < INNER_LOOP; k++) {
                    /* Complex reduction with mixed operations */
                    double temp = arr[(i * MIDDLE_LOOP + j) * INNER_LOOP + k].precision;
                    double prev = arr[(i * MIDDLE_LOOP + j) * INNER_LOOP + (k-1)].precision;
                    
                    /* Data-dependent floating point operations */
                    if (temp > prev) {
                        acc += temp * prev * (k % 7);
                    } else {
                        acc -= temp / (prev + 1.0) * (k % 5);
                    }
                    
                    /* Integer operations with side effects */
                    arr[(i * MIDDLE_LOOP + j) * INNER_LOOP + k].data[k % 3] += 
                        (int)(acc * 100) % 256;
                }
            } else if (cond++ % 5 == 0) {
                /* Alternative path with different operations */
                for (int k = 2; k < INNER_LOOP; k += 2) {
                    float f1 = arr[(i * MIDDLE_LOOP + j) * INNER_LOOP + k].value;
                    float f2 = arr[(i * MIDDLE_LOOP + j) * INNER_LOOP + k-1].value;
                    acc += (double)(f1 * f2) / (k + 1);
                }
            }
        }
        
        /* Additional computation between loop levels */
        if (i % 4 == 0) {
            for (int j = 0; j < 10; j++) {
                acc *= 1.0001;
            }
        }
    }
    
    return acc;
}

/* Function with non-contiguous memory access pattern */
__attribute__((optimize("O3")))
float non_contiguous_access(struct MixedData* arr, int n) {
    float sum = 0.0f;
    volatile int skip = g_volatile_counter % 7 + 2;
    
    /* Process every skip-th element with stride */
    for (int i = 0; i < n; i += skip) {
        /* Nested loops with mixed operations */
        for (int j = 0; j < 3; j++) {
            if ((i + j) % 2 == 0) {
                /* Complex floating point chain */
                float temp = arr[i].value;
                for (int k = 0; k < 4; k++) {
                    temp = temp * 0.99f + arr[i].data[j] * 0.01f;
                }
                sum += temp;
                
                /* Conditional store */
                if (sum > 1000.0f) {
                    arr[i].data[j] = (int)(sum) % 100;
                }
            }
        }
        
        /* Additional volatile-dependent computation */
        if (g_volatile_float > 1.0f) {
            sum *= 0.999f;
        }
    }
    
    return sum;
}

/* Function with deeply nested loops and volatile conditions */
__attribute__((hot, optimize("O2")))
int deep_nested_loops(int* data, int n) {
    int result = 0;
    volatile int v1 = g_volatile_counter;
    volatile int v2 = g_volatile_counter + 1;
    
    /* Level 1 */
    for (int a = 0; a < 10; a++) {
        if (v1++ % 2 == 0) {
            /* Level 2 */
            for (int b = 0; b < 15; b++) {
                /* Level 3 - innermost with complex condition */
                for (int c = 0; c < 20; c++) {
                    int idx = (a * 15 + b) * 20 + c;
                    if (idx < n) {
                        /* Mixed integer operations */
                        if (v2++ % 3 == 0) {
                            data[idx] = data[idx] * 3 + 7;
                            result += data[idx] / 2;
                        } else {
                            data[idx] = data[idx] / 2 - 1;
                            result -= data[idx] * 2;
                        }
                        
                        /* Additional computation with branching */
                        for (int d = 0; d < 2; d++) {
                            if (result % 5 == d) {
                                result ^= (data[idx] << d);
                            }
                        }
                    }
                }
                
                /* Middle loop computation */
                if (b % 4 == 0) {
                    result |= 0xFF;
                }
            }
        } else {
            /* Alternative path */
            for (int b = 5; b < 12; b++) {
                result += data[a * b % n] * b;
            }
        }
    }
    
    return result;
}

/* Helper function to initialize data */
void initialize_data(struct MixedData* arr, int n) {
    unsigned int seed = 42;
    for (int i = 0; i < n; i++) {
        arr[i].id = i;
        arr[i].value = (float)(seed % 1000) / 100.0f;
        arr[i].precision = (double)(seed % 10000) / 1000.0;
        arr[i].tag = 'A' + (i % 26);
        
        for (int j = 0; j < 3; j++) {
            arr[i].data[j] = (seed = seed * 1103515245 + 12345) % 100;
        }
    }
}

int main() {
    /* Allocate and initialize data */
    int total_elements = OUTER_LOOP * MIDDLE_LOOP * INNER_LOOP;
    struct MixedData* data_array = (struct MixedData*)malloc(
        total_elements * sizeof(struct MixedData));
    
    int* int_data = (int*)malloc(SIZE * sizeof(int));
    
    if (!data_array || !int_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    initialize_data(data_array, total_elements);
    for (int i = 0; i < SIZE; i++) {
        int_data[i] = i * 3 + 7;
    }
    
    /* Call computation functions to trigger scheduler activity */
    double result1 = complex_reduction(data_array, total_elements);
    printf("Result 1: %f\n", result1);
    
    float result2 = non_contiguous_access(data_array, total_elements);
    printf("Result 2: %f\n", result2);
    
    int result3 = deep_nested_loops(int_data, SIZE);
    printf("Result 3: %d\n", result3);
    
    /* Final computation combining all results */
    double final_result = result1 + result2 + result3;
    printf("Final result: %f\n", final_result);
    
    /* Cleanup */
    free(data_array);
    free(int_data);
    
    return 0;
}
