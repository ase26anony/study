/* Complex loop patterns to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 2000
#define INNER_ITERS 100

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile float g_volatile_float = 1.5f;

/* Mixed data type structure */
struct MixedData {
    int id;
    float value;
    double weight;
    char tag;
};

/* Function with complex reduction pattern and carried dependency */
__attribute__((optimize("O2")))
double complex_reduction(const double* data, int n) {
    double acc = 0.0;
    volatile int cond = g_volatile_counter;
    
    /* Loop with carried dependency */
    for (int i = 1; i < n; i++) {
        if (cond & 1) {
            acc = acc + data[i] * data[i-1] * g_volatile_float;
        } else {
            acc = acc - data[i] / (data[i-1] + 0.001);
        }
        
        /* Volatile conditional to prevent dead code elimination */
        if (g_volatile_counter++ % 7 == 0) {
            cond ^= (i & 0xFF);
        }
    }
    
    /* Additional computation to create more RTL patterns */
    for (int j = 0; j < INNER_ITERS; j++) {
        acc += (data[j % n] * j) / (g_volatile_float + 0.1);
    }
    
    return acc;
}

/* Function with nested loops and mixed data types */
__attribute__((optimize("O3")))
float nested_mixed_processing(struct MixedData* array, int size) {
    float total = 0.0f;
    volatile int outer_cond = g_volatile_counter;
    
    /* Triple nested loop */
    for (int i = 0; i < size / 4; i++) {
        double weight_acc = 0.0;
        
        for (int j = 0; j < 3; j++) {
            float value_acc = 0.0f;
            
            for (int k = 0; k < INNER_ITERS; k++) {
                /* Non-contiguous access pattern */
                int idx = (i * 3 + j) * 7 + (k % 5);
                if (idx < size) {
                    /* Mixed type operations */
                    value_acc += array[idx].value * 
                                (array[idx].weight / (k + 1.0));
                    
                    /* Conditional store based on volatile */
                    if (outer_cond++ % 11 == 0) {
                        array[idx].tag = (char)(value_acc * 0.01);
                    }
                }
                
                /* Data-dependent branch */
                if (g_volatile_float > 0.5f && (k & 3) == 0) {
                    weight_acc += value_acc * 0.5;
                }
            }
            
            total += value_acc;
        }
        
        /* Reduction with volatile dependency */
        if (g_volatile_counter & (1 << (i % 5))) {
            total += (float)(weight_acc * 0.3);
        }
    }
    
    return total;
}

/* Function with deeply nested control flow */
__attribute__((hot, optimize("O2")))
int complex_control_flow(int* data, int n) {
    int result = 0;
    volatile int v1 = g_volatile_counter;
    volatile float v2 = g_volatile_float;
    
    /* Multiple nested loops with complex conditions */
    for (int i = 0; i < n; i += 2) {
        int local_sum = 0;
        
        for (int j = 0; j < 5; j++) {
            float fp_temp = 0.0f;
            
            for (int k = 0; k < 8; k++) {
                /* Complex condition with mixed types */
                if ((v1++ % 13 == 0) || (v2 > 1.0f && k > 3)) {
                    fp_temp += (data[(i + j) % n] * k) / (v2 + 0.5f);
                    
                    /* Additional inner conditional */
                    if (j % 2 == 0 && fp_temp > 10.0f) {
                        local_sum += (int)(fp_temp * 0.1);
                    }
                } else {
                    fp_temp -= data[(i + j + 1) % n] * 0.01;
                }
                
                /* Memory access with stride */
                if (k % 3 == 0) {
                    data[(i * 7 + k) % n] += local_sum;
                }
            }
            
            result += local_sum;
            
            /* Volatile-dependent branch */
            if (g_volatile_counter % 19 == 0) {
                v2 = v2 * 0.9f + 0.1f;
            }
        }
    }
    
    return result;
}

/* Simple PRNG to avoid library dependencies */
static unsigned int prng_state = 123456789;
unsigned int simple_rand() {
    prng_state = prng_state * 1103515245 + 12345;
    return (prng_state >> 16) & 0x7FFF;
}

/* Initialize arrays with pseudo-random data */
void initialize_data(double* dbl_data, struct MixedData* mixed_data, int* int_data, int size) {
    for (int i = 0; i < size; i++) {
        dbl_data[i] = (simple_rand() % 1000) / 100.0;
        int_data[i] = simple_rand() % 10000;
        
        mixed_data[i].id = i;
        mixed_data[i].value = (simple_rand() % 500) / 10.0f;
        mixed_data[i].weight = (simple_rand() % 1000) / 50.0;
        mixed_data[i].tag = (char)('A' + (simple_rand() % 26));
    }
}

int main() {
    /* Allocate and initialize data */
    double* dbl_array = (double*)malloc(SIZE * sizeof(double));
    struct MixedData* mixed_array = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    
    if (!dbl_array || !mixed_array || !int_array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    initialize_data(dbl_array, mixed_array, int_array, SIZE);
    
    /* Call computation functions with different patterns */
    double result1 = complex_reduction(dbl_array, SIZE);
    float result2 = nested_mixed_processing(mixed_array, SIZE);
    int result3 = complex_control_flow(int_array, SIZE);
    
    /* Combine results to ensure code isn't optimized away */
    double final_result = result1 + result2 + result3;
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %f\n", final_result);
    
    /* Additional volatile operations to affect scheduling */
    for (int i = 0; i < 100; i++) {
        g_volatile_counter += (int_array[i % SIZE] & 0xF);
        g_volatile_float *= 0.99f;
    }
    
    /* Clean up */
    free(dbl_array);
    free(mixed_array);
    free(int_array);
    
    return 0;
}
