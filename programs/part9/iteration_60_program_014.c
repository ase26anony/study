/* Test program to trigger selective scheduler debug dumps in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Inner loop with conditional branch and memory write */
void test_inner_loop(int *arr, int n) {
    volatile int sum = 0;
    int i, j;
    
    /* Complex loop with data dependencies */
    for (i = 0; i < n; i++) {
        int temp = arr[i];
        /* Create scheduling barriers with inline asm */
        asm volatile ("" : "+r" (temp) : : "memory");
        
        for (j = 0; j < 10; j++) {
            /* Conditional store with dependency */
            if (temp > 100) {
                arr[i] = temp - j;
                /* Memory barrier to prevent reordering */
                asm volatile ("" : : : "memory");
            } else {
                arr[i] = temp + j;
            }
            sum += arr[i];
        }
        
        /* Another scheduling barrier */
        asm volatile ("" : : "r" (sum) : "memory");
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r" (sum) : "memory");
}

/* Function 2: Nested loops with different iteration counts */
int test_nested_loops(int *matrix, int rows, int cols) {
    volatile int total = 0;
    int i, j, k;
    
    /* Outer loop with pipelining opportunities */
    for (i = 0; i < rows; i++) {
        int row_sum = 0;
        
        /* Middle loop */
        for (j = 0; j < cols; j++) {
            int elem = matrix[i * cols + j];
            
            /* Inner loop with computation */
            for (k = 0; k < 5; k++) {
                elem = (elem * 3 + 7) % 256;
                /* Create artificial dependency chain */
                asm volatile ("" : "+r" (elem) : : );
            }
            
            row_sum += elem;
            matrix[i * cols + j] = elem;
        }
        
        total += row_sum;
        
        /* Conditional branch with side effect */
        if (row_sum > 1000) {
            /* Function call to create scheduling boundary */
            srand(row_sum);
            total -= rand() % 100;
        }
    }
    
    return total;
}

/* Function 3: Complex control flow with switch and computed goto */
void test_complex_cf(int mode, int *data, int size) {
    static void *labels[] = { &&case0, &&case1, &&case2, &&case3 };
    volatile int result = 0;
    int i;
    
    /* Computed goto for complex control flow */
    goto *labels[mode % 4];
    
case0:
    for (i = 0; i < size; i += 2) {
        data[i] = data[i] * 2 + 1;
        /* Memory access pattern */
        asm volatile ("" : : "r" (data[i]) : "memory");
    }
    result = 1;
    goto end;
    
case1:
    for (i = 1; i < size; i += 2) {
        data[i] = data[i] / 2 - 1;
        /* Scheduling barrier */
        asm volatile ("" : : : "memory");
    }
    result = 2;
    goto end;
    
case2:
    /* Mixed pattern */
    for (i = 0; i < size; i++) {
        if (i % 3 == 0) {
            data[i] = -data[i];
        } else {
            data[i] = data[i] + i;
        }
    }
    result = 3;
    goto end;
    
case3:
    /* Nested conditionals */
    for (i = 0; i < size; i++) {
        switch (data[i] % 4) {
            case 0: data[i] += 10; break;
            case 1: data[i] -= 5; break;
            case 2: data[i] *= 2; break;
            case 3: data[i] /= 2; break;
        }
    }
    result = 4;
    
end:
    /* Ensure result is used */
    asm volatile ("" : : "r" (result) : "memory");
}

/* Function 4: Mixed operations with pointer aliasing */
int test_mixed_ops(int *a, int *b, int *c, int n) {
    volatile int acc = 0;
    int i;
    
    /* Loop with potential pointer aliasing */
    for (i = 0; i < n; i++) {
        /* Multiple memory operations */
        int x = a[i];
        int y = b[i];
        
        /* Complex arithmetic with dependencies */
        int t1 = x * y + i;
        int t2 = (x << 3) | (y & 0xFF);
        int t3 = t1 ^ t2;
        
        /* Conditional store with side effect */
        if (t3 > 0) {
            c[i] = t3;
            acc += t3;
        } else {
            c[i] = -t3;
            acc -= t3;
        }
        
        /* Dependency chain */
        asm volatile ("" : "+r" (acc) : : "memory");
    }
    
    return acc;
}

/* Main driver function */
int main() {
    const int SIZE = 100;
    const int ROWS = 20;
    const int COLS = 5;
    
    /* Allocate and initialize test data */
    int *arr1 = (int*)malloc(SIZE * sizeof(int));
    int *arr2 = (int*)malloc(SIZE * sizeof(int));
    int *arr3 = (int*)malloc(SIZE * sizeof(int));
    int *matrix = (int*)malloc(ROWS * COLS * sizeof(int));
    
    /* Initialize with random data */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        arr3[i] = rand() % 1000;
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        matrix[i] = rand() % 500;
    }
    
    /* Call all test functions to ensure compilation */
    test_inner_loop(arr1, SIZE);
    
    int sum1 = test_nested_loops(matrix, ROWS, COLS);
    
    test_complex_cf(0, arr2, SIZE);
    test_complex_cf(1, arr2, SIZE);
    test_complex_cf(2, arr2, SIZE);
    test_complex_cf(3, arr2, SIZE);
    
    int sum2 = test_mixed_ops(arr1, arr2, arr3, SIZE);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d %d\n", sum1, sum2);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(matrix);
    
    return 0;
}
