/* Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 10000
#define INNER_LOOP 50
#define MID_LOOP 20
#define OUTER_LOOP 10

/* Volatile variables to prevent optimization of control flow */
volatile int v_cond1 = 0;
volatile int v_cond2 = 1;
volatile float v_cond3 = 0.5f;

/* Mixed data type structure */
struct MixedData {
    int id;
    float value;
    double weight;
    char tag;
    int counter;
};

/* Function with complex loop nest and carried dependency */
__attribute__((optimize("O2")))
double complex_reduction(double* data, int size) {
    double acc = 0.0;
    volatile int seed = 12345;
    
    /* Outer loop */
    for (int i = 0; i < OUTER_LOOP; i++) {
        /* Middle loop with volatile condition */
        for (int j = 0; j < MID_LOOP; j++) {
            if (v_cond1 || (rand() % 100) > 50) {
                /* Innermost loop with data-dependent control flow */
                for (int k = 0; k < INNER_LOOP; k++) {
                    /* Data-dependent branch with volatile */
                    if ((v_cond2 && (k % 7 == 0)) || (seed % 3 == 0)) {
                        /* Carried dependency across iterations */
                        if (k > 0) {
                            acc += data[(i * MID_LOOP * INNER_LOOP + j * INNER_LOOP + k) % size] *
                                   data[(i * MID_LOOP * INNER_LOOP + j * INNER_LOOP + k - 1) % size];
                        }
                        /* Additional arithmetic to create more RTL patterns */
                        acc += (data[(i * MID_LOOP * INNER_LOOP + j * INNER_LOOP + k) % size] * 1.5) -
                               (data[(i * MID_LOOP * INNER_LOOP + j * INNER_LOOP + k) % size] / 2.0);
                    } else {
                        /* Alternative path with different operations */
                        acc -= data[(i * MID_LOOP * INNER_LOOP + j * INNER_LOOP + k) % size] * 0.75;
                    }
                    
                    /* Volatile modification to prevent dead code elimination */
                    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
                }
            } else {
                /* Different computation path */
                for (int k = 0; k < INNER_LOOP / 2; k++) {
                    acc *= 0.99;
                    acc += data[(j * INNER_LOOP + k * 2) % size] * 0.1;
                }
            }
            
            /* Mix integer and floating point operations */
            int temp_int = (int)acc;
            acc = acc - (double)temp_int + (double)(temp_int % 17);
        }
        
        /* Non-linear memory access pattern */
        for (int offset = 1; offset < 5; offset++) {
            acc += data[(i * offset * 7) % size] * 0.33;
        }
    }
    
    return acc;
}

/* Function processing mixed data types with non-contiguous access */
__attribute__((optimize("O2")))
float process_mixed_types(struct MixedData* data, int count) {
    float total = 0.0f;
    double weight_acc = 0.0;
    volatile int toggle = 0;
    
    /* Process every 3rd element with stride */
    for (int i = 0; i < count; i += 3) {
        /* Complex condition with volatile */
        if ((toggle++ % 5 == 0) || (v_cond3 > 0.3f)) {
            /* Mixed type operations */
            total += data[i].value * (float)data[i].weight;
            weight_acc += data[i].weight * (i % 11);
            
            /* Conditional store based on volatile */
            if (v_cond2 || (data[i].id % 7 == 0)) {
                data[i].counter = (int)(total * 100.0f);
            }
        } else {
            /* Alternative processing path */
            total -= data[i].value * 0.5f;
            weight_acc /= 1.1;
        }
        
        /* Additional nested loop for complexity */
        for (int j = 0; j < 3; j++) {
            if ((i + j) < count) {
                /* Non-unit stride access */
                int idx = (i + j * 2) % count;
                total += (float)data[idx].id * 0.01f;
                
                /* More volatile-dependent branching */
                if (v_cond1 && (j % 2 == 0)) {
                    weight_acc += data[idx].weight * 0.25;
                }
            }
        }
        
        /* Integer operations mixed with float */
        int int_temp = data[i].id * 3;
        total += (float)(int_temp % 19) * 0.05f;
    }
    
    return total + (float)weight_acc;
}

/* Deeply nested loops with volatile conditionals */
__attribute__((optimize("O3")))
int nested_conditional_loops(int* array, int size) {
    int result = 0;
    volatile int v1 = 1, v2 = 0, v3 = 1;
    
    /* Level 1 */
    for (int a = 0; a < size / 100; a++) {
        /* Volatile-dependent branch */
        if (v1 || (a % 13 == 0)) {
            /* Level 2 */
            for (int b = 0; b < 15; b++) {
                /* Complex condition */
                if ((v2 && (b % 3 == 0)) || (!v3 && (b > 7))) {
                    /* Level 3 - innermost with reduction */
                    for (int c = 0; c < 25; c++) {
                        /* Data-dependent operation with carried dependency */
                        if (c > 0) {
                            result += array[(a * 375 + b * 25 + c) % size] -
                                      array[(a * 375 + b * 25 + c - 1) % size];
                        } else {
                            result += array[(a * 375 + b * 25) % size];
                        }
                        
                        /* Additional arithmetic */
                        result *= (c % 5) + 1;
                        result &= 0xFFFF;
                        
                        /* Volatile modification */
                        v1 = (v1 * 1664525 + 1013904223) & 0x7FFFFFFF;
                    }
                    
                    /* Inter-loop computation */
                    result ^= (b * 17);
                } else {
                    /* Different computation path */
                    for (int c = 0; c < 10; c++) {
                        result -= array[(a * 150 + b * 10 + c) % size] / 2;
                    }
                }
                
                /* More branching */
                if (v3 || (b % 4 == 0)) {
                    result += 1000;
                }
            }
        }
        
        /* Post-loop computation */
        result = (result * 31) % 10007;
        v2 = !v2;
    }
    
    return result;
}

/* Main driver function */
int main() {
    /* Initialize data arrays */
    double* double_data = (double*)malloc(SIZE * sizeof(double));
    struct MixedData* mixed_data = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    int* int_data = (int*)malloc(SIZE * sizeof(int));
    
    /* Simple pseudo-random initialization */
    unsigned int seed = 123456789;
    for (int i = 0; i < SIZE; i++) {
        /* Linear congruential generator */
        seed = seed * 1103515245 + 12345;
        double_data[i] = (double)(seed % 10000) / 100.0;
        
        seed = seed * 1103515245 + 12345;
        mixed_data[i].id = seed % 1000;
        mixed_data[i].value = (float)(seed % 1000) / 10.0f;
        mixed_data[i].weight = (double)(seed % 500) / 50.0;
        mixed_data[i].tag = (char)('A' + (seed % 26));
        mixed_data[i].counter = 0;
        
        seed = seed * 1103515245 + 12345;
        int_data[i] = seed % 10000;
    }
    
    /* Call computation functions to trigger scheduler activity */
    double result1 = complex_reduction(double_data, SIZE);
    float result2 = process_mixed_types(mixed_data, SIZE);
    int result3 = nested_conditional_loops(int_data, SIZE);
    
    /* Combine results to ensure code is live */
    double final_result = result1 + result2 + result3;
    
    /* Print results to prevent optimization */
    printf("Results: %.6f, %.6f, %d\n", result1, result2, result3);
    printf("Final combined: %.6f\n", final_result);
    
    /* Additional volatile operations to maintain side effects */
    v_cond1 = (int)final_result % 2;
    v_cond2 = (int)final_result % 3;
    v_cond3 = (float)((int)final_result % 100) / 100.0f;
    
    /* Clean up */
    free(double_data);
    free(mixed_data);
    free(int_data);
    
    return 0;
}
