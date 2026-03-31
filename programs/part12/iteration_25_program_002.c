/* test_ddg_coverage.c
 * Compile with: gcc -O3 -ftree-vectorize -funroll-loops -fmodulo-sched -fdump-tree-dd -fdump-tree-sms test_ddg_coverage.c -o test_ddg_coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 100000

/* Prevent optimization and create dependencies */
volatile int global_seed = 42;

/* Noinline functions to prevent optimization */
__attribute__((noinline)) int use_value(int val) {
    return val ^ global_seed;
}

__attribute__((noinline)) void modify_value(int* ptr) {
    *ptr += global_seed;
}

/* Test 1: Flow (RAW) dependency with carried dependency across iterations */
__attribute__((noinline)) int test_flow_dependency(int* arr) {
    int sum = 0;
    /* Classic accumulation with flow dependency */
    for (int i = 0; i < SIZE; i++) {
        sum += arr[i];  /* Flow dependency: sum from previous iteration */
        arr[i] = sum;   /* Store the cumulative sum */
    }
    
    /* Another pattern: recurrence with distance > 0 */
    int prev = arr[0];
    for (int i = 1; i < SIZE; i++) {
        int curr = arr[i] + prev;  /* Flow: depends on prev from previous iteration */
        arr[i] = curr;
        prev = curr;
    }
    
    return sum;
}

/* Test 2: Anti (WAR) dependency */
__attribute__((noinline)) int test_anti_dependency(int* arr, int* brr) {
    int temp = 0;
    for (int i = 0; i < SIZE; i++) {
        temp = arr[i];      /* Read arr[i] */
        arr[i] = brr[i];    /* Overwrite arr[i] - anti-dependency with previous read */
        brr[i] = temp;      /* Use the read value */
    }
    
    /* More complex anti-dependency with computation */
    for (int i = 0; i < SIZE - 1; i++) {
        int read_val = arr[i] * 2;      /* Read arr[i] */
        arr[i] = brr[i] + read_val;     /* Overwrite arr[i] */
        brr[i] = read_val - arr[i+1];   /* Use read_val and read arr[i+1] */
    }
    
    return temp;
}

/* Test 3: Output (WAW) dependency */
__attribute__((noinline)) int test_output_dependency(int* arr) {
    int result = 0;
    volatile int* volatile_ptr = arr;  /* Volatile pointer to prevent optimization */
    
    for (int i = 0; i < SIZE; i++) {
        /* Multiple writes to same location */
        arr[i] = i * 2;                /* First write */
        arr[i] = arr[i] + global_seed; /* Second write - output dependency */
        volatile_ptr[i] = arr[i] * 3;  /* Third write through volatile */
        
        /* Conditional output dependency */
        if (i % 2 == 0) {
            arr[i] = arr[i] + 1;
        } else {
            arr[i] = arr[i] - 1;
        }
    }
    
    /* Nested output dependencies */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < 4; j++) {
            arr[i] = arr[i] + j;  /* Multiple writes in inner loop */
        }
    }
    
    return result;
}

/* Test 4: Nested loops with cross-iteration dependencies */
__attribute__((noinline)) int test_nested_dependency(int matrix[SIZE][SIZE]) {
    int sum = 0;
    
    /* Outer loop carried dependency */
    for (int i = 1; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            /* Flow dependency on previous row */
            matrix[i][j] = matrix[i-1][j] + matrix[i][j];
            sum += matrix[i][j];
        }
    }
    
    /* Anti-dependency in nested loops */
    for (int i = 0; i < SIZE - 1; i++) {
        int row_sum = 0;
        for (int j = 0; j < SIZE; j++) {
            row_sum += matrix[i][j];      /* Read */
            matrix[i][j] = matrix[i+1][j]; /* Overwrite - anti-dependency */
        }
        sum += row_sum;
    }
    
    return sum;
}

