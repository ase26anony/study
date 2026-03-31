/* Complex loop patterns to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 10000
#define INNER_LOOP_BOUND 50
#define MIDDLE_LOOP_BOUND 20
#define OUTER_LOOP_BOUND 10

/* Volatile variables to prevent optimization of control flow */
volatile int g_volatile_counter = 0;
volatile float g_volatile_float = 0.5f;

/* Mixed data type structure for non-contiguous access */
struct MixedData {
    int id;
    float value;
    double weight;
    char tag;
};

/* Simple LCG PRNG to avoid library dependencies */
static unsigned int lcg_seed = 123456789;
static inline unsigned int lcg_rand() {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Function 1: Reduction with carried dependency across iterations */
__attribute__((optimize("O2")))
double reduction_with_carry(struct MixedData* data, int count) {
    double acc = 0.0;
    volatile int v = g_volatile_counter;
    
    /* Outer loop */
    for (int i = 0; i < OUTER_LOOP_BOUND; i++) {
        /* Middle loop with data-dependent condition */
        for (int j = 0; j < MIDDLE_LOOP_BOUND; j++) {
            /* Innermost loop with carried dependency */
            for (int k = 1; k < INNER_LOOP_BOUND; k++) {
                /* Data-dependent branch using volatile */
                if ((v + k) % 7 == 0) {
                    /* Carried dependency: acc depends on previous iteration */
                    acc = acc + data[k].value * data[k-1].weight;
                    
                    /* Mixed operations */
                    data[k].id = (int)(acc * 100.0);
                    data[k].tag = (char)((int)acc % 26 + 'A');
                } else {
                    /* Alternative path with different operations */
                    acc = acc - data[k].weight * 0.5;
                    data[k].value = (float)(acc * 0.1);
                }
                
                /* Additional floating point operation */
                acc = acc * 1.0001;
                
                /* Volatile read to prevent optimization */
                v = g_volatile_counter + (lcg_rand() % 5);
            }
            
            /* Non-contiguous access pattern */
            if (j % 3 == 0) {
                data[j].weight = data[j].weight * 1.1 + (double)j;
            }
        }
        
        /* Conditional store based on volatile */
        if (v % 2 == 0) {
            data[i].value = (float)acc * 0.01f;
        }
    }
    
    return acc;
}

/* Function 2: Nested loops with complex data-dependent control flow */
__attribute__((optimize("O3")))
float complex_nested_loops(float* array, int size) {
    float result = 0.0f;
    volatile float vf = g_volatile_float;
    
    /* Triple nested loop */
    for (int i = 0; i < size / 100; i++) {
        for (int j = 0; j < 15; j++) {
            /* Data-dependent loop bound */
            int bound = 10 + (lcg_rand() % 10);
            for (int k = 0; k < bound; k++) {
                /* Complex condition with volatile */
                if ((vf > 0.3f) && ((i + j + k) % 11 == 0)) {
                    result += array[i * 50 + k] * 2.0f;
                    
                    /* Memory access with stride */
                    array[(i * 3 + k) % size] = result * 0.5f;
                } else if ((vf < 0.7f) && ((i * j) % 13 == 0)) {
                    result -= array[j * 30 + k] * 1.5f;
                    
                    /* Different stride pattern */
                    array[(j * 7 + k) % size] = result * 0.3f;
                }
                
                /* Mixed integer/float operations */
                int temp = (int)result;
                result = (float)temp * 0.8f + result * 0.2f;
                
                /* Update volatile */
                vf = vf * 0.99f + 0.01f * (float)(k % 5);
            }
            
            /* Additional operation between middle loops */
            result = result + (float)j * 0.01f;
        }
        
        /* Outer loop operation with conditional */
        if (i % 4 == 0) {
            result = result * 0.95f;
        }
    }
    
    return result;
}

/* Function 3: Mixed data type processing with pointer arithmetic */
__attribute__((optimize("O2")))
int process_mixed_types(struct MixedData* data, int count) {
    int total = 0;
    double running_sum = 0.0;
    volatile int vi = g_volatile_counter;
    
    /* Process with stride of 3 for non-contiguous access */
    for (int i = 0; i < count - 3; i += 3) {
        /* Pointer arithmetic with mixed types */
        struct MixedData* ptr1 = &data[i];
        struct MixedData* ptr2 = &data[i + 1];
        struct MixedData* ptr3 = &data[i + 2];
        
        /* Data-dependent branching */
        if ((vi + i) % 5 == 0) {
            running_sum += ptr1->weight * ptr2->value;
            ptr3->id = (int)running_sum;
            total += ptr3->id;
        } else if ((vi + i) % 3 == 0) {
            running_sum -= ptr2->weight / ptr3->value;
            ptr1->value = (float)running_sum;
            total += (int)ptr1->value;
        } else {
            running_sum = running_sum * 1.05 + (double)ptr1->id;
            ptr2->tag = (char)('A' + (i % 26));
            total += ptr2->tag;
        }
        
        /* Nested conditional inside the loop */
        for (int j = 0; j < 2; j++) {
            if ((running_sum > 1000.0) && (j == 0)) {
                ptr1->weight = ptr1->weight * 0.9;
                total -= (int)ptr1->weight;
            }
            
            /* Update volatile */
            vi = g_volatile_counter + j;
        }
        
        /* Additional floating point operation chain */
        double temp = running_sum;
        for (int k = 0; k < 3; k++) {
            temp = temp * 1.001 - (double)k * 0.1;
        }
        running_sum = temp;
    }
    
    return total;
}

/* Main function that orchestrates all computations */
int main() {
    /* Allocate and initialize data */
    struct MixedData* data = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    float* float_array = (float*)malloc(SIZE * sizeof(float));
    
    if (!data || !float_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        data[i].id = lcg_rand() % 1000;
        data[i].value = (float)(lcg_rand() % 100) / 10.0f;
        data[i].weight = (double)(lcg_rand() % 1000) / 100.0;
        data[i].tag = (char)('A' + (i % 26));
        float_array[i] = (float)(lcg_rand() % 100) / 5.0f;
    }
    
    /* Update volatile variables */
    g_volatile_counter = lcg_rand() % 100;
    g_volatile_float = (float)(lcg_rand() % 100) / 100.0f;
    
    /* Call all computation functions to create varied scheduling workload */
    double result1 = reduction_with_carry(data, SIZE);
    float result2 = complex_nested_loops(float_array, SIZE);
    int result3 = process_mixed_types(data, SIZE);
    
    /* Combine results to ensure they're used */
    double final_result = result1 + (double)result2 + (double)result3;
    
    /* Print results to prevent dead code elimination */
    printf("Result 1 (reduction with carry): %.6f\n", result1);
    printf("Result 2 (complex nested loops): %.6f\n", result2);
    printf("Result 3 (mixed types): %d\n", result3);
    printf("Final combined result: %.6f\n", final_result);
    
    /* Cleanup */
    free(data);
    free(float_array);
    
    return 0;
}
