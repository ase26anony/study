/* Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 10000
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
__attribute__((optimize("O2")))
double complex_reduction(struct MixedData* arr, int n) {
    double acc = 0.0;
    volatile int v = g_volatile_counter;
    
    /* Outer loop */
    for (int i = 0; i < OUTER_LOOP; i++) {
        /* Middle loop with volatile condition */
        for (int j = 0; j < MIDDLE_LOOP; j++) {
            if (v % (j + 2) == 0) {
                /* Innermost loop with carried dependency */
                for (int k = 1; k < INNER_LOOP; k++) {
                    /* Non-contiguous access pattern (every 3rd element) */
                    int idx = (i * MIDDLE_LOOP * INNER_LOOP + j * INNER_LOOP + k) % n;
                    int prev_idx = (idx - 1 + n) % n;
                    
                    /* Reduction with carried dependency across iterations */
                    acc = acc + arr[idx].value * arr[prev_idx].weight;
                    
                    /* Mixed data type operations */
                    arr[idx].data[k % 3] = (int)(acc * g_volatile_float);
                    
                    /* Data-dependent conditional store */
                    if (acc > 1000.0) {
                        arr[idx].weight = acc * 0.5;
                    }
                }
            } else {
                /* Alternative path with different operations */
                for (int k = 0; k < INNER_LOOP / 2; k++) {
                    int idx = (i * MIDDLE_LOOP * INNER_LOOP + j * INNER_LOOP + k * 2) % n;
                    acc = acc - arr[idx].value / (k + 1);
                }
            }
            
            /* Update volatile variable to affect control flow */
            v = (v * 1103515245 + 12345) & 0x7fffffff;
        }
    }
    
    return acc;
}

/* Function with deeply nested loops and complex conditions */
__attribute__((optimize("O3")))
float nested_conditional_processing(struct MixedData* arr, int n) {
    float result = 0.0f;
    volatile float vf = g_volatile_float;
    
    /* Triple nested loop with data-dependent conditions */
    for (int a = 0; a < 10; a++) {
        for (int b = 0; b < 15; b++) {
            /* Volatile condition prevents loop unrolling */
            if ((vf * a) > (b * 2.0f)) {
                for (int c = 0; c < 25; c++) {
                    int idx = (a * 375 + b * 25 + c) % n;
                    
                    /* Complex conditional chain */
                    if (arr[idx].id % 3 == 0) {
                        result += arr[idx].value * 2.0f;
                        arr[idx].tag = 'A';
                    } else if (arr[idx].id % 3 == 1) {
                        result -= arr[idx].value * 1.5f;
                        arr[idx].tag = 'B';
                        
                        /* Nested condition inside else-if */
                        if (result < 0) {
                            arr[idx].weight = -result;
                        }
                    } else {
                        result *= 0.9f;
                        arr[idx].tag = 'C';
                    }
                    
                    /* Memory access with stride */
                    if (c % 4 == 0) {
                        arr[idx].data[0] = (int)(result * 100);
                    }
                }
            }
            
            /* Modify volatile float */
            vf = vf * 0.95f + 0.1f;
        }
    }
    
    return result;
}

/* Function with pointer arithmetic and mixed operations */
__attribute__((optimize("O2")))
double pointer_based_computation(struct MixedData* arr, int n) {
    double sum = 0.0;
    struct MixedData* ptr = arr;
    struct MixedData* end = arr + n;
    
    /* Process array with pointer arithmetic */
    while (ptr < end) {
        /* Skip every 5th element */
        if ((ptr - arr) % 5 != 0) {
            /* Mixed type computation */
            double temp = ptr->value * ptr->weight;
            
            /* Conditional update based on volatile */
            if (g_volatile_counter++ % 7 == 0) {
                temp *= 1.1;
                ptr->tag = 'X';
            }
            
            sum += temp;
            
            /* Update structure fields */
            ptr->id = (int)(sum * 1000);
            ptr->data[0] = ptr->id % 256;
            ptr->data[1] = (ptr->id / 256) % 256;
            ptr->data[2] = ptr->id / 65536;
        }
        
        /* Non-unit stride access */
        ptr += 3;
    }
    
    return sum;
}

/* Initialize array with pseudo-random data */
void initialize_array(struct MixedData* arr, int n) {
    unsigned int seed = 123456789;
    
    for (int i = 0; i < n; i++) {
        arr[i].id = i;
        
        /* Simple LCG for pseudo-random values */
        seed = seed * 1103515245 + 12345;
        arr[i].value = (float)((seed >> 16) & 0x7FFF) / 32767.0f * 100.0f;
        
        seed = seed * 1103515245 + 12345;
        arr[i].weight = (double)((seed >> 16) & 0x7FFF) / 32767.0 * 200.0;
        
        arr[i].tag = '0' + (i % 10);
        
        for (int j = 0; j < 3; j++) {
            seed = seed * 1103515245 + 12345;
            arr[i].data[j] = (seed >> 16) & 0xFF;
        }
    }
}

int main() {
    /* Allocate and initialize data */
    struct MixedData* data = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    initialize_array(data, SIZE);
    
    /* Call computation functions to create scheduling opportunities */
    double result1 = complex_reduction(data, SIZE);
    printf("Result 1: %f\n", result1);
    
    float result2 = nested_conditional_processing(data, SIZE);
    printf("Result 2: %f\n", result2);
    
    double result3 = pointer_based_computation(data, SIZE);
    printf("Result 3: %f\n", result3);
    
    /* Combine results to ensure all computations are used */
    double final_result = result1 + result2 + result3;
    printf("Final combined result: %f\n", final_result);
    
    /* Additional volatile operations to affect scheduling */
    for (int i = 0; i < 100; i++) {
        g_volatile_counter = (g_volatile_counter * 1664525 + 1013904223) & 0x7fffffff;
        g_volatile_float = g_volatile_float * 0.99f + 0.01f;
    }
    
    free(data);
    return 0;
}
