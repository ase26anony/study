/* Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 10000
#define INNER_LOOP 50
#define MIDDLE_LOOP 20
#define OUTER_LOOP 10

/* Mixed data type structure with non-contiguous access pattern */
struct MixedData {
    int id;
    float value;
    double weight;
    char tag;
    int counter;
};

/* Volatile variables to prevent optimization of control flow */
volatile int g_volatile_seed = 42;
volatile int g_volatile_mod = 7;

/* Simple PRNG to avoid library dependencies */
static inline int simple_rand(int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

/* Function with complex loop nest and data-dependent control flow */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
double complex_reduction(struct MixedData *data, int count) {
    double acc = 0.0;
    volatile int v_seed = g_volatile_seed;
    
    /* Triple nested loop with data-dependent branches */
    for (int i = 0; i < OUTER_LOOP; i++) {
        int local_seed = v_seed + i;
        
        for (int j = 0; j < MIDDLE_LOOP; j++) {
            /* Data-dependent condition using volatile */
            if ((simple_rand(&local_seed) % g_volatile_mod) > 3) {
                for (int k = 0; k < INNER_LOOP; k++) {
                    /* Reduction with carried dependency */
                    int idx = (i * MIDDLE_LOOP * INNER_LOOP + 
                              j * INNER_LOOP + k) % count;
                    
                    /* Non-contiguous access pattern */
                    if (idx > 0 && idx < count - 1) {
                        /* Critical carried dependency chain */
                        double prev = data[idx - 1].weight;
                        double curr = data[idx].weight;
                        double next = data[idx + 1].weight;
                        
                        /* Complex reduction pattern */
                        acc = acc + prev * curr * 0.5;
                        acc = acc - curr * next * 0.3;
                        
                        /* Conditional store based on volatile */
                        if ((simple_rand(&local_seed) % 5) == 0) {
                            data[idx].value = (float)(acc * 0.01);
                        }
                    }
                    
                    /* Additional floating point operations */
                    float temp = data[idx % count].value;
                    acc += (double)temp * 0.1;
                    
                    /* Data-dependent branch in innermost loop */
                    if ((idx % 13) == (simple_rand(&local_seed) % 13)) {
                        acc *= 1.0001;
                    }
                }
            } else {
                /* Alternative path with different operations */
                for (int k = 0; k < INNER_LOOP / 2; k++) {
                    int idx = (i * MIDDLE_LOOP * INNER_LOOP + 
                              j * INNER_LOOP + k * 2) % count;
                    
                    /* Integer operations mixed with float */
                    data[idx].counter++;
                    acc += data[idx].weight * data[idx].counter;
                    
                    /* Memory access with stride */
                    if (idx + 3 < count) {
                        acc += data[idx + 3].weight * 0.7;
                    }
                }
            }
            
            /* Middle loop computation */
            acc = acc * 0.9999 + (double)j * 0.001;
        }
        
        /* Outer loop update with volatile dependency */
        if ((simple_rand(&local_seed) % 100) < g_volatile_mod) {
            acc = acc * 1.1;
        }
    }
    
    return acc;
}

/* Function with mixed data type processing */
__attribute__((optimize("O3", "funroll-loops")))
float mixed_type_processing(struct MixedData *data, int count) {
    float result = 0.0f;
    volatile int v_mod = g_volatile_mod;
    
    /* Process every 3rd element with non-unit stride */
    for (int i = 0; i < count; i += 3) {
        /* Type conversions and mixed operations */
        double dbl_val = (double)data[i].value;
        int int_val = data[i].id;
        
        /* Complex expression with multiple operation types */
        result += (float)(dbl_val * int_val * 0.01);
        
        /* Conditional based on volatile */
        if ((i % v_mod) == 0) {
            /* Nested loop with small iteration count */
            for (int j = 0; j < 5; j++) {
                result += data[(i + j) % count].value * 0.5f;
                
                /* Additional dependency chain */
                if (j > 0) {
                    result -= data[(i + j - 1) % count].value * 0.2f;
                }
            }
            
            /* Update structure fields */
            data[i].weight = dbl_val * 2.0;
            data[i].tag = (char)((int)data[i].tag + 1);
        }
        
        /* Another data-dependent branch */
        if (data[i].counter > 100) {
            result *= 1.05f;
            data[i].counter = 0;
        }
    }
    
    return result;
}

/* Function with deeply nested loops and volatile conditions */
__attribute__((hot, optimize("O2")))
int nested_loop_pattern(int *array, int size) {
    int total = 0;
    volatile int v_seed = g_volatile_seed;
    
    /* Four-level nested loop */
    for (int a = 0; a < 5; a++) {
        int seed_a = v_seed + a;
        
        for (int b = 0; b < 8; b++) {
            if ((simple_rand(&seed_a) % 3) == 0) {
                for (int c = 0; c < 6; c++) {
                    int seed_b = seed_a + c;
                    
                    for (int d = 0; d < 4; d++) {
                        /* Complex index calculation */
                        int idx = (a * 8 * 6 * 4 + b * 6 * 4 + 
                                  c * 4 + d) % size;
                        
                        /* Array access with carried dependency */
                        if (idx > 0) {
                            total += array[idx] - array[idx - 1];
                        }
                        
                        /* Volatile-dependent operation */
                        if ((simple_rand(&seed_b) % 7) < g_volatile_mod) {
                            array[idx] = total % 1000;
                        }
                        
                        /* Floating point in integer loop */
                        float temp = (float)total * 0.01f;
                        if (temp > 50.0f) {
                            total -= (int)(temp);
                        }
                    }
                }
            } else {
                /* Alternative path */
                for (int c = 0; c < 3; c++) {
                    for (int d = 0; d < 10; d++) {
                        int idx = (b * 30 + c * 10 + d) % size;
                        total ^= array[idx];
                    }
                }
            }
        }
        
        /* Loop-carried dependency update */
        total = (total * 31 + a) & 0xFFFF;
    }
    
    return total;
}

/* Main driver function */
int main() {
    /* Allocate and initialize data */
    struct MixedData *data = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    int *int_array = (int*)malloc(SIZE * sizeof(int));
    
    if (!data || !int_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    int seed = 123456789;
    for (int i = 0; i < SIZE; i++) {
        data[i].id = i;
        data[i].value = (float)(simple_rand(&seed) % 1000) * 0.01f;
        data[i].weight = (double)(simple_rand(&seed) % 500) * 0.02;
        data[i].tag = (char)(i % 128);
        data[i].counter = simple_rand(&seed) % 200;
        
        int_array[i] = simple_rand(&seed) % 10000;
    }
    
    /* Update volatile to affect control flow */
    g_volatile_mod = 5;
    
    /* Call computation functions with complex patterns */
    double result1 = complex_reduction(data, SIZE);
    printf("Result 1: %f\n", result1);
    
    float result2 = mixed_type_processing(data, SIZE);
    printf("Result 2: %f\n", result2);
    
    int result3 = nested_loop_pattern(int_array, SIZE);
    printf("Result 3: %d\n", result3);
    
    /* Combine results to ensure all computations are used */
    double final_result = result1 + result2 + result3;
    printf("Final combined result: %f\n", final_result);
    
    /* Cleanup */
    free(data);
    free(int_array);
    
    return 0;
}
