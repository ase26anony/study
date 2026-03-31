/* Complex loop structures to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

/* Mixed data type structure with non-contiguous access pattern */
struct mixed_data {
    int index;
    float f_value;
    double d_value;
    volatile int flag;  /* volatile to prevent optimization */
};

/* Global volatile variables to create data-dependent control flow */
volatile int g_volatile_counter = 0;
volatile int g_volatile_threshold = 100;

/* Function with complex loop nest and reduction pattern */
__attribute__((optimize("O2")))
double complex_reduction(struct mixed_data* data, int size) {
    double acc = 0.0;
    volatile int v_cond = g_volatile_counter;
    
    /* Outer loop with volatile condition */
    for (int i = 1; i < size; i += (v_cond % 3) + 1) {
        float temp_f = 0.0f;
        double temp_d = 0.0;
        
        /* Middle loop with data-dependent condition */
        for (int j = 0; j < (i % 8) + 2; j++) {
            /* Inner loop with carried dependency */
            for (int k = 0; k < 4; k++) {
                /* Critical reduction with carried dependency */
                acc = acc + data[i].d_value * data[i-1].f_value;
                
                /* Mixed operations to generate diverse RTL */
                temp_f += data[(i + j + k) % size].f_value * 0.5f;
                temp_d = temp_d * 1.01 + data[(i - j + k) % size].d_value;
                
                /* Volatile read to prevent optimization */
                if (g_volatile_counter > g_volatile_threshold) {
                    temp_f *= 1.1f;
                }
            }
            
            /* Conditional store based on volatile */
            if (v_cond % 7 == 0) {
                data[j].f_value = temp_f;
            }
        }
        
        /* Non-contiguous access pattern */
        if (i % 5 == 0) {
            acc += data[i * 3 % size].d_value;
        }
        
        /* Update volatile condition */
        v_cond = (v_cond * 1103515245 + 12345) & 0x7fffffff;
    }
    
    return acc;
}

/* Function with deeply nested loops and mixed operations */
__attribute__((optimize("O3")))
float nested_mixed_operations(struct mixed_data* data, int size) {
    float result = 0.0f;
    volatile int outer_cond = g_volatile_counter % 13;
    
    /* Triple nested loop with volatile conditions */
    for (int a = 0; a < size / 4; a += (outer_cond % 2) + 1) {
        double mid_acc = 0.0;
        
        for (int b = 0; b < 6; b++) {
            int inner_cond = (a * b + g_volatile_counter) % 11;
            
            for (int c = 0; c < 4; c++) {
                /* Complex addressing with mixed types */
                int idx = (a * 7 + b * 3 + c * 2) % size;
                
                /* Mixed precision operations */
                result += data[idx].f_value * (inner_cond % 5);
                mid_acc += data[idx].d_value * 0.25;
                
                /* Conditional with volatile */
                if (g_volatile_counter > (inner_cond * 50)) {
                    result -= data[(idx + 1) % size].f_value;
                }
                
                /* Memory access with stride */
                if (c % 2 == 0) {
                    data[idx].flag = inner_cond;
                }
            }
            
            /* Reduction across middle loop */
            result += (float)(mid_acc * 0.1);
        }
        
        /* Update volatile for next iteration */
        outer_cond = (outer_cond * 1664525 + 1013904223) & 0x7fffffff;
    }
    
    return result;
}

/* Function with pointer chasing and complex dependencies */
__attribute__((optimize("O2")))
double pointer_chasing_reduction(struct mixed_data* data, int size) {
    double sum = 0.0;
    struct mixed_data* current = &data[0];
    volatile int chase_counter = 0;
    
    for (int i = 0; i < size * 2; i++) {
        /* Pointer chasing with stride */
        int next_idx = (current->index + i * 3) % size;
        current = &data[next_idx];
        
        /* Reduction with carried dependency */
        sum = sum * 1.0001 + current->d_value;
        
        /* Complex conditional chain */
        if (chase_counter % 3 == 0) {
            sum += current->f_value * 2.0;
        } else if (chase_counter % 5 == 0) {
            sum -= current->d_value * 0.5;
        }
        
        /* Volatile update in loop */
        chase_counter = (chase_counter + g_volatile_counter) % 17;
        
        /* Nested loop with small iteration count */
        for (int j = 0; j < 3; j++) {
            sum += data[(next_idx + j) % size].f_value * j;
            
            /* Prevent optimization with volatile */
            if (g_volatile_counter > 50) {
                data[(next_idx + j) % size].flag = j;
            }
        }
    }
    
    return sum;
}

/* Initialize data with pseudo-random values */
void init_data(struct mixed_data* data, int size) {
    uint32_t seed = 123456789;
    
    for (int i = 0; i < size; i++) {
        seed = seed * 1103515245 + 12345;
        data[i].index = i;
        data[i].f_value = (float)((seed & 0xFFFF) / 65536.0) * 100.0f;
        
        seed = seed * 1103515245 + 12345;
        data[i].d_value = (double)((seed & 0xFFFF) / 65536.0) * 200.0;
        data[i].flag = 0;
    }
}

int main() {
    const int DATA_SIZE = 2048;  /* Large enough for non-trivial scheduling */
    struct mixed_data* data = malloc(DATA_SIZE * sizeof(struct mixed_data));
    
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    init_data(data, DATA_SIZE);
    
    /* Update volatile globals */
    g_volatile_counter = 75;
    g_volatile_threshold = 100;
    
    /* Call all computation functions to maximize scheduler activity */
    double result1 = complex_reduction(data, DATA_SIZE);
    float result2 = nested_mixed_operations(data, DATA_SIZE);
    double result3 = pointer_chasing_reduction(data, DATA_SIZE);
    
    /* Combine results to ensure all computations are live */
    double final_result = result1 + result2 + result3;
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %.6f\n", final_result);
    
    /* Additional volatile operations to create more scheduling opportunities */
    for (int i = 0; i < 100; i++) {
        g_volatile_counter = (g_volatile_counter * 13 + 7) % 1000;
        
        /* Small hot loop with mixed operations */
        double temp = 0.0;
        for (int j = 0; j < 16; j++) {
            temp += data[(i + j) % DATA_SIZE].d_value;
            if (g_volatile_counter % 3 == 0) {
                temp *= 1.01;
            }
        }
        final_result += temp;
    }
    
    printf("Adjusted result: %.6f\n", final_result);
    
    free(data);
    return 0;
}
