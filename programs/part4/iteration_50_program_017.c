/* test_ddg_edges.c - Program to trigger DDG edge creation in GCC scheduler */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_global_array[1024];
int g_result = 0;

/* Function prototypes */
int test_raw_dependencies(int *arr, int n);
int test_war_waw_dependencies(int *arr, int n);
int test_memory_aliasing(int *arr1, int *arr2, int n);
int test_control_dependencies(int *arr, int n);
int test_mixed_dependencies(float *farr, int *iarr, int n);
int test_nested_loop_dependencies(int *arr, int n, int m);
int test_loop_carried_dependencies(int *arr, int n);

/* Helper to prevent dead code elimination */
static int use_result(int x) {
    volatile int sink = x;
    return sink;
}

/* 1. Test RAW (true data) dependencies */
int test_raw_dependencies(int *arr, int n) {
    int sum = 0;
    /* Loop with flow dependencies across iterations */
    for (int i = 2; i < n; i++) {
        /* RAW: arr[i-1] read before arr[i] write in next iteration */
        arr[i] = arr[i-1] + arr[i-2] + g_volatile;
        sum += arr[i];
    }
    
    /* Additional RAW within same iteration */
    for (int i = 1; i < n; i++) {
        int temp = arr[i-1];
        arr[i] = temp * 2 + i;
        sum += arr[i];
    }
    
    return use_result(sum);
}

/* 2. Test WAR and WAW dependencies */
int test_war_waw_dependencies(int *arr, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        int temp1 = arr[i];  /* Read arr[i] */
        
        /* WAR: arr[i] will be written later, but temp1 uses old value */
        arr[i] = temp1 * 3 + i;  /* Write arr[i] - anti-dependency with above */
        
        /* WAW: Multiple writes to same location */
        if (i % 3 == 0) {
            arr[i] = arr[i] + 1;  /* Another write to arr[i] */
        }
        
        sum += arr[i];
    }
    
    /* Complex WAW pattern */
    for (int i = 0; i < n; i++) {
        arr[i] = i * 2;
        arr[i] = arr[i] + (i % 5);  /* Second write */
        arr[i] = arr[i] * arr[i];   /* Third write */
        sum += arr[i];
    }
    
    return use_result(sum);
}

/* 3. Test memory aliasing dependencies */
int test_memory_aliasing(int *arr1, int *arr2, int n) {
    int sum = 0;
    
    /* arr1 and arr2 may alias - compiler doesn't know */
    int *ptr1 = arr1;
    int *ptr2 = arr2;
    
    for (int i = 1; i < n; i++) {
        /* Potential aliasing between ptr1[i] and ptr2[i-1] */
        ptr1[i] = ptr2[i-1] + g_volatile;
        ptr2[i] = ptr1[i] * 2;
        
        /* More complex aliasing pattern */
        if (i % 4 == 0) {
            ptr1[i/2] = ptr2[i] + 1;
        }
        
        sum += ptr1[i] + ptr2[i];
    }
    
    /* Pointer arithmetic that could cause aliasing */
    for (int i = 0; i < n-1; i++) {
        *(ptr1 + i) = *(ptr2 + i + 1) + 7;
        sum += *(ptr1 + i);
    }
    
    return use_result(sum);
}

/* 4. Test control dependencies */
int test_control_dependencies(int *arr, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        int val = arr[i];
        
        /* Branch creates control dependencies */
        if (val % 2 == 0) {
            arr[i] = val * 3 + 1;
        } else {
            arr[i] = val * 2 - 1;
        }
        
        /* Nested control flow */
        if (arr[i] > 100) {
            arr[i] = arr[i] % 100;
            if (arr[i] < 50) {
                arr[i] = arr[i] + 50;
            }
        }
        
        sum += arr[i];
    }
    
    /* Loop with multiple exit conditions */
    int j = 0;
    while (j < n) {
        if (arr[j] > 1000) break;
        if (arr[j] < 0) continue;
        
        arr[j] = arr[j] + j;
        sum += arr[j];
        j++;
    }
    
    return use_result(sum);
}

/* 5. Test mixed data types (int and float) */
int test_mixed_dependencies(float *farr, int *iarr, int n) {
    float fsum = 0.0f;
    int isum = 0;
    
    for (int i = 1; i < n; i++) {
        /* Mixed type dependencies */
        farr[i] = farr[i-1] * 1.5f + (float)iarr[i];
        iarr[i] = (int)farr[i] + iarr[i-1];
        
        /* More mixed operations */
        if (i % 3 == 0) {
            farr[i] = farr[i] / 2.0f;
            iarr[i] = iarr[i] * 2;
        }
        
        fsum += farr[i];
        isum += iarr[i];
    }
    
    return use_result((int)fsum + isum);
}

/* 6. Test nested loops with dependencies */
int test_nested_loop_dependencies(int *arr, int n, int m) {
    int sum = 0;
    
    /* Outer loop with carried dependency */
    for (int i = 1; i < n; i++) {
        /* Inner loop with its own dependencies */
        for (int j = 1; j < m; j++) {
            /* 2D access pattern with dependencies */
            int idx = i * m + j;
            int prev_idx = (i-1) * m + j;
            int left_idx = i * m + (j-1);
            
            /* Multiple dependency types */
            arr[idx] = arr[prev_idx] + arr[left_idx] + g_volatile;
            
            /* Anti-dependency within inner loop */
            int temp = arr[idx];
            arr[idx] = temp * (i + j);
            
            sum += arr[idx];
        }
    }
    
    return use_result(sum);
}

/* 7. Test loop-carried dependencies with distance > 1 */
int test_loop_carried_dependencies(int *arr, int n) {
    int sum = 0;
    
    /* Distance 2 dependency */
    for (int i = 2; i < n; i++) {
        arr[i] = arr[i-2] * 3 - arr[i-1] + i;
        sum += arr[i];
    }
    
    /* Distance 3 dependency with stride */
    for (int i = 3; i < n; i++) {
        arr[i] = arr[i-3] + arr[i-2] + arr[i-1];
        sum += arr[i];
    }
    
    /* Variable distance dependency */
    for (int i = 1; i < n; i++) {
        int distance = (i % 4) + 1;
        if (i >= distance) {
            arr[i] = arr[i-distance] + g_volatile;
        }
        sum += arr[i];
    }
    
    return use_result(sum);
}

/* Main function that runs all tests */
int main(int argc, char **argv) {
    int n = 1000;
    int m = 100;
    
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 100) n = 100;
        if (n > 10000) n = 10000;
    }
    
    /* Allocate arrays with different alignments */
    int *arr1 = (int*)aligned_alloc(64, n * sizeof(int));
    int *arr2 = (int*)aligned_alloc(64, n * sizeof(int));
    float *farr = (float*)aligned_alloc(64, n * sizeof(float));
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        arr1[i] = i * 3 % 97;
        arr2[i] = i * 7 % 101;
        farr[i] = (float)i * 0.5f;
        g_global_array[i % 1024] = i;
    }
    
    int total = 0;
    
    /* Run all tests to create various DDG edges */
    total += test_raw_dependencies(arr1, n);
    total += test_war_waw_dependencies(arr2, n);
    total += test_memory_aliasing(arr1, arr2, n);
    total += test_control_dependencies(arr1, n);
    total += test_mixed_dependencies(farr, arr1, n);
    total += test_nested_loop_dependencies(arr1, n/10, m);
    total += test_loop_carried_dependencies(arr2, n);
    
    /* Use volatile to prevent optimization of final result */
    volatile int final_result = total % 1000000;
    
    printf("Result: %d\n", final_result);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr);
    
    return final_result != 0;
}
