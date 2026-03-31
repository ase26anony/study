/* test_ddg_coverage.c
 * Designed to trigger DDG edge creation in GCC's scheduler
 * Compile with: gcc -O2 -fmodulo-sched -funroll-loops -c test_ddg_coverage.c
 * Or: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimizations */
volatile int g_volatile = 0;
int g_global_array[1024];
int g_result = 0;

/* Function prototypes */
int test_raw_dep(int *arr, int *brr, int n);
int test_war_waw_dep(int *arr, int *brr, int n);
int test_memory_aliasing(int *arr, int *brr, int *crr, int n);
int test_control_dep(int *arr, int *brr, int n);
int test_mixed_deps(int *arr, int *brr, float *farr, int n);
int test_nested_loop_deps(int *arr, int n);

/* Test 1: True Data Dependencies (RAW/flow dependencies) */
int test_raw_dep(int *arr, int *brr, int n) {
    int sum = 0;
    
    /* Loop with multiple RAW dependencies */
    for (int i = 2; i < n; i++) {
        /* RAW dependency chain: arr[i] depends on arr[i-1] and arr[i-2] */
        arr[i] = arr[i-1] + arr[i-2] + brr[i];
        
        /* Another RAW chain with different distance */
        brr[i] = brr[i-1] * 2 + g_volatile;
        
        /* Cross-iteration dependency with distance > 0 */
        sum += arr[i] + brr[i];
    }
    
    /* Prevent dead code elimination */
    g_global_array[0] = sum;
    return sum;
}

/* Test 2: Anti and Output Dependencies (WAR/WAW) */
int test_war_waw_dep(int *arr, int *brr, int n) {
    int temp = 0;
    int x = g_volatile;
    
    for (int i = 1; i < n; i++) {
        /* WAR (anti-dependency): brr[i] reads arr[i] before it's written */
        temp = arr[i] + x;
        
        /* WAW (output-dependency): arr[i] written twice */
        arr[i] = temp * 2;
        arr[i] = arr[i] + brr[i-1];  /* Overwrites previous value */
        
        /* Another WAR: using arr[i] after it was written */
        brr[i] = arr[i] + temp;
        
        /* Complex WAW with memory */
        g_global_array[i % 1024] = i;
        g_global_array[i % 1024] = brr[i];  /* Overwrites previous store */
    }
    
    return temp + brr[n-1];
}

/* Test 3: Memory Aliasing Dependencies */
int test_memory_aliasing(int *arr, int *brr, int *crr, int n) {
    int *p = arr;
    int *q = brr;
    int sum = 0;
    
    /* Force potential aliasing */
    if (g_volatile % 2) {
        q = arr + 10;  /* q might alias with p */
    }
    
    for (int i = 0; i < n; i++) {
        /* Memory operations that may alias */
        *p = i * 2 + g_volatile;
        *q = *p + crr[i];  /* Could be RAW if p and q alias */
        
        /* Pointer arithmetic to create complex dependencies */
        p = &arr[(i + 1) % n];
        q = &brr[(i + g_volatile) % n];
        
        /* Array accesses with non-linear indices */
        arr[(i * 7) % n] = brr[(i * 3) % n] + 1;
        
        sum += *p + *q;
    }
    
    return sum;
}

/* Test 4: Control Dependencies */
int test_control_dep(int *arr, int *brr, int n) {
    int sum = 0;
    int threshold = g_volatile;
    
    for (int i = 0; i < n; i++) {
        /* Control-dependent computations */
        if (arr[i] > threshold) {
            brr[i] = arr[i] * 2;
            sum += brr[i];
            
            /* Nested control flow */
            if (i % 3 == 0) {
                arr[i] = brr[i] / 2;
            } else {
                arr[i] = brr[i] + g_volatile;
            }
        } else {
            brr[i] = arr[i] / 2;
            sum -= brr[i];
            
            /* Different computation path */
            if (i % 5 == 0) {
                arr[i] = brr[i] * 3;
            }
        }
        
        /* Loop-carried dependency across control paths */
        threshold = (threshold + brr[i]) % 100;
    }
    
    return sum;
}

/* Test 5: Mixed Dependencies with Different Data Types */
int test_mixed_deps(int *arr, int *brr, float *farr, int n) {
    float fsum = 0.0f;
    int isum = 0;
    
    for (int i = 1; i < n; i++) {
        /* Integer RAW */
        arr[i] = arr[i-1] + brr[i];
        
        /* Floating point RAW */
        farr[i] = farr[i-1] * 1.5f + (float)arr[i];
        
        /* WAR between integer and float */
        isum += (int)farr[i];
        farr[i] = (float)isum * 0.5f;
        
        /* WAW on float array */
        farr[i] = farr[i] + (float)g_volatile;
        
        /* Mixed-type dependencies */
        brr[i] = (int)farr[i] + arr[i-1];
        
        fsum += farr[i];
    }
    
    return isum + (int)fsum;
}

/* Test 6: Nested Loops with Complex Dependencies */
int test_nested_loop_deps(int *arr, int n) {
    int sum = 0;
    
    /* Outer loop with carried dependency */
    for (int i = 1; i < n; i++) {
        int inner_sum = 0;
        
        /* Inner loop with multiple dependency types */
        for (int j = 1; j < 10; j++) {
            /* RAW in inner loop */
            inner_sum += arr[(i * 10 + j) % n];
            
            /* WAR in inner loop */
            arr[(i * 10 + j) % n] = inner_sum + j;
            
            /* Control in inner loop */
            if (inner_sum % 2) {
                arr[(i * 10 + j) % n] *= 2;
            }
        }
        
        /* Outer loop carried dependency */
        sum = sum * 3 + inner_sum;
        
        /* Output dependency across outer iterations */
        g_global_array[i % 1024] = sum;
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
    
    /* Allocate arrays with different alignments */
    int *arr1 = (int*)aligned_alloc(64, n * sizeof(int));
    int *arr2 = (int*)aligned_alloc(64, n * sizeof(int));
    int *arr3 = (int*)aligned_alloc(64, n * sizeof(int));
    float *farr = (float*)aligned_alloc(64, n * sizeof(float));
    
    /* Initialize with non-constant values */
    for (int i = 0; i < n; i++) {
        arr1[i] = i + argc;
        arr2[i] = i * 2 + argc;
        arr3[i] = i * 3 + argc;
        farr[i] = (float)i * 0.5f + argc;
    }
    
    int total = 0;
    
    /* Run all tests to trigger different DDG edge creations */
    total += test_raw_dep(arr1, arr2, n);
    total += test_war_waw_dep(arr2, arr3, n);
    total += test_memory_aliasing(arr1, arr2, arr3, n);
    total += test_control_dep(arr1, arr3, n);
    total += test_mixed_deps(arr1, arr2, farr, n);
    total += test_nested_loop_deps(arr1, n);
    
    /* Use volatile to prevent optimization of final result */
    g_volatile = total % 1000;
    
    printf("Result: %d\n", total);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(farr);
    
    return total != 0 ? 0 : 1;
}
