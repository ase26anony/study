/* Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 5000
#define INNER_LOOP 100
#define MIDDLE_LOOP 50

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile float g_volatile_float = 1.5f;

/* Mixed data type structure */
struct MixedData {
    int id;
    float value;
    double weight;
    char tag;
    int data[3];
};

/* Function 1: Reduction with carried dependency across iterations */
__attribute__((optimize("O2", "fsel-sched-pipelining")))
double reduction_with_dependency(double* arr, int n) {
    double acc = 0.0;
    double prev = arr[0];
    
    /* Complex loop with data-dependent control flow */
    for (int i = 1; i < n; i++) {
        /* Volatile condition to prevent optimization */
        if (g_volatile_counter++ % 7 == 0) {
            acc += arr[i] * prev;
        } else {
            acc += arr[i] / (prev + 1.0);
        }
        
        /* Non-trivial arithmetic creating scheduling complexity */
        double temp = arr[i] * g_volatile_float;
        if (temp > 100.0) {
            acc -= temp * 0.1;
        }
        
        prev = arr[i];
        
        /* Inner loop with volatile condition */
        for (int j = 0; j < INNER_LOOP; j++) {
            if ((g_volatile_counter + j) % 11 == 0) {
                acc += 0.001 * j;
            }
        }
    }
    
    return acc;
}

/* Function 2: Mixed data types with non-contiguous access */
__attribute__((optimize("O3", "funroll-loops")))
float process_mixed_data(struct MixedData* data, int count) {
    float total = 0.0f;
    volatile int skip = 3; /* Non-unit stride */
    
    /* Triple nested loop with mixed operations */
    for (int outer = 0; outer < count / 100; outer++) {
        for (int middle = 0; middle < MIDDLE_LOOP; middle++) {
            /* Data-dependent branching */
            if (g_volatile_float * middle > 20.0f) {
                for (int inner = 0; inner < INNER_LOOP; inner += skip) {
                    int idx = (outer * MIDDLE_LOOP + middle) * 3 + inner % 3;
                    if (idx < count) {
                        /* Mixed type operations */
                        total += data[idx].value * data[idx].weight;
                        
                        /* Conditional store based on volatile */
                        if (g_volatile_counter++ % 13 == 0) {
                            data[idx].data[inner % 3] = (int)(total * 100);
                        }
                        
                        /* Complex floating point chain */
                        double temp = data[idx].weight;
                        for (int k = 0; k < 5; k++) {
                            temp = temp * 0.9 + data[idx].value * 0.1;
                            if (k % 2 == 0 && g_volatile_float > 1.0f) {
                                total += (float)temp;
                            }
                        }
                    }
                }
            }
        }
    }
    
    return total;
}

/* Function 3: Deeply nested loops with volatile conditionals */
__attribute__((hot, optimize("O2")))
int complex_nested_loops(int* matrix, int rows, int cols) {
    int result = 0;
    
    /* 4-level nested loop */
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            
            /* Volatile conditional with side effect */
            volatile int local_volatile = g_volatile_counter;
            if (local_volatile % 17 == 0) {
                for (int k = 0; k < INNER_LOOP / 10; k++) {
                    /* Reduction with dependency */
                    int sum = 0;
                    for (int m = 0; m < k + 1; m++) {
                        sum += matrix[(idx + m) % (rows * cols)] * m;
                        
                        /* More volatile conditions */
                        if ((g_volatile_counter + m) % 19 == 0) {
                            sum -= matrix[(idx - m + rows * cols) % (rows * cols)];
                        }
                    }
                    result += sum;
                    
                    /* Floating point in integer loop */
                    if (k % 3 == 0) {
                        float ftemp = (float)sum * g_volatile_float;
                        result += (int)ftemp;
                    }
                }
            } else {
                /* Alternative path with different operations */
                for (int k = 0; k < INNER_LOOP / 20; k++) {
                    result ^= matrix[(idx + k) % (rows * cols)] << (k % 8);
                }
            }
            
            /* Update volatile */
            g_volatile_counter += (i * j) % 23;
        }
    }
    
    return result;
}

/* Simple PRNG to avoid library dependencies */
static unsigned int seed = 123456789;
unsigned int simple_rand() {
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
}

/* Initialize arrays with pseudo-random data */
void initialize_data(double* arr, int n, struct MixedData* mixed, int m, int* matrix, int size) {
    for (int i = 0; i < n; i++) {
        arr[i] = (simple_rand() % 1000) / 100.0;
    }
    
    for (int i = 0; i < m; i++) {
        mixed[i].id = i;
        mixed[i].value = (simple_rand() % 1000) / 50.0f;
        mixed[i].weight = (simple_rand() % 1000) / 200.0;
        mixed[i].tag = 'A' + (i % 26);
        for (int j = 0; j < 3; j++) {
            mixed[i].data[j] = simple_rand() % 100;
        }
    }
    
    for (int i = 0; i < size; i++) {
        matrix[i] = simple_rand() % 1000;
    }
}

int main() {
    /* Allocate and initialize data */
    double* array = (double*)malloc(SIZE * sizeof(double));
    struct MixedData* mixed_array = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    int* matrix = (int*)malloc(SIZE * SIZE / 100 * sizeof(int));
    
    if (!array || !mixed_array || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    initialize_data(array, SIZE, mixed_array, SIZE, matrix, SIZE * SIZE / 100);
    
    /* Call all computation functions to trigger various scheduling scenarios */
    double result1 = reduction_with_dependency(array, SIZE);
    float result2 = process_mixed_data(mixed_array, SIZE);
    int result3 = complex_nested_loops(matrix, SIZE / 10, 10);
    
    /* Combine results to ensure code isn't optimized away */
    double final_result = result1 + result2 + result3;
    
    /* Print result to prevent dead code elimination */
    printf("Final combined result: %f\n", final_result);
    printf("Volatile counter value: %d\n", g_volatile_counter);
    
    /* Cleanup */
    free(array);
    free(mixed_array);
    free(matrix);
    
    return 0;
}
