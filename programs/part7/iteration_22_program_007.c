/* test_sel_sched.c - Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 2048
#define INNER_LOOP 128
#define MIDDLE_LOOP 64
#define OUTER_LOOP 32

/* Volatile variables to prevent optimization of control flow */
volatile int g_volatile_counter = 0;
volatile float g_volatile_float = 1.5f;
volatile double g_volatile_double = 2.71828;

/* Mixed data type structure with non-contiguous access pattern */
struct MixedData {
    int id;
    float value;
    double weight;
    char tag;
    int32_t flags;
};

/* Simple LCG PRNG to avoid external dependencies */
static uint32_t lcg_seed = 12345;
static inline uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Function 1: Reduction with carried dependency across iterations */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
double reduction_with_carry(struct MixedData* data, int count) {
    double acc = 0.0;
    double prev = data[0].weight;
    
    /* Complex loop with data-dependent branching */
    for (int i = 1; i < count; i++) {
        /* Volatile read to force scheduler to consider side effects */
        int volatile_flag = g_volatile_counter;
        
        /* Carried dependency: uses value from previous iteration */
        double current = data[i].weight;
        acc += prev * current;
        
        /* Data-dependent conditional with volatile */
        if (volatile_flag & 0x1) {
            acc += data[i].value * g_volatile_float;
        } else {
            acc -= data[i-1].value * 0.5f;
        }
        
        /* Non-linear update with mixed operations */
        prev = current + (data[i].id % 100) * 0.01;
        
        /* Another volatile-dependent branch */
        if (g_volatile_float > 2.0f) {
            acc *= 1.0001;
        }
    }
    
    return acc;
}

/* Function 2: Nested loops with non-contiguous access and mixed types */
__attribute__((optimize("O3", "funroll-loops")))
float nested_loop_complex(struct MixedData* data, int size) {
    float total = 0.0f;
    
    /* Triple nested loop - outer loops */
    for (int o = 0; o < OUTER_LOOP; o++) {
        /* Volatile read in outer loop condition */
        int outer_cond = g_volatile_counter + o;
        
        for (int m = 0; m < MIDDLE_LOOP; m++) {
            /* Data-dependent middle loop limit */
            int middle_limit = (outer_cond % 8) + MIDDLE_LOOP / 2;
            if (m >= middle_limit) continue;
            
            for (int i = 0; i < INNER_LOOP; i++) {
                /* Non-contiguous access: process every 3rd element */
                int idx = (o * MIDDLE_LOOP * INNER_LOOP + 
                          m * INNER_LOOP + i) % size;
                if (idx % 3 != 0) continue;
                
                /* Mixed type operations */
                float val = data[idx].value;
                double weight = data[idx].weight;
                
                /* Complex conditional with volatile */
                if (g_volatile_double > weight) {
                    total += val * (float)weight;
                } else {
                    total -= val * 0.75f;
                }
                
                /* Integer arithmetic with carried dependency */
                int temp = data[idx].id;
                data[idx].flags = (temp << 3) | (data[idx].flags & 0x7);
                
                /* Another volatile-dependent operation */
                if (g_volatile_counter & (1 << (i % 5))) {
                    total *= 1.00001f;
                }
            }
            
            /* Volatile update in middle loop */
            g_volatile_counter += m % 7;
        }
    }
    
    return total;
}

