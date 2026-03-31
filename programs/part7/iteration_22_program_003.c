/* Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 2000
#define INNER_LOOP 50
#define MIDDLE_LOOP 20
#define OUTER_LOOP 10

/* Mixed data type structure with padding */
struct MixedData {
    int id;
    float value;
    double weight;
    char tag;
    int counter;
    volatile int flag;  /* volatile to prevent optimization */
};

/* Global volatile variables to create data-dependent control flow */
volatile int global_seed = 42;
volatile int global_mod = 7;

/* Function with complex reduction pattern and carried dependency */
__attribute__((optimize("O2")))
double complex_reduction(struct MixedData* data, int n) {
    double acc = 0.0;
    volatile int cond = global_seed;
    
    /* Triple nested loop with data-dependent conditions */
    for (int i = 0; i < OUTER_LOOP; i++) {
        volatile int outer_cond = (cond % 3) == 0;
        
        for (int j = 0; j < MIDDLE_LOOP; j++) {
            /* Non-trivial condition with volatile */
            if ((global_seed + i * j) % 5 == 0) {
                volatile int middle_flag = 1;
            }
            
            for (int k = 0; k < INNER_LOOP; k++) {
                /* Data-dependent branch with volatile */
                volatile int should_process = ((i + j + k + global_seed) & 1);
                
                if (should_process) {
                    /* Carried dependency across iterations */
                    int idx = (i * MIDDLE_LOOP * INNER_LOOP + 
                              j * INNER_LOOP + k) % n;
                    
                    /* Non-contiguous access pattern */
                    if (idx > 0 && idx < n - 1) {
                        /* Reduction with carried dependency */
                        acc += data[idx].weight * data[idx-1].value;
                        acc -= data[idx].value * data[idx+1].weight;
                        
                        /* Mixed type operations */
                        data[idx].counter += (int)(acc * 0.1);
                        data[idx].value = (float)(acc * 0.01);
                    }
                    
                    /* Conditional store based on volatile */
                    if ((global_seed + idx) % global_mod == 0) {
                        data[idx].tag = 'X';
                    }
                } else {
                    /* Alternative path with different operations */
                    int idx = (j * INNER_LOOP + k) % n;
                    if (idx % 3 == 0) {  /* Non-unit stride */
                        data[idx].id = i * j * k;
                    }
                }
                
                /* Volatile operation to prevent dead code elimination */
                global_seed = (global_seed * 1103515245 + 12345) & 0x7fffffff;
            }
        }
    }
    
    return acc;
}

/* Function with pointer arithmetic and non-contiguous access */
__attribute__((optimize("O3")))
float pointer_arithmetic_reduction(struct MixedData* data, int n) {
    float sum = 0.0f;
    volatile int skip = 3;  /* Non-unit stride */
    
    /* Deeply nested loops with complex indexing */
    for (int a = 0; a < 8; a++) {
        volatile int a_mod = (global_seed + a) % 4;
        
        for (int b = 0; b < 12; b++) {
            /* Data-dependent loop bound */
            int limit = (a_mod + b) % 8 + 4;
            
            for (int c = 0; c < limit; c++) {
                /* Complex index calculation */
                int idx = (a * 100 + b * 10 + c * skip) % n;
                
                if (idx >= 0 && idx < n) {
                    /* Mixed operations creating diverse RTL */
                    double temp = data[idx].weight;
                    sum += (float)temp * data[idx].value;
                    
                    /* Conditional with volatile */
                    if ((global_seed + idx) & 1) {
                        data[idx].value = sum * 0.5f;
                    } else {
                        data[idx].value = -sum * 0.25f;
                    }
                    
                    /* Memory access pattern that's hard to optimize */
                    if (idx > skip && (idx % skip) == 0) {
                        data[idx - skip].weight = temp * 0.9;
                    }
                }
                
                /* Update volatile to affect control flow */
                global_mod = (global_mod * 17 + 1) % 13;
            }
        }
    }
    
    return sum;
}

/* Function with reduction and complex control flow */
__attribute__((hot, optimize("O2")))
int integer_reduction_with_branches(int* array, int n) {
    int max_val = -1000000;
    int min_val = 1000000;
    volatile int threshold = global_seed % 100;
    
    /* Loop with multiple exit points and conditions */
    for (int i = 0; i < n; i += 2) {  /* Process every other element */
        /* Multiple volatile conditions */
        volatile int cond1 = (i % 3) == (global_seed % 3);
        volatile int cond2 = (array[i] > threshold);
        volatile int cond3 = ((i + global_mod) & 7) == 0;
        
        if (cond1 && cond2) {
            /* Find maximum with dependency chain */
            if (array[i] > max_val) {
                max_val = array[i];
                /* Additional operation on max update */
                threshold = (threshold + max_val) % 50;
            }
            
            /* Nested conditional */
            if (cond3) {
                /* Complex operation chain */
                int temp = array[i] * 2 - array[i/2];
                array[i] = temp % 1000;
                
                /* Another volatile update */
                global_seed = (global_seed + temp) & 0xfff;
            }
        } else if (!cond1 || cond3) {
            /* Alternative path for minimum */
            if (array[i] < min_val) {
                min_val = array[i];
            }
            
            /* Modify array with stride */
            if (i + 4 < n) {
                array[i + 4] = array[i] * 3;
            }
        }
        
        /* Periodic volatile update affecting future iterations */
        if (i % 17 == 0) {
            global_mod = (global_mod + 1) % 11;
        }
    }
    
    return max_val - min_val;
}

/* Initialize data with pseudo-random values */
void initialize_data(struct MixedData* data, int n, int* int_array) {
    unsigned int seed = 123456789;
    
    for (int i = 0; i < n; i++) {
        /* Simple LCG for reproducibility */
        seed = seed * 1103515245 + 12345;
        
        data[i].id = i;
        data[i].value = (float)((seed % 1000) * 0.01);
        data[i].weight = (double)((seed % 2000) * 0.001);
        data[i].tag = 'A' + (i % 26);
        data[i].counter = 0;
        data[i].flag = (seed & 1);
        
        int_array[i] = (int)(seed % 10000) - 5000;
    }
}

int main() {
    /* Allocate and initialize data */
    struct MixedData* data = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    
    if (!data || !int_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    initialize_data(data, SIZE, int_array);
    
    printf("Starting complex computations...\n");
    
    /* Call all computation functions to increase scheduling complexity */
    double result1 = complex_reduction(data, SIZE);
    printf("Result 1: %f\n", result1);
    
    float result2 = pointer_arithmetic_reduction(data, SIZE);
    printf("Result 2: %f\n", result2);
    
    int result3 = integer_reduction_with_branches(int_array, SIZE);
    printf("Result 3: %d\n", result3);
    
    /* Final combination to ensure all results are used */
    double final_result = result1 + result2 + result3;
    printf("Final combined result: %f\n", final_result);
    
    /* Print some data to prevent complete optimization */
    printf("Sample data[100]: id=%d, value=%f, weight=%f\n", 
           data[100].id, data[100].value, data[100].weight);
    
    free(data);
    free(int_array);
    
    return 0;
}
