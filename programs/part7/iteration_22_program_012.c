/* Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 10000
#define INNER_LOOP 50
#define MIDDLE_LOOP 20
#define OUTER_LOOP 10

/* Volatile variables to prevent optimization */
volatile int v_cond1 = 0;
volatile int v_cond2 = 1;
volatile float v_float = 3.14f;
volatile double v_double = 2.71828;

/* Mixed data type structure */
struct MixedData {
    int id;
    float value;
    double weight;
    char tag;
    int counter;
};

/* Function with complex loop nest and carried dependency */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
double complex_reduction(struct MixedData* data, int n) {
    double acc = 0.0;
    double prev = data[0].weight;
    
    /* Outer loop */
    for (int i = 0; i < OUTER_LOOP; i++) {
        /* Middle loop with volatile condition */
        for (int j = 0; j < MIDDLE_LOOP; j++) {
            if (v_cond1 || (rand() % 100) > 50) {
                /* Inner loop with carried dependency */
                for (int k = 0; k < INNER_LOOP; k++) {
                    int idx = (i * MIDDLE_LOOP * INNER_LOOP + j * INNER_LOOP + k) % n;
                    
                    /* Complex reduction with mixed operations */
                    double current = data[idx].weight;
                    acc = acc + current * prev;
                    
                    /* Conditional operations based on volatile */
                    if (v_cond2 && data[idx].value > v_float) {
                        acc += data[idx].value * 0.5;
                    }
                    
                    /* Non-contiguous access pattern */
                    if (idx % 3 == 0) {
                        prev = current * 1.1;
                    } else if (idx % 7 == 0) {
                        prev = current * 0.9;
                    } else {
                        prev = current;
                    }
                    
                    /* Integer operations mixed in */
                    data[idx].counter += (int)(acc * 0.01);
                }
            }
        }
        
        /* Additional volatile-dependent computation */
        if (v_cond1) {
            for (int j = 0; j < n / 100; j++) {
                int idx = (i * 17 + j * 13) % n;
                acc += data[idx].weight * v_double;
            }
        }
    }
    
    return acc;
}

/* Function with deeply nested loops and mixed data types */
__attribute__((optimize("O3", "funroll-loops")))
float nested_mixed_processing(struct MixedData* data, int n) {
    float result = 0.0f;
    volatile int v_local = rand() % 100;
    
    /* Triple nested loop with data-dependent conditions */
    for (int a = 0; a < 5; a++) {
        for (int b = 0; b < 8; b++) {
            /* Volatile condition prevents loop optimization */
            if (v_local++ % 3 == 0) {
                for (int c = 0; c < 12; c++) {
                    int idx = (a * 100 + b * 10 + c) % n;
                    
                    /* Mixed type operations */
                    float temp = data[idx].value;
                    double dtemp = data[idx].weight;
                    
                    /* Complex conditional chain */
                    if (temp > 0.5f) {
                        result += temp * 2.0f;
                        if (dtemp > 1.0) {
                            result -= (float)(dtemp * 0.3);
                        }
                    } else if (v_cond2) {
                        result += (float)(dtemp * 0.1);
                    }
                    
                    /* Non-linear array access */
                    int stride_idx = (idx * 7 + 3) % n;
                    data[stride_idx].value = result * 0.5f;
                    
                    /* Integer operation with side effect */
                    data[idx].id += (int)(result * 100);
                }
            }
        }
    }
    
    return result;
}

/* Function with pointer chasing and complex control flow */
__attribute__((hot, optimize("O2")))
int pointer_chasing_reduction(struct MixedData* data, int n) {
    int sum = 0;
    struct MixedData* current = &data[0];
    
    /* Loop with pointer chasing pattern */
    for (int i = 0; i < n * 2; i++) {
        /* Volatile check prevents optimization */
        if (v_cond1 || (i % 17) == 0) {
            /* Mixed operations */
            sum += current->id;
            sum -= (int)(current->value * 10);
            
            /* Conditional store */
            if (current->weight > v_double) {
                current->counter = sum % 1000;
            }
            
            /* Non-contiguous pointer advance */
            int advance = (i % 4 == 0) ? 3 : 1;
            current = &data[(current->id + advance) % n];
            
            /* Additional volatile-dependent computation */
            if (v_cond2) {
                sum += rand() % 100;
            }
        } else {
            /* Alternative path with different operations */
            sum += current->counter * 2;
            current = &data[(current->id * 13 + 7) % n];
        }
        
        /* Inner small loop */
        for (int j = 0; j < 3; j++) {
            if ((i + j) % 5 == 0) {
                sum += j * current->id;
            }
        }
    }
    
    return sum;
}

/* Main function that drives all computations */
int main() {
    /* Allocate and initialize data */
    struct MixedData* data = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    if (!data) return 1;
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE; i++) {
        data[i].id = i;
        data[i].value = (float)((i * 17 + 23) % 100) / 10.0f;
        data[i].weight = (double)((i * 13 + 7) % 200) / 20.0;
        data[i].tag = (char)('A' + (i % 26));
        data[i].counter = 0;
    }
    
    /* Seed random for volatile conditions */
    srand(42);
    
    /* Perform multiple complex computations */
    double result1 = 0.0;
    float result2 = 0.0f;
    int result3 = 0;
    
    /* Call computation functions multiple times */
    for (int iter = 0; iter < 3; iter++) {
        v_cond1 = iter % 2;
        v_cond2 = (iter % 3) != 0;
        v_float = 2.5f + iter * 0.1f;
        v_double = 1.5 + iter * 0.2;
        
        result1 += complex_reduction(data, SIZE);
        result2 += nested_mixed_processing(data, SIZE);
        result3 += pointer_chasing_reduction(data, SIZE);
        
        /* Modify data between iterations */
        for (int i = 0; i < SIZE; i += 7) {
            data[i].value *= 1.1f;
            data[i].weight *= 0.95;
        }
    }
    
    /* Combine and print results to prevent dead code elimination */
    printf("Results: %f, %f, %d\n", 
           result1, result2, result3);
    
    /* Additional print to ensure all data is used */
    printf("Sample data[100]: id=%d, value=%f, weight=%f\n",
           data[100].id, data[100].value, data[100].weight);
    
    free(data);
    return 0;
}
