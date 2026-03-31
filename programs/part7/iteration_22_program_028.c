/* Complex loop patterns to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 10000
#define INNER_ITERS 50
#define MIDDLE_ITERS 100
#define OUTER_ITERS 20

/* Volatile variables to prevent optimization of control flow */
volatile int g_volatile_counter = 0;
volatile float g_volatile_float = 0.5f;

/* Mixed data type structure with non-contiguous access pattern */
struct MixedData {
    int id;
    float value;
    double weight;
    char tag;
};

/* Function with carried dependency reduction - prime candidate for pipelining */
__attribute__((optimize("O2", "fsel-sched-pipelining", "fsel-sched-pipelining-outer-loops")))
double reduction_with_carried_dependency(double* data, int n) {
    double acc = 0.0;
    volatile int v = g_volatile_counter;
    
    /* Reduction with carried dependency across iterations */
    for (int i = 1; i < n; i++) {
        if (v & 1) {
            acc = acc + data[i] * data[i-1] * (1.0 + g_volatile_float);
        } else {
            acc = acc - data[i] * data[i-1] * (0.5 - g_volatile_float);
        }
        v = v ^ (i & 0xFF); /* Data-dependent modification */
    }
    
    return acc;
}

/* Function with deeply nested loops and volatile conditionals */
__attribute__((optimize("O3", "funroll-loops")))
void nested_loops_with_volatile(struct MixedData* array, int size) {
    volatile int outer_cond = g_volatile_counter % 7;
    volatile float middle_cond = g_volatile_float;
    int temp_sum = 0;
    float temp_float = 0.0f;
    
    /* 3-level nested loop as requested */
    for (int i = 0; i < OUTER_ITERS; i++) {
        if (outer_cond > (i % 5)) { /* Data-dependent branch */
            for (int j = 0; j < MIDDLE_ITERS; j++) {
                volatile int inner_volatile = rand() % 100; /* Prevent optimization */
                
                for (int k = 0; k < INNER_ITERS; k++) {
                    /* Non-contiguous access pattern (every 3rd element) */
                    int idx = (i * MIDDLE_ITERS * INNER_ITERS + 
                              j * INNER_ITERS + k) % size;
                    
                    if (inner_volatile > 50) { /* Another data-dependent branch */
                        array[idx].value = array[idx].value * 1.1f + 
                                          g_volatile_float;
                        temp_float += array[idx].value;
                    } else {
                        array[idx].weight = array[idx].weight * 0.9 - 
                                           (k % 10) * 0.01;
                        temp_sum += (int)array[idx].weight;
                    }
                    
                    /* Mixed operations to create diverse RTL */
                    array[idx].id = (array[idx].id + temp_sum) & 0xFFFF;
                    inner_volatile = (inner_volatile * 1103515245 + 12345) & 0x7FFFFFFF;
                }
                
                /* Conditional store based on volatile */
                if (middle_cond > 0.3f) {
                    temp_sum += j * 2;
                }
            }
        }
        outer_cond = (outer_cond * 3 + 1) % 11;
    }
    
    /* Use results to prevent dead code elimination */
    g_volatile_counter += temp_sum;
    g_volatile_float += temp_float * 0.0001f;
}

/* Function with mixed data type processing and pointer arithmetic */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
float process_mixed_types(struct MixedData* data, int n) {
    float result = 0.0f;
    volatile int selector = g_volatile_counter;
    
    /* Process with stride of 3 for non-contiguous access */
    for (int i = 0; i < n; i += 3) {
        struct MixedData* elem = &data[i % n];
        
        /* Complex conditional with mixed operations */
        if ((selector ^ elem->id) & 1) {
            result += elem->value * (float)elem->weight + 
                     (elem->tag * 0.01f);
            
            /* Memory access pattern that might require scheduling */
            if (i > 0) {
                struct MixedData* prev = &data[(i-3) % n];
                result -= prev->value * 0.5f;
                prev->weight = elem->weight * 0.8 + prev->weight * 0.2;
            }
        } else {
            result -= elem->value * (float)elem->weight * 0.7f;
            elem->id = (elem->id + (int)result) & 0xFFF;
        }
        
        selector = (selector * 1664525 + 1013904223) & 0x7FFFFFFF;
    }
    
    return result;
}

/* Initialize with pseudo-random data (avoiding external libs for portability) */
void initialize_data(double* double_data, int double_size,
                    struct MixedData* mixed_data, int mixed_size) {
    unsigned int seed = 123456789;
    
    for (int i = 0; i < double_size; i++) {
        seed = seed * 1103515245 + 12345;
        double_data[i] = (double)(seed % 1000) / 100.0;
    }
    
    for (int i = 0; i < mixed_size; i++) {
        seed = seed * 1103515245 + 12345;
        mixed_data[i].id = seed % 10000;
        
        seed = seed * 1103515245 + 12345;
        mixed_data[i].value = (float)(seed % 1000) / 50.0f;
        
        seed = seed * 1103515245 + 12345;
        mixed_data[i].weight = (double)(seed % 2000) / 75.0;
        
        mixed_data[i].tag = (char)(i % 26 + 'A');
    }
}

int main() {
    /* Allocate and initialize data */
    double* double_data = (double*)malloc(SIZE * sizeof(double));
    struct MixedData* mixed_data = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    
    if (!double_data || !mixed_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    initialize_data(double_data, SIZE, mixed_data, SIZE);
    
    printf("Starting complex computations...\n");
    
    /* Call all computation functions to increase scheduler activity */
    double result1 = reduction_with_carried_dependency(double_data, SIZE);
    
    nested_loops_with_volatile(mixed_data, SIZE);
    
    float result2 = process_mixed_types(mixed_data, SIZE);
    
    /* Additional complex loop to increase chances of debug dump */
    volatile int extra_counter = 0;
    double extra_sum = 0.0;
    
    for (int i = 0; i < SIZE; i++) {
        if (extra_counter & (1 << (i % 8))) {
            extra_sum += double_data[i] * mixed_data[i].value;
        } else {
            extra_sum -= double_data[i] * mixed_data[i % 100].weight;
        }
        
        /* Complex update with multiple operations */
        extra_counter = (extra_counter * 3 + i) % 997;
        mixed_data[i % SIZE].value += (float)(extra_sum * 0.0001);
    }
    
    /* Combine and print results to ensure code is live */
    double final_result = result1 + result2 + extra_sum;
    printf("Final combined result: %f\n", final_result);
    printf("Volatile counter: %d, volatile float: %f\n", 
           g_volatile_counter, g_volatile_float);
    
    /* Cleanup */
    free(double_data);
    free(mixed_data);
    
    return 0;
}
