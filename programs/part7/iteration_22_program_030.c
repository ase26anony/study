/* test_sel_sched.c - Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

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

/* Function 1: Reduction with carried dependency across iterations */
__attribute__((optimize("O2")))
double reduction_with_carry(const double* data, int size) {
    double acc = 0.0;
    double prev = data[0];
    
    /* Complex loop with data-dependent carried dependency */
    for (int i = 1; i < size; i++) {
        /* Volatile read to prevent optimization */
        int volatile_flag = g_volatile_counter & 0xFF;
        
        /* Carried dependency: uses value from previous iteration */
        double current = data[i];
        acc = acc + prev * current;
        
        /* Data-dependent conditional */
        if (volatile_flag > 128) {
            acc = acc - (prev * 0.5);
        }
        
        prev = current;
        
        /* Additional floating point operations */
        acc = acc / (1.0 + (i % 100) * 0.01);
    }
    
    return acc;
}

/* Function 2: Mixed data types with non-contiguous access */
__attribute__((optimize("O2")))
float process_mixed_data(struct MixedData* array, int count) {
    float total = 0.0f;
    double weight_acc = 0.0;
    
    /* Process every 3rd element with non-unit stride */
    for (int i = 0; i < count; i += 3) {
        /* Volatile condition */
        int should_process = (g_volatile_counter + i) % 7;
        
        if (should_process > 3) {
            /* Mixed type operations */
            total += array[i].value * array[i].weight;
            weight_acc += array[i].weight;
            
            /* Conditional store based on volatile */
            if (g_volatile_float > 1.0f) {
                array[i].data[0] = (int)(total * 100);
            }
        }
        
        /* Additional nested loop with data dependency */
        for (int j = 0; j < 2; j++) {
            total += array[i].data[j] * 0.01f;
        }
    }
    
    return total + (float)weight_acc;
}

/* Function 3: Deeply nested loops with volatile conditionals */
__attribute__((optimize("O3")))
int nested_loop_computation(int* matrix, int dim) {
    int result = 0;
    
    /* Triple nested loop - selective scheduler target */
    for (int i = 0; i < dim; i++) {
        /* Outer loop volatile check */
        volatile int outer_flag = g_volatile_counter + i;
        
        for (int j = 0; j < dim; j++) {
            /* Middle loop with data-dependent branch */
            if ((i * j) % 11 == 0) {
                for (int k = 0; k < dim; k++) {
                    /* Innermost loop with complex addressing */
                    int idx = (i * dim * dim) + (j * dim) + k;
                    
                    /* Volatile conditional in innermost loop */
                    if ((g_volatile_counter + idx) % 5 == 0) {
                        result += matrix[idx] * 2;
                    } else {
                        result -= matrix[idx];
                    }
                    
                    /* Additional floating point in integer loop */
                    float temp = (float)result / (k + 1);
                    result += (int)temp;
                    
                    /* Memory access pattern with stride */
                    if (k % 4 == 0) {
                        matrix[idx] = result % 256;
                    }
                }
            }
        }
        
        /* Reduction operation in outer loop */
        result = result % 10000;
    }
    
    return result;
}

/* Function 4: Complex reduction with multiple data types */
__attribute__((optimize("O2")))
double complex_reduction(int* int_data, float* float_data, double* double_data, int size) {
    double int_sum = 0.0;
    double float_sum = 0.0;
    double double_sum = 0.0;
    
    /* Loop with multiple carried dependencies */
    for (int i = 1; i < size; i++) {
        /* Chain of dependencies */
        double prev_int = int_data[i-1];
        double prev_float = float_data[i-1];
        
        /* Multiple accumulators with cross dependencies */
        int_sum = int_sum + prev_int * double_data[i];
        float_sum = float_sum + prev_float * int_data[i];
        double_sum = double_sum + int_sum * float_sum;
        
        /* Volatile-based conditional with side effect */
        if (g_volatile_counter++ % 1000 == 0) {
            double_sum = double_sum * 0.99;
        }
        
        /* Non-linear addressing */
        int mirror_idx = size - i - 1;
        if (mirror_idx > 0) {
            float_sum += float_data[mirror_idx] * 0.5;
        }
    }
    
    /* Final reduction with mixed types */
    return int_sum + float_sum + double_sum;
}

/* Simple PRNG for initialization (no library dependency) */
static unsigned int seed = 123456789;
unsigned int simple_rand() {
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
}

/* Initialize arrays with pseudo-random data */
void initialize_data(double* double_arr, int* int_arr, float* float_arr, 
                     struct MixedData* mixed_arr, int size) {
    for (int i = 0; i < size; i++) {
        double_arr[i] = (simple_rand() % 1000) / 100.0;
        int_arr[i] = simple_rand() % 10000;
        float_arr[i] = (simple_rand() % 1000) / 50.0f;
        
        if (i % 3 == 0) {
            mixed_arr[i].id = i;
            mixed_arr[i].value = (simple_rand() % 1000) / 100.0f;
            mixed_arr[i].weight = (simple_rand() % 1000) / 200.0;
            mixed_arr[i].tag = 'A' + (i % 26);
            for (int j = 0; j < 3; j++) {
                mixed_arr[i].data[j] = simple_rand() % 100;
            }
        }
    }
}

int main() {
    const int DATA_SIZE = 5000;
    const int MATRIX_DIM = 50;
    const int MATRIX_SIZE = MATRIX_DIM * MATRIX_DIM * MATRIX_DIM;
    
    /* Allocate and initialize data */
    double* double_data = (double*)malloc(DATA_SIZE * sizeof(double));
    int* int_data = (int*)malloc(DATA_SIZE * sizeof(int));
    float* float_data = (float*)malloc(DATA_SIZE * sizeof(float));
    struct MixedData* mixed_data = (struct MixedData*)malloc(DATA_SIZE * sizeof(struct MixedData));
    int* matrix_data = (int*)malloc(MATRIX_SIZE * sizeof(int));
    
    if (!double_data || !int_data || !float_data || !mixed_data || !matrix_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    initialize_data(double_data, int_data, float_data, mixed_data, DATA_SIZE);
    
    for (int i = 0; i < MATRIX_SIZE; i++) {
        matrix_data[i] = simple_rand() % 1000;
    }
    
    /* Execute all computation functions to trigger various scheduler patterns */
    double result1 = reduction_with_carry(double_data, DATA_SIZE);
    printf("Result 1 (reduction with carry): %.6f\n", result1);
    
    float result2 = process_mixed_data(mixed_data, DATA_SIZE);
    printf("Result 2 (mixed data): %.6f\n", result2);
    
    int result3 = nested_loop_computation(matrix_data, MATRIX_DIM);
    printf("Result 3 (nested loops): %d\n", result3);
    
    double result4 = complex_reduction(int_data, float_data, double_data, DATA_SIZE);
    printf("Result 4 (complex reduction): %.6f\n", result4);
    
    /* Combine results to ensure all computations are used */
    double final_result = result1 + result2 + result3 + result4;
    printf("Final combined result: %.6f\n", final_result);
    
    /* Update volatile variables to affect future executions */
    g_volatile_counter = (int)final_result % 1000;
    g_volatile_float = (float)(final_result / 1000.0);
    
    /* Cleanup */
    free(double_data);
    free(int_data);
    free(float_data);
    free(mixed_data);
    free(matrix_data);
    
    return 0;
}
