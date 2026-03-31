/* test_ddg_coverage.c
 * Program designed to trigger DDG edge creation in GCC's instruction scheduler
 * Compile with: gcc -O2 -fmodulo-sched -funroll-loops -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimizations */
volatile int g_volatile = 0;
int g_global_array[1024];
int g_result = 0;

/* Function prototypes */
int test_raw_dependencies(int *arr, int n);
int test_war_waw_dependencies(int *arr1, int *arr2, int n);
int test_memory_aliasing(int *arr, int n, int *ptr1, int *ptr2);
int test_control_dependencies(int *arr, int n);
int test_mixed_dependencies(int *arr1, int *arr2, int n);
int test_loop_carried_dependencies(int *arr, int n);

/* Test 1: True Data Dependencies (RAW) with loop-carried dependencies */
int test_raw_dependencies(int *arr, int n) {
    int sum = 0;
    /* Multiple RAW dependencies with different distances */
    for (int i = 2; i < n; i++) {
        /* Distance 1 RAW dependency */
        arr[i] = arr[i-1] + g_volatile;
        
        /* Distance 2 RAW dependency */
        arr[i] += arr[i-2] * 2;
        
        /* Floating point RAW dependency */
        float temp = (float)arr[i] / 3.14f;
        arr[i] = (int)temp + i;
        
        sum += arr[i];
    }
    return sum;
}

/* Test 2: Anti (WAR) and Output (WAW) Dependencies */
int test_war_waw_dependencies(int *arr1, int *arr2, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* WAR (anti-dependency): read arr1[i], then write to it */
        int temp = arr1[i] + arr2[i];
        arr1[i] = temp * 2;  /* WAR: arr1[i] read above, written here */
        
        /* WAW (output-dependency): multiple writes to same location */
        arr2[i] = temp + 1;
        arr2[i] = arr2[i] * 3;  /* WAW: arr2[i] written twice */
        
        /* Another WAR with different data types */
        float f_temp = (float)arr1[i];
        arr1[i] = (int)(f_temp * 1.5f);  /* WAR on arr1[i] */
        
        sum += arr1[i] + arr2[i];
    }
    return sum;
}

/* Test 3: Memory Aliasing Dependencies */
int test_memory_aliasing(int *arr, int n, int *ptr1, int *ptr2) {
    int sum = 0;
    
    /* Force potential aliasing */
    ptr1 = &arr[n/3];
    ptr2 = &arr[n/2];
    
    for (int i = 1; i < n-1; i++) {
        /* Memory operations with potential aliasing */
        *ptr1 = arr[i] + arr[i-1];
        *ptr2 = *ptr1 * 2;  /* May alias with ptr1 */
        
        /* More complex aliasing pattern */
        arr[i] = *ptr1 + *ptr2;
        
        /* Pointer arithmetic creating ambiguous dependencies */
        int *p = &arr[i % 10];
        *p = arr[i] + i;
        
        sum += arr[i] + *ptr1 + *ptr2;
    }
    return sum;
}

/* Test 4: Control Dependencies */
int test_control_dependencies(int *arr, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Control-dependent computations */
        if (arr[i] > 0) {
            arr[i] = arr[i] * 2 + g_volatile;
        } else {
            arr[i] = arr[i] / 2 - g_volatile;
        }
        
        /* Nested control flow */
        if (i % 3 == 0) {
            arr[i] += 1;
            if (i % 5 == 0) {
                arr[i] *= 2;
            }
        }
        
        /* Switch-like control dependency */
        switch (arr[i] % 4) {
            case 0: arr[i] += 10; break;
            case 1: arr[i] -= 5; break;
            case 2: arr[i] *= 3; break;
            default: arr[i] /= 2; break;
        }
        
        sum += arr[i];
    }
    return sum;
}

/* Test 5: Mixed Dependencies with Function Calls */
int test_mixed_dependencies(int *arr1, int *arr2, int n) {
    int sum = 0;
    
    for (int i = 1; i < n; i++) {
        /* RAW with different data types */
        double d_val = (double)arr1[i-1] * 1.234;
        arr1[i] = (int)d_val + i;
        
        /* WAR across different arrays */
        int temp = arr2[i];
        arr2[i] = arr1[i] * 2;  /* WAR on arr2[i] */
        arr1[i] = temp + arr2[i];  /* RAW on arr2[i], WAR on arr1[i] */
        
        /* WAW with volatile */
        arr1[i] = g_volatile;
        arr1[i] = arr1[i] + 1;  /* WAW on arr1[i] */
        
        /* Memory barrier effect */
        __asm__ volatile ("" : : : "memory");
        
        sum += arr1[i] + arr2[i];
    }
    return sum;
}

/* Test 6: Complex Loop-Carried Dependencies */
int test_loop_carried_dependencies(int *arr, int n) {
    int sum = 0;
    
    /* Multiple loop-carried dependencies with different distances */
    for (int i = 4; i < n; i++) {
        /* Distance 1, 2, and 3 dependencies */
        arr[i] = arr[i-1] + arr[i-2] + arr[i-3] + arr[i-4];
        
        /* Interleaved dependencies */
        int t1 = arr[i-1] * 2;
        int t2 = arr[i-2] + t1;
        arr[i] = arr[i-3] + t2;
        
        /* Recurrence with computation */
        arr[i] = (arr[i] * 3 - arr[i-1]) / 2;
        
        /* Floating-point recurrence */
        float f1 = (float)arr[i-1];
        float f2 = (float)arr[i-2];
        arr[i] = (int)(f1 * f2 / 3.14f);
        
        sum += arr[i];
    }
    return sum;
}

/* Main function that runs all tests */
int main(int argc, char **argv) {
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 100) n = 100;
        if (n > 10000) n = 10000;
    }
    
    /* Initialize arrays with non-uniform data */
    int *arr1 = (int*)malloc(n * sizeof(int));
    int *arr2 = (int*)malloc(n * sizeof(int));
    
    for (int i = 0; i < n; i++) {
        arr1[i] = (i * 3) % 97;
        arr2[i] = (i * 7) % 113;
    }
    
    int total_sum = 0;
    
    /* Run all tests to trigger different DDG edge types */
    total_sum += test_raw_dependencies(arr1, n);
    total_sum += test_war_waw_dependencies(arr1, arr2, n);
    total_sum += test_memory_aliasing(arr1, n, &arr1[n/4], &arr1[n/3]);
    total_sum += test_control_dependencies(arr2, n);
    total_sum += test_mixed_dependencies(arr1, arr2, n);
    total_sum += test_loop_carried_dependencies(arr1, n);
    
    /* Store result to prevent dead code elimination */
    g_result = total_sum;
    
    printf("Result: %d\n", total_sum % 1000);
    
    free(arr1);
    free(arr2);
    
    return 0;
}
