/* Test program to trigger selective scheduler debug dumps in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Inner loop with conditional branch and memory writes */
void test_inner_loop(int *arr, int n) {
    volatile int sum = 0;  /* Prevent dead code elimination */
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Create data dependencies */
        int temp = arr[i];
        
        /* Conditional store with branch */
        if (temp > 100) {
            arr[i] = temp * 2;
            sum += arr[i];
        } else {
            arr[i] = temp / 2;
            sum -= arr[i];
        }
        
        /* Inline assembly to create unschedulable dependencies */
        asm volatile ("" : : "r"(sum) : "memory");
        
        /* Nested loop for more complexity */
        for (j = 0; j < 5; j++) {
            arr[i] += j;
            asm volatile ("" : : "r"(arr[i]) : "memory");
        }
    }
    
    /* Force use of result */
    asm volatile ("" : : "r"(sum) : "memory");
}

/* Function 2: Nested loops with different iteration counts */
int test_nested_loops(int size) {
    volatile int counter = 0;
    int i, j, k;
    int *matrix = (int*)malloc(size * size * sizeof(int));
    
    if (!matrix) return -1;
    
    /* Triple nested loop for outer loop pipelining */
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            int idx = i * size + j;
            matrix[idx] = i + j;
            
            /* Inner loop with data dependency */
            for (k = 0; k < 3; k++) {
                matrix[idx] += k * (i - j);
                
                /* Conditional with unpredictable branch */
                if ((i ^ j) & 1) {
                    matrix[idx] *= 2;
                } else {
                    matrix[idx] /= 2;
                }
                
                /* Memory barrier */
                asm volatile ("" : : "r"(matrix[idx]) : "memory");
            }
            
            counter += matrix[idx];
        }
        
        /* Function call within loop to create control flow */
        if (i % 10 == 0) {
            counter += rand() % 100;
        }
    }
    
    int result = counter;
    free(matrix);
    
    /* Ensure result is used */
    asm volatile ("" : : "r"(result) : "memory");
    return result;
}

/* Function 3: Complex control flow with switch and computed goto */
void test_complex_cf(int mode, int iterations) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    volatile int state = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Switch with multiple cases */
        switch (mode) {
            case 0:
                state += i * 2;
                goto *labels[i % 4];
            case 1:
                state -= i * 3;
                if (state > 100) mode = 2;
                break;
            case 2:
                state *= i;
                if (state < 0) state = 0;
                break;
            default:
                state = i % 100;
                break;
        }
        
        /* Label-based computed goto */
        label0:
            state += 1;
            continue;
        label1:
            state += 2;
            mode = (mode + 1) % 3;
            continue;
        label2:
            state += 3;
            asm volatile ("" : : "r"(state) : "memory");
            continue;
        label3:
            state += 4;
            break;
    }
    
    /* Array access with pointer arithmetic */
    int array[100];
    int *ptr = array;
    for (i = 0; i < 100; i++) {
        *ptr++ = state + i;
        asm volatile ("" : : "r"(array[i]) : "memory");
    }
}

/* Function 4: Mixed operations with floating point */
double test_mixed_ops(int n) {
    volatile double acc = 0.0;
    int i;
    double *values = (double*)malloc(n * sizeof(double));
    
    if (!values) return 0.0;
    
    /* Initialize with pattern */
    for (i = 0; i < n; i++) {
        values[i] = (i % 10) * 1.5;
    }
    
    /* Loop with FP operations and conditions */
    for (i = 1; i < n; i++) {
        double prev = values[i-1];
        double curr = values[i];
        
        /* Create FP dependencies */
        if (prev > curr) {
            values[i] = prev * 1.1;
            acc += values[i];
        } else {
            values[i] = curr / 1.1;
            acc -= values[i];
        }
        
        /* Integer operation mixed in */
        int idx = (int)values[i] % n;
        values[idx] += 0.5;
        
        /* Memory barrier */
        asm volatile ("" : : "r"(acc) : "memory");
    }
    
    double result = acc;
    free(values);
    
    /* Force use of result */
    asm volatile ("" : : "r"(result) : "memory");
    return result;
}

/* Main driver to ensure all functions are compiled */
int main(int argc, char **argv) {
    int test_size = 100;
    int *test_array = (int*)malloc(test_size * sizeof(int));
    
    if (!test_array) return 1;
    
    /* Initialize with random data */
    srand(time(NULL));
    for (int i = 0; i < test_size; i++) {
        test_array[i] = rand() % 200;
    }
    
    /* Call all test functions to ensure they're compiled */
    test_inner_loop(test_array, test_size);
    
    int result1 = test_nested_loops(50);
    
    test_complex_cf(argc > 1 ? atoi(argv[1]) : 0, 100);
    
    double result2 = test_mixed_ops(test_size);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %f\n", result1, result2);
    
    free(test_array);
    return 0;
}
