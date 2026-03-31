/* Complex program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 10000
#define INNER_LOOP 50
#define MID_LOOP 20
#define OUTER_LOOP 10

/* Volatile variables to prevent optimization of control flow */
volatile int v_cond1 = 0;
volatile int v_cond2 = 1;
volatile int v_cond3 = 2;

/* Mixed data type structure for complex memory access */
struct MixedData {
    int id;
    float value;
    double weight;
    char tag;
    int counter;
};

/* Function with carried dependency reduction pattern */
__attribute__((optimize("O2")))
double reduction_with_carry(struct MixedData* data, int n) {
    double acc = 0.0;
    volatile int seed = 12345;
    
    /* Complex loop with data-dependent control flow */
    for (int i = 2; i < n; i += (seed % 3) + 1) {  /* Non-unit stride */
        /* Create carried dependency */
        double temp = data[i].weight * data[i-1].value + data[i-2].id;
        
        /* Volatile condition prevents optimization */
        if (v_cond1 || (seed % 7) > 3) {
            acc = acc + temp;
            data[i].value = (float)(acc * 0.1);
        } else {
            acc = acc - temp * 0.5;
        }
        
        /* More operations to increase instruction count */
        data[i].counter = (int)(acc * 100) % 256;
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    return acc;
}

/* Function with deeply nested loops and mixed operations */
__attribute__((optimize("O2")))
int nested_loops_complex(int* arr, int n) {
    int total = 0;
    volatile int v1 = v_cond2;
    volatile int v2 = v_cond3;
    
    /* Outer loop - level 1 */
    for (int i = 0; i < OUTER_LOOP; i++) {
        /* Middle loop - level 2 */
        for (int j = 0; j < MID_LOOP; j++) {
            /* Inner loop - level 3 with volatile condition */
            for (int k = 0; k < INNER_LOOP; k++) {
                /* Data-dependent branching */
                if ((v1 > 0) && ((i * j + k) % 17 < v2)) {
                    /* Integer operations */
                    int idx = (i * 137 + j * 29 + k * 3) % n;
                    arr[idx] = arr[idx] * 3 + 7;
                    
                    /* Floating point operations mixed in */
                    float fval = (float)arr[idx] / 3.14159f;
                    if (fval > 100.0f) {
                        total += (int)fval;
                    }
                } else {
                    /* Alternative path with different operations */
                    int idx = (i * 97 + j * 13 + k * 5) % n;
                    arr[idx] = arr[idx] / 2 - 1;
                    total -= arr[idx];
                }
                
                /* More complex conditional */
                if ((i + j + k) % 23 == 0) {
                    v1 = (v1 * 1664525 + 1013904223) & 0x7fffffff;
                }
            }
            
            /* Loop-carried dependency */
            if (j % 4 == 0) {
                v2 = (v2 + total) & 0xff;
            }
        }
        
        /* Outer loop operation with memory access */
        for (int m = 0; m < 5; m++) {
            int idx = (i * 5 + m) % n;
            arr[idx] = total % 1000;
        }
    }
    
    return total;
}

/* Function with non-contiguous memory access pattern */
__attribute__((optimize("O3")))
double strided_access(struct MixedData* data, int n, int stride) {
    double sum = 0.0;
    volatile int toggle = 1;
    
    /* Process with non-contiguous stride */
    for (int i = 0; i < n; i += stride) {
        /* Complex conditional based on volatile */
        if (toggle || (i % 13 == 0)) {
            /* Mixed type operations */
            sum += data[i].weight * data[i].value;
            data[i].id = (int)(sum * 100);
            
            /* Additional floating point operation */
            double temp = data[i].weight;
            for (int j = 0; j < 3; j++) {
                temp = temp * 0.99 + 0.01;
            }
            data[i].weight = temp;
        } else {
            /* Alternative computation path */
            sum -= data[i].id * 0.5;
            data[i].value = (float)(sum * 0.01);
        }
        
        /* Update volatile to affect control flow */
        toggle = !toggle;
        
        /* Small inner loop for additional complexity */
        int local_sum = 0;
        for (int k = 0; k < 4; k++) {
            local_sum += data[i].counter + k;
        }
        data[i].counter = local_sum % 100;
    }
    
    return sum;
}

/* Main driver with initialization and multiple computation patterns */
int main() {
    /* Allocate and initialize data */
    struct MixedData* data = (struct MixedData*)malloc(SIZE * sizeof(struct MixedData));
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random data */
    int seed = 42;
    for (int i = 0; i < SIZE; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        data[i].id = seed % 1000;
        data[i].value = (float)(seed % 100) / 10.0f;
        data[i].weight = (double)(seed % 1000) / 100.0;
        data[i].tag = (char)('A' + (seed % 26));
        data[i].counter = seed % 100;
        
        int_array[i] = seed % 500;
    }
    
    double result1 = 0.0, result2 = 0.0, result3 = 0.0;
    int int_result = 0;
    
    /* Call different computation patterns to increase scheduling complexity */
    for (int iter = 0; iter < 3; iter++) {
        result1 += reduction_with_carry(data, SIZE - 100);
        int_result += nested_loops_complex(int_array, SIZE);
        result2 += strided_access(data, SIZE, 3);
        result3 += strided_access(data, SIZE, 4);
    }
    
    /* Combine results to ensure all computations are used */
    double final_result = result1 + result2 + result3 + int_result;
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %f\n", final_result);
    
    /* Cleanup */
    free(data);
    free(int_array);
    
    return 0;
}
