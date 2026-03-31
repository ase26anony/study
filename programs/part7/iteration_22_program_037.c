/* Complex loop patterns to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 2000
#define INNER_LOOP 50
#define MIDDLE_LOOP 30
#define OUTER_LOOP 20

/* Volatile variables to prevent optimization of control flow */
volatile int volatile_cond = 0;
volatile float volatile_float = 1.5f;

/* Mixed data type structure */
struct MixedData {
    int id;
    float value;
    double weight;
    char tag;
};

/* Simple PRNG to avoid library dependencies */
static unsigned int seed = 123456789;
static inline unsigned int fast_rand() {
    seed = (1103515245 * seed + 12345) & 0x7fffffff;
    return seed;
}

/* Function 1: Reduction with carried dependency across iterations */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
double reduction_with_carry(double* data, int n) {
    double acc = 0.0;
    double prev = data[0];
    
    /* Complex loop with data-dependent branching */
    for (int i = 1; i < n; i++) {
        /* Volatile condition prevents dead code elimination */
        if (volatile_cond > (fast_rand() % 100)) {
            /* Reduction with carried dependency */
            acc += data[i] * prev;
            prev = data[i];
            
            /* Mixed operations to create diverse RTL */
            acc += (i % 2 == 0) ? data[i-1] * 0.5 : data[i] * 2.0;
        } else {
            /* Alternative path with different operations */
            acc -= data[i] / (prev + 1.0);
            prev = data[i] * 0.8;
        }
        
        /* Additional computation to increase basic block size */
        for (int j = 0; j < 3; j++) {
            acc += (data[i] * j * 0.1);
        }
    }
    return acc;
}

/* Function 2: Nested loops with mixed data types and non-contiguous access */
__attribute__((optimize("O3", "funroll-loops")))
float process_mixed_data(struct MixedData* data, int count) {
    float total = 0.0f;
    double temp_acc = 0.0;
    
    /* Triple nested loop structure */
    for (int outer = 0; outer < OUTER_LOOP; outer++) {
        volatile_cond = fast_rand() % 10;
        
        for (int middle = 0; middle < MIDDLE_LOOP; middle++) {
            /* Data-dependent condition with volatile */
            if (volatile_float > (fast_rand() % 100) * 0.01f) {
                for (int inner = 0; inner < INNER_LOOP; inner++) {
                    /* Non-contiguous access (every 3rd element) */
                    int idx = (outer * MIDDLE_LOOP * INNER_LOOP + 
                              middle * INNER_LOOP + inner) * 3 % count;
                    
                    if (idx < count) {
                        /* Mixed type operations */
                        total += data[idx].value * data[idx].weight;
                        temp_acc += data[idx].id * 0.001;
                        
                        /* Conditional store based on volatile */
                        if (volatile_cond > 5) {
                            data[idx].tag = (char)(total * 0.1);
                        }
                    }
                    
                    /* Additional volatile-dependent computation */
                    total += (volatile_float * inner * 0.01f);
                }
            } else {
                /* Alternative path for scheduler to consider */
                for (int inner = 0; inner < INNER_LOOP / 2; inner++) {
                    int idx = (middle * INNER_LOOP + inner) * 2 % count;
                    if (idx < count) {
                        total -= data[idx].value * 0.5f;
                    }
                }
            }
        }
    }
    
    return total + (float)temp_acc;
}

/* Function 3: Deeply nested loops with complex control flow */
__attribute__((optimize("O2", "fsel-sched-pipelining-outer-loops")))
int complex_nested_loops(int* arr, int n) {
    int result = 0;
    
    /* Four-level nested loop */
    for (int a = 0; a < 10; a++) {
        volatile_cond = fast_rand() % 20;
        
        for (int b = 0; b < 15; b++) {
            for (int c = 0; c < 20; c++) {
                /* Innermost loop with data-dependent condition */
                for (int d = 0; d < 25; d++) {
                    int idx = (a * 1000 + b * 100 + c * 10 + d) % n;
                    
                    /* Complex conditional with volatile */
                    if ((volatile_cond + a + b) > (c + d)) {
                        result += arr[idx] * 2;
                        
                        /* Floating point in integer loop */
                        float temp = arr[idx] * 0.3f;
                        result += (int)(temp * volatile_float);
                    } else {
                        result -= arr[idx] / 3;
                        
                        /* Additional branching */
                        if (arr[idx] % 2 == 0) {
                            result += arr[idx] * arr[(idx + 1) % n];
                        }
                    }
                    
                    /* Memory access pattern with stride */
                    if (d % 3 == 0) {
                        arr[idx] = result % 1000;
                    }
                }
                
                /* Middle loop computation */
                result += b * c * 7;
            }
            
            /* Outer loop computation with function call simulation */
            result += (b * volatile_cond);
        }
    }
    
    return result;
}

/* Main function that orchestrates all computations */
int main() {
    /* Initialize data arrays */
    double double_data[SIZE];
    struct MixedData mixed_data[SIZE * 2];
    int int_data[SIZE * 3];
    
    /* Fill with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        double_data[i] = (fast_rand() % 1000) * 0.01;
    }
    
    for (int i = 0; i < SIZE * 2; i++) {
        mixed_data[i].id = fast_rand() % 10000;
        mixed_data[i].value = (fast_rand() % 1000) * 0.01f;
        mixed_data[i].weight = (fast_rand() % 1000) * 0.001;
        mixed_data[i].tag = (char)(fast_rand() % 256);
    }
    
    for (int i = 0; i < SIZE * 3; i++) {
        int_data[i] = fast_rand() % 10000;
    }
    
    /* Perform computations to trigger scheduler activity */
    double result1 = 0.0;
    float result2 = 0.0f;
    int result3 = 0;
    
    /* Multiple calls to increase scheduling opportunities */
    for (int iter = 0; iter < 5; iter++) {
        volatile_cond = iter;
        volatile_float = 0.5f + iter * 0.1f;
        
        result1 += reduction_with_carry(double_data, SIZE);
        result2 += process_mixed_data(mixed_data, SIZE * 2);
        result3 += complex_nested_loops(int_data, SIZE * 3);
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: %f, %f, %d\n", result1, result2, result3);
    
    /* Additional volatile operations to maintain control flow */
    volatile int final_check = 0;
    for (int i = 0; i < 100; i++) {
        final_check += (result3 > 0) ? 1 : -1;
    }
    
    printf("Final check: %d\n", final_check);
    
    return 0;
}
