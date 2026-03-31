/* Test program to trigger selective scheduler debug dumps in GCC */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent optimizations */
volatile int g_volatile_counter = 0;

/* Function 1: Inner loop with conditional branch and memory write */
void test_inner_loop(int *arr, int n) {
    int i, j;
    int sum = 0;
    
    /* Create data dependencies that prevent simple scheduling */
    for (i = 0; i < n; i++) {
        /* Complex condition with side effects */
        if (arr[i] > 0) {
            sum += arr[i];
            /* Memory write with dependency on previous read */
            arr[i] = sum;
        } else {
            /* Alternative path with different operations */
            sum -= arr[i];
            /* Use inline asm to create unschedulable dependency */
            asm volatile ("" : : "r"(sum));
        }
        
        /* Nested loop to create more scheduling opportunities */
        for (j = 0; j < 3; j++) {
            /* Mix of arithmetic operations */
            sum = (sum * 2) + j;
            /* Prevent dead code elimination */
            g_volatile_counter += sum;
        }
    }
    
    /* Final computation with volatile to ensure it's not optimized away */
    asm volatile ("" : : "r"(sum));
}

/* Function 2: Nested loops with different iteration counts */
void test_nested_loops(int *matrix, int rows, int cols) {
    int i, j, k;
    int temp = 0;
    
    /* Outer loop - selective scheduler might pipeline this */
    for (i = 0; i < rows; i++) {
        /* Middle loop */
        for (j = 0; j < cols; j++) {
            /* Inner loop with data dependency chain */
            for (k = 0; k < 4; k++) {
                /* Complex addressing calculation */
                int idx = (i * cols + j) * 4 + k;
                
                /* Conditional store with dependency */
                if (matrix[idx] > temp) {
                    temp = matrix[idx];
                    /* Memory barrier effect */
                    asm volatile ("" : : "r"(temp));
                } else {
                    matrix[idx] = temp;
                }
                
                /* More arithmetic to create scheduling pressure */
                temp = (temp * 3 + 7) % 256;
            }
        }
        
        /* Function call within loop to create control flow complexity */
        g_volatile_counter += temp;
    }
}

/* Function 3: Switch statement with computed goto-like behavior */
void test_switch_complex(int mode, int *data, int size) {
    int i;
    int result = 0;
    
    /* Loop with switch inside - creates complex control flow */
    for (i = 0; i < size; i++) {
        switch (mode) {
            case 0:
                /* Simple arithmetic */
                result += data[i];
                /* Force spill/reload */
                asm volatile ("" : : "r"(result));
                break;
                
            case 1:
                /* More complex computation */
                result = (result << 1) ^ data[i];
                /* Conditional with side effect */
                if (result & 1) {
                    data[i] = result;
                }
                break;
                
            case 2:
                /* Nested conditionals */
                if (data[i] > 100) {
                    result -= data[i];
                } else if (data[i] < -100) {
                    result += data[i];
                } else {
                    result *= 2;
                }
                break;
                
            case 3:
                /* Memory intensive */
                result = data[data[i] % size];
                /* Prevent optimization */
                asm volatile ("" : : "r"(result));
                break;
                
            default:
                /* Default path with loop */
                for (int j = 0; j < 2; j++) {
                    result += j * data[i];
                }
                break;
        }
        
        /* Update volatile to prevent dead code elimination */
        g_volatile_counter ^= result;
    }
}

/* Function 4: Mixed control flow with function pointers */
void test_mixed_control_flow(int *arr, int n) {
    int i;
    int state = 0;
    
    /* Loop with varying control flow */
    for (i = 0; i < n; i++) {
        /* Multiple basic blocks within loop */
        if (i % 3 == 0) {
            state = arr[i] * 2;
            /* Create anti-dependency */
            asm volatile ("" : : "r"(state));
        } else if (i % 3 == 1) {
            state = arr[i] + state;
            /* Memory operation */
            arr[i] = state;
        } else {
            state = arr[i] - state;
            /* Complex expression */
            state = (state << 3) | (state >> 5);
        }
        
        /* Small inner loop */
        for (int j = 0; j < 2; j++) {
            state += j;
            /* Prevent optimization */
            g_volatile_counter += state;
        }
    }
}

/* Function 5: Data-dependent loop with early exit */
int test_data_dependent_loop(int *values, int count, int threshold) {
    int i;
    int total = 0;
    
    /* Loop with data-dependent exit condition */
    for (i = 0; i < count; i++) {
        total += values[i];
        
        /* Early exit based on computation */
        if (total > threshold) {
            /* Complex exit path */
            for (int j = 0; j < i; j++) {
                values[j] = total;
            }
            /* Force register pressure */
            asm volatile ("" : : "r"(total), "r"(i));
            break;
        }
        
        /* Continue path with more computation */
        total = (total * 3 + 1) / 2;
    }
    
    return total;
}

/* Main driver function */
int main(int argc, char **argv) {
    /* Allocate test arrays */
    int size1 = 100;
    int size2 = 50;
    int *arr1 = (int*)malloc(size1 * sizeof(int));
    int *arr2 = (int*)malloc(size2 * sizeof(int));
    int *matrix = (int*)malloc(10 * 10 * 4 * sizeof(int));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < size1; i++) {
        arr1[i] = (i * 37) % 101;
    }
    
    for (int i = 0; i < size2; i++) {
        arr2[i] = (i * 19) % 73;
    }
    
    for (int i = 0; i < 10 * 10 * 4; i++) {
        matrix[i] = (i * 13) % 256;
    }
    
    /* Call all test functions to ensure they're compiled */
    test_inner_loop(arr1, size1);
    test_nested_loops(matrix, 10, 10);
    test_switch_complex(argc % 4, arr2, size2);
    test_mixed_control_flow(arr1, size1);
    int result = test_data_dependent_loop(arr2, size2, 1000);
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d, Volatile counter: %d\n", result, g_volatile_counter);
    
    /* Clean up */
    free(arr1);
    free(arr2);
    free(matrix);
    
    return 0;
}
