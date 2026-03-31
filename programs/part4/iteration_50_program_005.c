/* test_ddg_coverage.c
 * Designed to trigger DDG edge creation in GCC's instruction scheduler
 * Compile with: gcc -O2 -fmodulo-sched -funroll-loops -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_global_array[1024];
int g_result = 0;

/* Function prototypes */
int test_raw_dependencies(int *arr, int n);
int test_war_waw_dependencies(int *arr1, int *arr2, int n);
int test_memory_aliasing(int *arr, int *ptr1, int *ptr2, int n);
int test_control_dependencies(int *arr, int n);
int test_mixed_dependencies(int *arr1, int *arr2, float *farr, int n);
int test_loop_carried_dependencies(int *arr, int n);

/* Test 1: True Data Dependencies (RAW) with loop-carried dependency */
int test_raw_dependencies(int *arr, int n) {
    int sum = 0;
    /* Loop with multiple RAW dependencies */
    for (int i = 2; i < n; i++) {
        /* RAW dependency: arr[i] depends on arr[i-1] and arr[i-2] */
        arr[i] = arr[i-1] + arr[i-2] + g_volatile;
        /* Another RAW dependency chain */
        arr[i] = arr[i] * 3 - arr[i-1];
        /* Cross-iteration dependency (distance > 0) */
        sum += arr[i] + arr[i-2];
    }
    return sum;
}

/* Test 2: Anti (WAR) and Output (WAW) Dependencies */
int test_war_waw_dependencies(int *arr1, int *arr2, int n) {
    int temp = 0;
    
    for (int i = 0; i < n; i++) {
        /* WAR (anti-dependency): arr2[i] read before arr1[i] write */
        int read_val = arr2[i] + g_volatile;
        
        /* WAW (output-dependency): arr1[i] written twice */
        arr1[i] = read_val * 2;
        arr1[i] = arr1[i] + i;  /* Second write to same location */
        
        /* Another WAR: arr1[i] read, then arr2[i] written */
        temp += arr1[i];
        arr2[i] = temp % 256;
    }
    
    return temp;
}

/* Test 3: Memory Aliasing Dependencies */
int test_memory_aliasing(int *arr, int *ptr1, int *ptr2, int n) {
    int sum = 0;
    
    /* Force potential aliasing */
    ptr1 = &arr[0];
    ptr2 = &arr[n/2];
    
    for (int i = 0; i < n/2; i++) {
        /* Memory operations that may alias */
        *ptr1 = *ptr2 + g_volatile;  /* Could create memory dependency */
        sum += *ptr1;
        
        /* Pointer arithmetic - creates ambiguous dependencies */
        ptr1++;
        ptr2--;
        
        /* Array access with variable index - harder to analyze */
        arr[i*2] = arr[n-i-1] + 1;
    }
    
    return sum;
}

/* Test 4: Control Dependencies */
int test_control_dependencies(int *arr, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Control-dependent computation */
        if (arr[i] > g_volatile) {
            /* Branch creates control dependencies */
            arr[i] = arr[i] * 2 + 1;
            sum += arr[i];
        } else {
            arr[i] = arr[i] / 2 - 1;
            sum -= arr[i];
        }
        
        /* Nested control flow */
        for (int j = 0; j < 4; j++) {
            if ((i + j) % 3 == 0) {
                arr[i] += j;
            }
        }
    }
    
    return sum;
}

/* Test 5: Mixed Data Types and Dependencies */
int test_mixed_dependencies(int *arr1, int *arr2, float *farr, int n) {
    float fsum = 0.0f;
    
    for (int i = 1; i < n; i++) {
        /* Integer RAW */
        arr1[i] = arr1[i-1] + arr2[i];
        
        /* Floating point RAW */
        farr[i] = farr[i-1] * 1.5f + (float)arr1[i];
        
        /* Mixed type dependencies */
        arr2[i] = (int)farr[i] + g_volatile;
        
        /* WAR between float and int */
        fsum += farr[i];
        farr[i] = fsum / (i+1);
    }
    
    return (int)fsum;
}

/* Test 6: Complex Loop-Carried Dependencies */
int test_loop_carried_dependencies(int *arr, int n) {
    int sum = 0;
    
    /* Multiple interleaved dependency chains */
    for (int i = 3; i < n; i++) {
        /* Chain 1: distance 1 */
        int t1 = arr[i-1] + g_volatile;
        
        /* Chain 2: distance 2 */
        int t2 = arr[i-2] * 2;
        
        /* Chain 3: distance 3 */
        int t3 = arr[i-3] / 3;
        
        /* Combine chains with different distances */
        arr[i] = t1 + t2 - t3;
        
        /* Create output dependency */
        arr[i] = arr[i] % 1000;
        
        /* Anti-dependency with earlier computation */
        sum = arr[i-1] + sum;
        arr[i-1] = i * 2;
    }
    
    return sum;
}

/* Main function that runs all tests */
int main(int argc, char **argv) {
    int n = 1000;
    int result = 0;
    
    /* Initialize arrays with non-constant values */
    int *arr1 = (int*)malloc(n * sizeof(int));
    int *arr2 = (int*)malloc(n * sizeof(int));
    float *farr = (float*)malloc(n * sizeof(float));
    
    /* Initialize with pseudo-random values to prevent constant propagation */
    for (int i = 0; i < n; i++) {
        arr1[i] = (i * 37) % 100;
        arr2[i] = (i * 53) % 100;
        farr[i] = (float)((i * 71) % 100) / 3.0f;
    }
    
    /* Run all tests to create various DDG edge types */
    result += test_raw_dependencies(arr1, n);
    result += test_war_waw_dependencies(arr1, arr2, n);
    result += test_memory_aliasing(arr1, &arr1[0], &arr1[n/2], n);
    result += test_control_dependencies(arr2, n);
    result += test_mixed_dependencies(arr1, arr2, farr, n);
    result += test_loop_carried_dependencies(arr1, n);
    
    /* Store result to prevent dead code elimination */
    g_result = result;
    
    /* Print result to ensure code isn't optimized away */
    printf("Result: %d\n", result);
    
    free(arr1);
    free(arr2);
    free(farr);
    
    return 0;
}
