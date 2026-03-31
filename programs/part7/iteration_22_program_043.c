/* Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 10000
#define INNER_LOOP 50
#define MIDDLE_LOOP 20
#define OUTER_LOOP 10

/* Mixed data type structure for complex access patterns */
struct MixedData {
    int id;
    float value;
    double weight;
    char tag;
    volatile int flag; /* volatile to prevent optimization */
};

/* Global volatile variables to create data-dependent control flow */
volatile int global_counter = 0;
volatile int global_seed = 12345;

/* Simple LCG PRNG to avoid external dependencies */
static unsigned int prng_state = 123456789;
static inline unsigned int simple_rand(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return (unsigned int)(prng_state >> 16) & 32767;
}

/* Function with complex loop nest and carried dependency */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
double complex_reduction(struct MixedData* data, int n) {
    double acc = 0.0;
    volatile int v_cond = global_counter;
    
    /* Outer loop */
    for (int i = 0; i < OUTER_LOOP; i++) {
        /* Middle loop with volatile condition */
        for (int j = 0; j < MIDDLE_LOOP; j++) {
            if (v_cond & (1 << (j % 16))) {
                /* Innermost loop with carried dependency */
                for (int k = 0; k < INNER_LOOP; k++) {
                    int idx = (i * MIDDLE_LOOP * INNER_LOOP + 
                              j * INNER_LOOP + k) % n;
                    int prev_idx = (idx > 0) ? idx - 1 : n - 1;
                    
                    /* Complex reduction with mixed operations */
                    double temp = data[idx].weight * data[prev_idx].value;
                    acc = acc + temp;
                    
                    /* Data-dependent conditional */
                    if (data[idx].id % (simple_rand() % 8 + 2)) {
                        acc -= data[prev_idx].weight * 0.5;
                    }
                    
                    /* Volatile read to prevent dead code elimination */
                    v_cond = global_seed + k;
                }
            } else {
                /* Alternative path with different operations */
                for (int k = 0; k < INNER_LOOP / 2; k++) {
                    int idx = (i * MIDDLE_LOOP * (INNER_LOOP / 2) + 
                              j * (INNER_LOOP / 2) + k) % n;
                    acc += data[idx].id * 0.01;
                }
            }
        }
        
        /* Update volatile to change control flow */
        global_counter += i;
    }
    
    return acc;
}

/* Function with non-contiguous memory access patterns */
__attribute__((optimize("O3", "funroll-loops")))
float strided_processing(struct MixedData* data, int n) {
    float result = 0.0f;
    volatile int stride_selector = global_seed;
    
    /* Process with stride 3 for non-contiguous access */
    for (int base = 0; base < n; base += 3) {
        /* Nested loops with data-dependent conditions */
        for (int offset = 0; offset < 3; offset++) {
            int idx = base + offset;
            if (idx >= n) break;
            
            /* Multiple condition checks with volatile */
            if ((stride_selector & 1) && (data[idx].tag != 'X')) {
                for (int iter = 0; iter < 5; iter++) {
                    /* Mixed type operations */
                    result += data[idx].value * iter;
                    
                    /* Conditional store based on complex condition */
                    if (data[idx].id % (simple_rand() % 5 + 1)) {
                        data[idx].weight = result * 0.1;
                    }
                }
            }
            
            /* Update volatile variable */
            stride_selector = simple_rand();
        }
        
        /* Additional reduction with floating point */
        for (int i = 0; i < 2; i++) {
            result -= data[base % n].value * 0.3f;
        }
    }
    
    return result;
}

/* Deeply nested loop with volatile conditionals */
__attribute__((hot, optimize("O2")))
int nested_conditional(struct MixedData* data, int n) {
    int total = 0;
    volatile int v1 = global_counter;
    volatile int v2 = global_seed;
    
    /* Triple nested loop with volatile conditions */
    for (int a = 0; a < 8; a++) {
        if (v1++ % 3) {
            for (int b = 0; b < 12; b++) {
                if (v2 & (1 << (b % 8))) {
                    for (int c = 0; c < 25; c++) {
                        int idx = (a * 300 + b * 25 + c) % n;
                        
                        /* Complex integer arithmetic */
                        total += data[idx].id * c;
                        total -= data[idx].tag * b;
                        
                        /* Floating point conversion */
                        float ftemp = data[idx].value * a;
                        total += (int)ftemp;
                        
                        /* Memory access with stride */
                        if (c % 4 == 0) {
                            int stride_idx = (idx + 7) % n;
                            total += data[stride_idx].id;
                        }
                    }
                } else {
                    /* Alternative computation path */
                    for (int c = 0; c < 10; c++) {
                        total += simple_rand() % 100;
                    }
                }
                
                /* Update volatile in middle loop */
                v2 = simple_rand();
            }
        }
        
        /* Update volatile in outer loop */
        v1 = global_counter + a;
    }
    
    return total;
}

/* Main driver function */
int main(void) {
    /* Allocate and initialize data */
    struct MixedData* data = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    if (!data) return 1;
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        data[i].id = simple_rand() % 1000;
        data[i].value = (simple_rand() % 1000) / 10.0f;
        data[i].weight = (simple_rand() % 1000) / 100.0;
        data[i].tag = 'A' + (simple_rand() % 26);
        data[i].flag = simple_rand() % 2;
    }
    
    /* Update volatile globals */
    global_counter = simple_rand() % 100;
    global_seed = simple_rand() % 256;
    
    /* Call all computation functions to create varied scheduling patterns */
    double result1 = complex_reduction(data, SIZE);
    float result2 = strided_processing(data, SIZE);
    int result3 = nested_conditional(data, SIZE);
    
    /* Combine results to ensure all computations are live */
    double final_result = result1 + result2 + result3;
    
    /* Print result to prevent optimization */
    printf("Final result: %f\n", final_result);
    
    /* Additional volatile operations to maintain control flow complexity */
    for (int i = 0; i < 100; i++) {
        global_counter += data[i % SIZE].id;
        global_seed ^= data[i % SIZE].tag;
    }
    
    free(data);
    return 0;
}