/* Test 5: Mixed data types and operations */
__attribute__((noinline)) float test_mixed_types(float* farr, double* darr, int* iarr) {
    float fsum = 0.0f;
    double dsum = 0.0;
    
    /* Interleaved dependencies with different types */
    for (int i = 1; i < SIZE; i++) {
        /* Flow dependency with float */
        farr[i] = farr[i-1] * 1.5f + farr[i];
        fsum += farr[i];
        
        /* Anti-dependency with int */
        int ival = iarr[i];              /* Read */
        iarr[i] = (int)(darr[i] * 100);  /* Overwrite */
        darr[i] = ival * 0.01;           /* Use read value */
        
        /* Output dependency with double */
        darr[i] = dsum;                  /* First write */
        dsum += darr[i];                 /* Read */
        darr[i] = dsum * 2.0;            /* Second write */
    }
    
    return fsum;
}

/* Test 6: Control flow with dependencies */
__attribute__((noinline)) int test_control_flow_dependency(int* arr, int* brr) {
    int sum = 0;
    
    for (int i = 1; i < SIZE; i++) {
        /* Conditional flow dependency */
        if (arr[i] > 0) {
            sum += arr[i];          /* Flow dependency on sum */
            arr[i] = sum;           /* Output dependency on arr[i] */
        } else {
            int temp = brr[i];      /* Read brr[i] */
            brr[i] = arr[i];        /* Overwrite brr[i] - anti-dependency */
            arr[i] = temp;          /* Use read value */
            sum -= temp;
        }
        
        /* Switch-like dependency pattern */
        switch (i % 3) {
            case 0:
                arr[i] = arr[i-1] + 1;  /* Flow dependency */
                break;
            case 1:
                brr[i] = arr[i];        /* Read arr[i] */
                arr[i] = i;             /* Overwrite arr[i] - anti-dependency */
                break;
            case 2:
                arr[i] = arr[i] * 2;    /* Read-modify-write - output dependency */
                break;
        }
    }
    
    return sum;
}

/* Test 7: Pointer aliasing creates ambiguous dependencies */
__attribute__((noinline)) int test_pointer_aliasing(int* arr, int* brr) {
    int* ptr1 = arr;
    int* ptr2 = brr;
    int* ptr3 = &arr[SIZE/2];
    
    int sum = 0;
    for (int i = 0; i < SIZE/2; i++) {
        /* Potential aliasing creates conservative dependencies */
        ptr1[i] = ptr2[i] + sum;    /* Flow on sum, read ptr2 */
        sum = ptr1[i];              /* Flow dependency */
        
        /* ptr3 may alias with ptr1 */
        ptr3[i] = ptr1[i] * 2;      /* Possible anti/output dependencies */
        ptr2[i] = ptr3[i] - i;      /* Flow on ptr3 */
    }
    
    return sum;
}

int main() {
    /* Allocate and initialize data */
    int* arr = (int*)malloc(SIZE * sizeof(int));
    int* brr = (int*)malloc(SIZE * sizeof(int));
    float* farr = (float*)malloc(SIZE * sizeof(float));
    double* darr = (double*)malloc(SIZE * sizeof(double));
    int (*matrix)[SIZE] = (int(*)[SIZE])malloc(SIZE * SIZE * sizeof(int));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = (i * 13 + 7) % 100;
        brr[i] = (i * 17 + 11) % 100;
        farr[i] = (float)(i * 0.5f);
        darr[i] = (double)(i * 0.25);
    }
    
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            matrix[i][j] = (i * SIZE + j) % 100;
        }
    }
    
    int total = 0;
    
    /* Run tests multiple times to ensure execution */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        total += test_flow_dependency(arr);
        total += test_anti_dependency(arr, brr);
        total += test_output_dependency(arr);
        total += test_nested_dependency(matrix);
        
        float ftemp = test_mixed_types(farr, darr, arr);
        total += (int)ftemp;
        
        total += test_control_flow_dependency(arr, brr);
        total += test_pointer_aliasing(arr, brr);
        
        /* Modify global seed to change patterns */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    /* Cleanup */
    free(arr);
    free(brr);
    free(farr);
    free(darr);
    free(matrix);
    
    return 0;
}
