/* Complex test program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 2048
#define INNER_LOOP 100
#define MIDDLE_LOOP 50
#define OUTER_LOOP 20

/* Volatile variables to prevent optimization of control flow */
volatile int g_volatile_counter = 0;
volatile float g_volatile_float = 0.5f;

/* Mixed data type structure with non-contiguous access pattern */
struct MixedData {
    int id;
    float value;
    double weight;
    char tag;
    int counter;
};

/* Function 1: Reduction with carried dependency across iterations */
__attribute__((optimize("O2")))
double reduction_with_dependency(struct MixedData* data, int count) {
    double acc = 0.0;
    double prev = data[0].weight;
    
    /* Complex loop with data-dependent control flow */
    for (int i = 1; i < count; i += 3) {  /* Non-unit stride */
        /* Volatile read to prevent optimization */
        volatile int cond = g_volatile_counter & (1 << (i & 7));
        
        /* Carried dependency: uses result from previous iteration */
        double current = data[i].weight;
        double product = prev * current;
        
        /* Data-dependent branch */
        if (cond) {
            acc += product * data[i].value;
        } else {
            acc -= product * (1.0 - data[i].value);
        }
        
        /* Mixed operations */
        data[i].counter += (int)(acc * 100);
        prev = current;
        
        /* Additional volatile dependency */
        if (g_volatile_float > 0.3f) {
            data[i].value *= 1.01f;
        }
    }
    
    return acc;
}

/* Function 2: Deeply nested loops with volatile conditionals */
__attribute__((optimize("O3")))
int nested_loops_complex(int* array, int size) {
    int total = 0;
    volatile int outer_cond = g_volatile_counter % 7;
    
    /* Outer loop */
    for (int o = 0; o < OUTER_LOOP; o++) {
        volatile int middle_cond = (g_volatile_counter + o) % 11;
        
        /* Middle loop */
        for (int m = 0; m < MIDDLE_LOOP; m++) {
            volatile int inner_cond = (g_volatile_counter + m) % 13;
            
            /* Innermost loop with data-dependent branching */
            for (int i = 0; i < INNER_LOOP; i++) {
                int index = (o * 100 + m * 10 + i) % size;
                
                /* Complex conditional with mixed operations */
                if (outer_cond > 3 && middle_cond < 8) {
                    if (inner_cond % 2 == 0) {
                        array[index] += i * m;
                        total += array[index] * 2;
                    } else {
                        array[index] -= i / (m + 1);
                        total -= array[index] / 3;
                    }
                } else if (inner_cond % 3 == 0) {
                    array[index] *= 2;
                    total += array[index] >> 1;
                }
                
                /* Floating point operation in integer loop */
                if (g_volatile_float > 0.0f) {
                    float temp = (float)array[index] * g_volatile_float;
                    array[index] = (int)temp;
                }
            }
            
            /* Update volatile condition */
            middle_cond = (middle_cond * 13 + 7) % 17;
        }
        
        outer_cond = (outer_cond * 11 + 5) % 19;
    }
    
    return total;
}

/* Function 3: Mixed data type processing with pointer arithmetic */
__attribute__((optimize("O2")))
float process_mixed_types(struct MixedData* data, int count) {
    float result = 0.0f;
    struct MixedData* ptr = data;
    
    /* Loop with pointer arithmetic and type conversions */
    for (int i = 0; i < count; i++) {
        /* Access every 4th element with pointer stride */
        ptr = data + (i * 4) % count;
        
        /* Complex expression with mixed types */
        double temp = (double)ptr->value * ptr->weight;
        
        /* Conditional store based on volatile */
        if (g_volatile_counter++ % 5 == 0) {
            ptr->value = (float)(temp * 0.95);
            result += ptr->value;
        } else {
            ptr->weight = temp * 1.05;
            result -= (float)ptr->weight;
        }
        
        /* Integer operation with dependency */
        ptr->counter = (ptr->counter + i) % 1000;
        
        /* Additional floating point chain */
        if (i % 3 == 0) {
            float chain = result;
            for (int j = 0; j < 5; j++) {
                chain = chain * 0.9f + ptr->value;
                if (g_volatile_float > chain) {
                    chain += 0.1f;
                }
            }
            result = chain;
        }
    }
    
    return result;
}

/* Function 4: Matrix-like operations with reduction */
__attribute__((optimize("O3")))
double matrix_reduction(double* matrix, int rows, int cols) {
    double total = 0.0;
    
    for (int r = 0; r < rows; r++) {
        double row_acc = 0.0;
        volatile int row_cond = (r + g_volatile_counter) % 4;
        
        for (int c = 0; c < cols; c++) {
            int idx = r * cols + c;
            
            /* Pattern that creates scheduling challenges */
            if (row_cond == 0) {
                matrix[idx] = matrix[idx] * 1.1 + c * 0.01;
                row_acc += matrix[idx] * (r + 1);
            } else if (row_cond == 1) {
                matrix[idx] = matrix[idx] / 1.1 - r * 0.01;
                row_acc -= matrix[idx] * (c + 1);
            } else {
                matrix[idx] = (matrix[idx] + r * c) * 0.5;
                row_acc *= 1.0 + matrix[idx] * 0.001;
            }
            
            /* Cross-iteration dependency */
            if (c > 0) {
                matrix[idx] += matrix[idx - 1] * 0.1;
            }
        }
        
        total += row_acc;
        
        /* Volatile update every few iterations */
        if (r % 7 == 0) {
            g_volatile_float = (float)(total * 0.001);
        }
    }
    
    return total;
}

/* Simple pseudo-random generator to avoid library dependencies */
static unsigned int seed = 123456789;
unsigned int simple_rand() {
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
}

/* Initialize data with pseudo-random values */
void initialize_data(struct MixedData* data, int count, double* matrix, int matrix_size) {
    for (int i = 0; i < count; i++) {
        data[i].id = i;
        data[i].value = (float)(simple_rand() % 1000) / 1000.0f;
        data[i].weight = (double)(simple_rand() % 2000) / 1000.0;
        data[i].tag = (char)('A' + (i % 26));
        data[i].counter = simple_rand() % 10000;
    }
    
    for (int i = 0; i < matrix_size; i++) {
        matrix[i] = (double)(simple_rand() % 10000) / 100.0;
    }
}

int main() {
    /* Allocate and initialize data */
    struct MixedData* data = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    double* matrix = (double*)malloc(SIZE * SIZE / 4 * sizeof(double));
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = simple_rand() % 1000;
    }
    
    initialize_data(data, SIZE, matrix, SIZE * SIZE / 4);
    
    /* Call all computation functions to create varied scheduling scenarios */
    double result1 = reduction_with_dependency(data, SIZE);
    int result2 = nested_loops_complex(int_array, SIZE);
    float result3 = process_mixed_types(data, SIZE);
    double result4 = matrix_reduction(matrix, SIZE / 16, SIZE / 16);
    
    /* Combine results to prevent dead code elimination */
    double final_result = result1 + result2 + result3 + result4;
    
    /* Print result to ensure code is live */
    printf("Final combined result: %f\n", final_result);
    printf("Volatile counter: %d, Volatile float: %f\n", 
           g_volatile_counter, g_volatile_float);
    
    /* Cleanup */
    free(data);
    free(int_array);
    free(matrix);
    
    return 0;
}
