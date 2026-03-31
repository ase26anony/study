/* test_sel_sched.c - Program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function 1: Inner loop with conditional branch and memory write */
void test_inner_loop(int *arr, int n) {
    volatile int sum = 0;  /* Prevent optimization */
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Create data dependencies */
        int temp = arr[i];
        
        /* Conditional store with dependency chain */
        if (temp > 100) {
            arr[i] = temp * 2 + i;
            /* Inline asm to create unschedulable dependency */
            asm volatile ("" : : "r"(temp) : "memory");
        } else {
            arr[i] = temp / 2 - i;
        }
        
        /* Nested loop with memory access */
        for (j = 0; j < 5; j++) {
            sum += arr[i] + j;
            /* Memory barrier to prevent reordering */
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Use result to prevent dead code elimination */
    if (sum > 1000) {
        printf("Inner loop sum: %d\n", sum);
    }
}

/* Function 2: Nested loops with different iteration counts */
void test_nested_loops(int *matrix, int rows, int cols) {
    volatile int total = 0;
    int i, j, k;
    
    for (i = 0; i < rows; i++) {
        int row_start = i * cols;
        
        for (j = 0; j < cols; j++) {
            int idx = row_start + j;
            int val = matrix[idx];
            
            /* Complex dependency chain */
            for (k = 0; k < 3; k++) {
                val = (val * 13 + 7) % 256;
                /* Force spill/reload */
                asm volatile ("# Dependency %0" : : "r"(val));
            }
            
            matrix[idx] = val;
            total += val;
            
            /* Conditional with unpredictable branch */
            if (val & 1) {
                total -= matrix[(idx + 1) % (rows * cols)];
            }
        }
        
        /* Function call to create control flow */
        if (i % 2 == 0) {
            /* External function call */
            clock();
        }
    }
    
    /* Use result */
    if (total != 0) {
        printf("Matrix total: %d\n", total);
    }
}

/* Function 3: Switch statement with computed goto */
void test_switch_complex(int mode, int *data, int size) {
    static void *labels[] = { &&case0, &&case1, &&case2, &&case3 };
    volatile int result = 0;
    int i;
    
    if (mode < 0 || mode > 3) mode = 0;
    
    /* Computed goto - creates complex control flow */
    goto *labels[mode];
    
case0:
    for (i = 0; i < size; i++) {
        data[i] = data[i] + i;
        result += data[i];
        /* Memory access pattern */
        asm volatile ("" : "=m"(data[i]) : : "memory");
    }
    goto end;
    
case1:
    for (i = size - 1; i >= 0; i--) {
        data[i] = data[i] * 2 - 1;
        result ^= data[i];
        /* Dependency */
        asm volatile ("# Case1 %0" : : "r"(data[i]));
    }
    goto end;
    
case2:
    i = 0;
    while (i < size) {
        data[i] = (data[i] << 1) | (data[i] >> 31);
        result |= data[i];
        i += 2;
        /* Control dependency */
        if (result & 1) i--;
    }
    goto end;
    
case3:
    do {
        data[0] = data[0] * 3 + 7;
        result = data[0] % 100;
        /* Loop with side effect */
        asm volatile ("# Case3 loop" : : : "memory");
    } while (result > 50 && size-- > 0);
    goto end;
    
end:
    /* Prevent dead code elimination */
    if (result > 100) {
        printf("Switch result: %d\n", result);
    }
}

/* Function 4: Mixed operations with pointer aliasing */
void test_pointer_aliasing(int *a, int *b, int *c, int n) {
    volatile int acc = 0;
    int i;
    
    /* Potential pointer aliasing creates scheduling constraints */
    for (i = 0; i < n; i++) {
        a[i] = b[i] + c[i];
        acc += a[i];
        
        /* Create anti-dependencies */
        b[i] = a[i] - b[i];
        c[i] = b[i] * c[i];
        
        /* Force serialization */
        asm volatile ("# Alias chain %0 %1 %2" 
                     : : "r"(a[i]), "r"(b[i]), "r"(c[i]) : "memory");
    }
    
    /* Use all results */
    for (i = 0; i < n; i++) {
        acc += b[i] - c[i];
    }
    
    if (acc != 0) {
        printf("Aliasing acc: %d\n", acc);
    }
}

/* Function 5: Recursive-like pattern with tail duplication */
void test_tail_duplication(int *arr, int start, int end) {
    volatile int count = 0;
    
    while (start < end) {
        int mid = (start + end) / 2;
        
        /* Multiple basic blocks in loop */
        if (arr[start] < arr[mid]) {
            arr[start] = arr[start] + arr[mid];
            count++;
            /* Branch with side effect */
            asm volatile ("# Branch taken" : : : "memory");
        } else {
            arr[mid] = arr[mid] - arr[start];
            count--;
        }
        
        /* Another condition */
        if (count > 10) {
            end = mid;
            /* Complex operation */
            arr[start] = (arr[start] * 7) >> 3;
        } else if (count < -10) {
            start = mid + 1;
            arr[mid] = (arr[mid] * 3) << 2;
        } else {
            start++;
            end--;
        }
        
        /* Memory synchronization */
        asm volatile ("" : : : "memory");
    }
    
    if (count != 0) {
        printf("Tail dup count: %d\n", count);
    }
}

/* Main driver */
int main(void) {
    int i;
    
    /* Allocate and initialize test data */
    int size1 = 100;
    int *arr1 = (int*)malloc(size1 * sizeof(int));
    int *arr2 = (int*)malloc(size1 * sizeof(int));
    int *arr3 = (int*)malloc(size1 * sizeof(int));
    
    int rows = 10, cols = 10;
    int *matrix = (int*)malloc(rows * cols * sizeof(int));
    
    /* Initialize with pseudo-random values */
    srand(42);
    for (i = 0; i < size1; i++) {
        arr1[i] = rand() % 200;
        arr2[i] = rand() % 200;
        arr3[i] = rand() % 200;
    }
    
    for (i = 0; i < rows * cols; i++) {
        matrix[i] = rand() % 256;
    }
    
    /* Call all test functions to ensure they're compiled */
    test_inner_loop(arr1, size1);
    test_nested_loops(matrix, rows, cols);
    test_switch_complex(rand() % 4, arr2, size1);
    test_pointer_aliasing(arr1, arr2, arr3, size1);
    test_tail_duplication(arr3, 0, size1);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(matrix);
    
    return 0;
}
