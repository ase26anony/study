/* test_ddg_coverage.c
 * Designed to trigger DDG edge creation in GCC's instruction scheduler
 * Compile with: gcc -O2 -fmodulo-sched -funroll-loops -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_array[1024];
int g_result = 0;
int* g_ptr1;
int* g_ptr2;

/* Function to create true data dependencies (RAW) */
int test_raw_dep(int* arr, int n) {
    int sum = 0;
    /* Loop with flow dependencies across iterations */
    for (int i = 2; i < n; i++) {
        arr[i] = arr[i-1] + arr[i-2] + g_volatile;  /* Distance = 1 and 2 */
        sum += arr[i];
    }
    return sum;
}

/* Function to create anti and output dependencies (WAR/WAW) */
int test_war_waw_dep(int* arr, int n) {
    int temp = 0;
    int acc = 0;
    
    for (int i = 0; i < n; i++) {
        /* WAR: Read arr[i] before writing to temp */
        acc = arr[i] + temp;
        
        /* WAW: Multiple writes to temp */
        temp = acc * 2;
        temp = temp + i;  /* Output dependency on temp */
        
        /* WAR: Read temp before writing to arr[i] */
        arr[i] = temp + g_volatile;
    }
    return acc;
}

/* Function with memory aliasing dependencies */
int test_memory_aliasing(int* arr1, int* arr2, int n) {
    int sum = 0;
    g_ptr1 = arr1;
    g_ptr2 = arr2;
    
    /* Pointer aliasing creates ambiguous memory dependencies */
    for (int i = 1; i < n; i++) {
        *g_ptr1 = *g_ptr2 + i;
        *g_ptr2 = *g_ptr1 * 2;
        
        /* Potential aliasing between arrays */
        arr1[i] = arr2[i-1] + g_volatile;
        arr2[i] = arr1[i] * 3;
        
        sum += arr1[i] + arr2[i];
        
        /* Modify pointers to create complex access patterns */
        if (i % 16 == 0) {
            g_ptr1 = &arr1[i];
            g_ptr2 = &arr2[i];
        }
    }
    return sum;
}

/* Function with control dependencies */
int test_control_dep(int* arr, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Control-dependent computation */
        if (arr[i] > 0) {
            sum += arr[i] * 2;
            arr[i] = sum + g_volatile;
        } else {
            sum -= arr[i] / 2;
            arr[i] = -sum;
        }
        
        /* Nested control flow */
        switch (i % 4) {
            case 0: arr[i] += 1; break;
            case 1: arr[i] += 2; break;
            case 2: arr[i] += 3; break;
            case 3: arr[i] += 4; break;
        }
    }
    return sum;
}

/* Mixed integer and floating-point operations */
float test_mixed_types(float* farr, int* iarr, int n) {
    float fsum = 0.0f;
    int isum = 0;
    
    for (int i = 1; i < n; i++) {
        /* Cross-type dependencies */
        farr[i] = farr[i-1] * 1.5f + (float)iarr[i];
        iarr[i] = (int)farr[i] + iarr[i-1] + g_volatile;
        
        /* Mixed operations create different DDG edge data types */
        fsum += farr[i];
        isum += iarr[i];
    }
    return fsum + (float)isum;
}

/* Complex nested loop structure */
int test_nested_loops(int* arr, int n, int m) {
    int total = 0;
    
    for (int i = 1; i < n; i++) {
        int inner_sum = 0;
        
        /* Inner loop with dependencies */
        for (int j = 1; j < m; j++) {
            /* Loop-carried dependency in inner loop */
            arr[j] = arr[j-1] + i * j + g_volatile;
            inner_sum += arr[j];
        }
        
        /* Outer loop dependency */
        total += inner_sum * i;
        
        /* Cross-iteration dependency in outer loop */
        if (i > 1) {
            arr[i] = arr[i-1] + total;
        }
    }
    return total;
}

/* Function with volatile and function calls */
int test_volatile_and_calls(int* arr, int n) {
    volatile int local_volatile = 0;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Volatile access creates hard dependencies */
        local_volatile = i;
        
        /* Function-like macro to prevent inlining */
        #ifdef __GNUC__
        __asm__ volatile ("" : : "r"(local_volatile) : "memory");
        #endif
        
        /* Memory clobbering operations */
        arr[i] = local_volatile + arr[i] + g_volatile;
        
        /* Complex computation with multiple dependencies */
        sum = (sum * 31 + arr[i]) % 10007;
        
        /* Additional volatile access */
        g_volatile = (g_volatile + 1) & 0xFF;
    }
    return sum;
}

/* Main function that exercises all test cases */
int main(int argc, char** argv) {
    int n = 1000;
    int m = 100;
    
    /* Initialize arrays with non-zero values */
    int* arr1 = (int*)malloc(n * sizeof(int));
    int* arr2 = (int*)malloc(n * sizeof(int));
    float* farr = (float*)malloc(n * sizeof(float));
    
    for (int i = 0; i < n; i++) {
        arr1[i] = (i * 37) % 101;
        arr2[i] = (i * 53) % 103;
        farr[i] = (float)i * 0.5f;
    }
    
    /* Run all test functions to create various DDG edge types */
    int result = 0;
    
    result += test_raw_dep(arr1, n);
    result += test_war_waw_dep(arr2, n);
    result += test_memory_aliasing(arr1, arr2, n);
    result += test_control_dep(arr1, n);
    
    float float_result = test_mixed_types(farr, arr1, n);
    result += (int)float_result;
    
    result += test_nested_loops(arr2, n/10, m);
    result += test_volatile_and_calls(arr1, n);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Store to global to ensure side effects */
    g_result = result;
    
    free(arr1);
    free(arr2);
    free(farr);
    
    return g_result != 0 ? 0 : 1;
}
