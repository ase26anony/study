/* Complex selective scheduling test program */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Mixed data type structure with non-contiguous access pattern */
struct MixedData {
    int index;
    float weight;
    double value;
    char tag;
    volatile int marker;  /* volatile to prevent optimization */
};

/* Function with deep loop nest and data-dependent control flow */
__attribute__((optimize("O2,unroll-loops")))
static double complex_reduction(const double* data, int size) {
    volatile int seed = 42;  /* volatile to prevent constant propagation */
    double acc = 0.0;
    double prev = data[0];
    
    /* Outer loop with volatile condition */
    for (int i = 1; i < size; i += (seed & 3) + 1) {
        double temp_sum = 0.0;
        
        /* Middle loop with data-dependent iteration count */
        int inner_limit = (int)(data[i] * 10) % 8 + 2;
        for (int j = 0; j < inner_limit; j++) {
            /* Innermost loop with reduction pattern */
            for (int k = 0; k < 4; k++) {
                /* Carried dependency across iterations */
                acc = acc + data[i] * prev;
                
                /* Mixed operations to generate diverse RTL */
                if ((k & 1) == 0) {
                    acc = acc - data[i] * 0.5;
                } else {
                    acc = acc + data[i] * 1.5;
                }
                
                /* Volatile read to prevent dead code elimination */
                if (seed > 0) {
                    acc = acc * 1.0001;
                }
            }
            prev = data[i] + j * 0.01;
        }
        
        /* Data-dependent conditional store */
        if (acc > 1000.0) {
            acc = acc * 0.99;
        }
    }
    
    return acc;
}

/* Function processing mixed data types with non-contiguous access */
__attribute__((optimize("O2")))
static double process_mixed_data(struct MixedData* array, int count) {
    volatile int offset = 0;
    double total = 0.0;
    float float_acc = 0.0f;
    int int_acc = 0;
    
    /* Process every 3rd element with stride */
    for (int i = 0; i < count; i += 3) {
        /* Nested loops with mixed operations */
        for (int j = 0; j < 2; j++) {
            /* Access structure members with different types */
            total += array[i].value * array[i].weight;
            int_acc += array[i].index * j;
            float_acc += array[i].weight * 0.5f;
            
            /* Volatile conditional to prevent optimization */
            if (offset < 10) {
                total = total - array[i].value * 0.1;
            }
            
            /* Another level of nesting */
            for (int k = 0; k < 2; k++) {
                /* Complex expression with type conversions */
                total += (double)array[i].index * 0.01;
                total += (double)((array[i].tag + k) % 256) * 0.001;
            }
        }
        
        /* Data-dependent branch with volatile */
        if (array[i].marker > 50 || offset > 5) {
            total = total * 1.05;
            offset = offset + 1;
        }
    }
    
    return total + float_acc + int_acc;
}

/* Function with deeply nested loops and volatile conditions */
__attribute__((optimize("O3")))
static int nested_loop_computation(int* matrix, int rows, int cols) {
    volatile int threshold = 100;
    int result = 0;
    
    /* Triple nested loop with volatile in condition */
    for (int i = 0; i < rows && threshold > 0; i += (threshold & 1) + 1) {
        for (int j = 0; j < cols; j += 2) {
            int local_sum = 0;
            
            /* Innermost complex loop */
            for (int k = 0; k < 4; k++) {
                /* Reduction with carried dependency */
                local_sum = local_sum + matrix[i * cols + j] * k;
                
                /* Conditional operations */
                if ((local_sum & 1) == 0) {
                    local_sum = local_sum >> 1;
                } else {
                    local_sum = local_sum * 3 + 1;
                }
                
                /* Memory access with stride */
                if (k > 0) {
                    local_sum += matrix[(i * cols + j + k) % (rows * cols)];
                }
            }
            
            result += local_sum;
            
            /* Volatile update to prevent optimization */
            if (threshold < 200) {
                threshold = threshold + (result & 1);
            }
        }
    }
    
    return result;
}

/* Simple pseudo-random generator to avoid library dependencies */
static uint32_t lcg_pseudo_rand(uint32_t* state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

/* Initialize array with pseudo-random data */
static void init_data(double* data, int size) {
    uint32_t state = 42;
    for (int i = 0; i < size; i++) {
        data[i] = (double)(lcg_pseudo_rand(&state) % 1000) / 10.0;
    }
}

/* Initialize mixed data array */
static void init_mixed_data(struct MixedData* array, int count) {
    uint32_t state = 123;
    for (int i = 0; i < count; i++) {
        array[i].index = lcg_pseudo_rand(&state) % 100;
        array[i].weight = (float)(lcg_pseudo_rand(&state) % 100) / 10.0f;
        array[i].value = (double)(lcg_pseudo_rand(&state) % 1000) / 100.0;
        array[i].tag = (char)(lcg_pseudo_rand(&state) % 256);
        array[i].marker = lcg_pseudo_rand(&state) % 100;
    }
}

int main(void) {
    const int DATA_SIZE = 2048;
    const int MIXED_SIZE = 1024;
    const int MATRIX_SIZE = 64;
    
    /* Allocate and initialize data */
    double* data = (double*)malloc(DATA_SIZE * sizeof(double));
    struct MixedData* mixed = (struct MixedData*)malloc(MIXED_SIZE * sizeof(struct MixedData));
    int* matrix = (int*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    
    if (!data || !mixed || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_data(data, DATA_SIZE);
    init_mixed_data(mixed, MIXED_SIZE);
    
    /* Initialize matrix with pseudo-random values */
    uint32_t state = 456;
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        matrix[i] = lcg_pseudo_rand(&state) % 1000;
    }
    
    /* Perform computations to trigger selective scheduling */
    double result1 = complex_reduction(data, DATA_SIZE);
    double result2 = process_mixed_data(mixed, MIXED_SIZE);
    int result3 = nested_loop_computation(matrix, MATRIX_SIZE, MATRIX_SIZE);
    
    /* Combine results to ensure code isn't optimized away */
    double final_result = result1 + result2 + result3;
    
    printf("Computation result: %f\n", final_result);
    printf("Breakdown: reduction=%.2f, mixed=%.2f, nested=%d\n", 
           result1, result2, result3);
    
    /* Cleanup */
    free(data);
    free(mixed);
    free(matrix);
    
    return 0;
}