/* Function 3: Deeply nested with reduction and pointer chasing */
__attribute__((hot, optimize("O2")))
double pointer_chasing_reduction(struct MixedData* data, int size) {
    double sum = 0.0;
    struct MixedData* current = &data[0];
    
    /* Multiple nested loops with pointer chasing */
    for (int layer1 = 0; layer1 < 16; layer1++) {
        /* Volatile-dependent loop bound */
        int bound1 = 8 + (g_volatile_counter % 8);
        
        for (int layer2 = 0; layer2 < bound1; layer2++) {
            /* Data-dependent inner loop iteration count */
            int iterations = (current->id % 16) + 4;
            
            for (int i = 0; i < iterations; i++) {
                /* Reduction with mixed operations */
                sum += current->value * current->weight;
                
                /* Conditional store based on volatile */
                if (g_volatile_float > current->value) {
                    current->weight *= 1.001;
                }
                
                /* Pointer chase with stride */
                int next_idx = (current->id + i) % size;
                current = &data[next_idx];
                
                /* Additional volatile check */
                if (g_volatile_counter & (1 << (i & 3))) {
                    sum -= 0.1 * current->value;
                }
            }
            
            /* Update volatile in outer loop */
            g_volatile_float += 0.01f;
        }
        
        /* Complex exit condition */
        if (sum > 1000000.0 || g_volatile_counter > 1000) {
            break;
        }
    }
    
    return sum;
}

/* Function 4: Matrix-style operations with triangular loops */
__attribute__((optimize("O3", "fsel-sched-pipelining-outer-loops")))
void matrix_style_operations(struct MixedData* data, int size, 
                            float* results, int results_size) {
    /* Triangular nested loops */
    for (int i = 0; i < size && i < results_size; i++) {
        float row_sum = 0.0f;
        
        for (int j = i; j < size; j += 3) { /* Non-unit stride */
            /* Complex addressing */
            int idx = (i * 7 + j * 3) % size;
            
            /* Mixed precision computation */
            double temp = data[idx].weight * g_volatile_double;
            
            /* Volatile-dependent operation */
            if (g_volatile_counter & (1 << (idx % 10))) {
                row_sum += (float)temp * data[j % size].value;
            } else {
                row_sum -= (float)(temp * 0.9);
            }
            
            /* Data-dependent conditional store */
            if (row_sum > data[idx].value) {
                data[idx].flags |= 0x1;
            }
        }
        
        results[i] = row_sum;
        
        /* Update volatile periodically */
        if (i % 17 == 0) {
            g_volatile_counter += i;
        }
    }
}

/* Main function that orchestrates all computations */
int main(void) {
    /* Allocate and initialize data with pseudo-random values */
    struct MixedData* data = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    float* results = (float*)malloc(SIZE * sizeof(float));
    
    if (!data || !results) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with LCG-generated values */
    for (int i = 0; i < SIZE; i++) {
        data[i].id = lcg_rand() % 1000;
        data[i].value = (lcg_rand() % 10000) / 100.0f;
        data[i].weight = (lcg_rand() % 20000) / 1000.0;
        data[i].tag = 'A' + (lcg_rand() % 26);
        data[i].flags = lcg_rand() & 0xFF;
    }
    
    /* Initialize results array */
    for (int i = 0; i < SIZE; i++) {
        results[i] = 0.0f;
    }
    
    /* Perform multiple complex computations to increase scheduler activity */
    double total_result = 0.0;
    
    /* Call each computation function multiple times with different parameters */
    for (int iter = 0; iter < 5; iter++) {
        g_volatile_counter = iter * 17;
        g_volatile_float = 1.0f + iter * 0.2f;
        g_volatile_double = 2.0 + iter * 0.1;
        
        total_result += reduction_with_carry(data, SIZE);
        total_result += nested_loop_complex(data, SIZE);
        total_result += pointer_chasing_reduction(data, SIZE);
        
        matrix_style_operations(data, SIZE, results, SIZE);
        
        /* Use results to prevent optimization */
        for (int i = 0; i < SIZE; i += 128) {
            total_result += results[i];
        }
    }
    
    /* Print final result to ensure code is live */
    printf("Final result: %f\n", total_result);
    printf("Volatile counter: %d\n", g_volatile_counter);
    printf("Volatile float: %f\n", g_volatile_float);
    printf("Volatile double: %lf\n", g_volatile_double);
    
    /* Cleanup */
    free(data);
    free(results);
    
    return 0;
}
