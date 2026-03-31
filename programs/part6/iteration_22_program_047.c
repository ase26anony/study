/* Test program to trigger scheduler context save/restore cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define ITER 1000

/* Function 1: Compute-intensive with tight floating-point loop */
void compute_intensive(float *a, float *b, float *c, int n) {
    for (int i = 0; i < n; i++) {
        // Mixed operations to create scheduling pressure
        float t = a[i] * b[i];
        t += a[i] / (b[i] + 1.0f);
        t = t * t - t / 2.0f;
        c[i] = t;
        
        // Conditional to create basic block boundaries
        if (c[i] > 100.0f) {
            c[i] = 100.0f;
        } else if (c[i] < -100.0f) {
            c[i] = -100.0f;
        }
    }
}

/* Function 2: Integer processing with branches and array manipulations */
int integer_processing(int *arr, int n) {
    int sum = 0;
    int prod = 1;
    
    // Nested loops with data dependencies
    for (int i = 0; i < n; i++) {
        // Arithmetic mix: multiply, add, shift
        arr[i] = (arr[i] * 3 + 7) >> 2;
        
        // Inner loop creates scheduling complexity
        for (int j = 0; j < 8; j++) {
            arr[i] ^= (arr[i] << j);
        }
        
        // Branch creates control flow
        if (arr[i] & 1) {
            sum += arr[i];
        } else {
            prod *= (arr[i] | 1);  // Ensure non-zero
        }
    }
    
    return sum + prod;
}

/* Function 3: Nested loops with function calls (scheduling barriers) */
void nested_loops_with_calls(int *matrix, int rows, int cols) {
    volatile int counter = 0;  // Prevent optimization
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            
            // Mixed operations
            matrix[idx] = (matrix[idx] * 7 + 3) % 256;
            
            // Function-like barrier (compiler can't see through volatile)
            counter++;
            
            // More arithmetic
            matrix[idx] ^= (matrix[idx] << 3);
            matrix[idx] |= 1;
            
            // Another barrier
            if (counter % 17 == 0) {
                matrix[idx] >>= 2;
            }
        }
    }
}

/* Function 4: Software pipelining candidate - reduction with loop carried dependency */
float reduction_pipeline(float *data, int n) {
    float sum1 = 0.0f, sum2 = 0.0f, sum3 = 0.0f, sum4 = 0.0f;
    
    // Unrolled loop with multiple accumulators
    for (int i = 0; i < n; i += 4) {
        sum1 += data[i];
        if (i + 1 < n) sum2 += data[i + 1];
        if (i + 2 < n) sum3 += data[i + 2];
        if (i + 3 < n) sum4 += data[i + 3];
        
        // Cross-iteration dependency
        sum1 = sum1 * 0.99f;
        sum2 = sum2 * 0.99f;
        sum3 = sum3 * 0.99f;
        sum4 = sum4 * 0.99f;
    }
    
    return sum1 + sum2 + sum3 + sum4;
}

/* Function 5: Complex control flow with switches */
int control_flow_test(int x, int iterations) {
    int result = x;
    
    for (int i = 0; i < iterations; i++) {
        // Switch creates multiple basic blocks
        switch (result % 5) {
            case 0:
                result = (result * 3) + 1;
                break;
            case 1:
                result = result ^ (result >> 3);
                break;
            case 2:
                result = result * result - result;
                break;
            case 3:
                result = (result << 4) | (result >> 28);
                break;
            case 4:
                result = ~result;
                break;
        }
        
        // Additional arithmetic
        result = (result + i) & 0xFFFF;
    }
    
    return result;
}

int main(int argc, char **argv) {
    // Use argv to prevent constant folding
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    srand(seed);
    
    // Allocate test arrays
    float *fa = malloc(SIZE * sizeof(float));
    float *fb = malloc(SIZE * sizeof(float));
    float *fc = malloc(SIZE * sizeof(float));
    
    int *arr1 = malloc(SIZE * sizeof(int));
    int *arr2 = malloc(SIZE * SIZE * sizeof(int));
    
    // Initialize with random data
    for (int i = 0; i < SIZE; i++) {
        fa[i] = (float)rand() / RAND_MAX * 200.0f - 100.0f;
        fb[i] = (float)rand() / RAND_MAX * 200.0f - 100.0f;
        arr1[i] = rand() % 1000;
    }
    
    for (int i = 0; i < SIZE * SIZE; i++) {
        arr2[i] = rand() % 1000;
    }
    
    // Call all functions to ensure they're compiled
    compute_intensive(fa, fb, fc, SIZE);
    
    int int_result = integer_processing(arr1, SIZE);
    printf("Integer result: %d\n", int_result);
    
    nested_loops_with_calls(arr2, SIZE / 16, 16);
    
    float float_result = reduction_pipeline(fa, SIZE);
    printf("Float result: %f\n", float_result);
    
    int cf_result = control_flow_test(seed, 1000);
    printf("Control flow result: %d\n", cf_result);
    
    // Cleanup
    free(fa);
    free(fb);
    free(fc);
    free(arr1);
    free(arr2);
    
    return 0;
}
