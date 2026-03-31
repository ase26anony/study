/* Test program to trigger selective scheduler debug dumps in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function with inner loop and conditional branch */
void test_inner_loop(int *arr, int n) {
    volatile int sum = 0;  /* volatile prevents optimization */
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Create data dependency */
        int temp = arr[i];
        
        /* Conditional store with side effect */
        if (temp > 100) {
            arr[i] = temp * 2;
            sum += arr[i];
        } else {
            arr[i] = temp / 2;
            sum -= arr[i];
        }
        
        /* Inline assembly to create unschedulable dependency */
        asm volatile ("" : : "r"(temp) : "memory");
    }
    
    /* Use the result to prevent dead code elimination */
    asm volatile ("" : : "r"(sum));
}

/* Function with nested loops for outer loop pipelining */
void test_nested_loops(int *matrix, int rows, int cols) {
    volatile int total = 0;
    int i, j, k;
    
    for (i = 0; i < rows; i++) {
        int row_sum = 0;
        for (j = 0; j < cols; j++) {
            /* Complex addressing calculation */
            int idx = i * cols + j;
            int val = matrix[idx];
            
            /* Multiple operations with dependencies */
            val = val * 3 + 7;
            if (val % 2 == 0) {
                val = val >> 1;
            } else {
                val = val * 2 + 1;
            }
            
            matrix[idx] = val;
            row_sum += val;
            
            /* Another memory barrier */
            asm volatile ("" : : "r"(val) : "memory");
        }
        
        /* Conditional update based on row_sum */
        if (row_sum > 1000) {
            total += row_sum;
        } else {
            total -= row_sum;
        }
    }
    
    asm volatile ("" : : "r"(total));
}

/* Function with switch statement and computed goto */
void test_switch_complex(int x) {
    static void *labels[] = { &&label0, &&label1, &&label2, &&label3 };
    volatile int result = 0;
    
    /* Computed goto creates complex control flow */
    goto *labels[x % 4];
    
label0:
    result = x * 2;
    asm volatile ("" : : "r"(result));
    goto end;
    
label1:
    result = x + 100;
    asm volatile ("" : : "r"(result));
    goto end;
    
label2:
    result = x - 50;
    asm volatile ("" : : "r"(result));
    goto end;
    
label3:
    result = x / 3;
    asm volatile ("" : : "r"(result));
    goto end;
    
end:
    /* Use multiple basic blocks */
    if (result > 0) {
        asm volatile ("nop" : : : "memory");
    } else {
        asm volatile ("nop" : : : "memory");
    }
}

/* Function with mixed operations and function calls */
int test_mixed_operations(int a, int b, int c) {
    volatile int x = a;
    volatile int y = b;
    volatile int z = c;
    int i;
    
    /* Loop with function call simulation */
    for (i = 0; i < 10; i++) {
        /* Complex expression with multiple dependencies */
        x = (x * y) + z;
        y = (y - x) ^ z;
        z = (z + x) | y;
        
        /* Memory operation */
        int *ptr = &x;
        *ptr = *ptr + 1;
        
        /* Conditional with side effect */
        if (z > 1000) {
            x = x >> 2;
            asm volatile ("" : : "r"(x));
        } else if (z < 0) {
            x = x << 1;
            asm volatile ("" : : "r"(x));
        } else {
            x = x ^ 0xFF;
            asm volatile ("" : : "r"(x));
        }
    }
    
    return x + y + z;
}

/* Main driver function */
int main() {
    const int SIZE = 100;
    const int ROWS = 10;
    const int COLS = 10;
    
    /* Allocate and initialize test data */
    int *array = (int *)malloc(SIZE * sizeof(int));
    int *matrix = (int *)malloc(ROWS * COLS * sizeof(int));
    
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < SIZE; i++) {
        array[i] = rand() % 200;
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        matrix[i] = rand() % 100;
    }
    
    /* Call test functions to ensure they're compiled */
    test_inner_loop(array, SIZE);
    test_nested_loops(matrix, ROWS, COLS);
    test_switch_complex(rand() % 100);
    int result = test_mixed_operations(rand() % 50, rand() % 50, rand() % 50);
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", result);
    printf("Array[0]: %d\n", array[0]);
    printf("Matrix[0]: %d\n", matrix[0]);
    
    free(array);
    free(matrix);
    
    return 0;
}
