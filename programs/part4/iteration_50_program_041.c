/* test_ddg_coverage.c
 * Program designed to trigger DDG edge creation in GCC's scheduler
 * Compile with: gcc -O2 -funroll-loops -fmodulo-sched -c test_ddg_coverage.c
 * Or: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -ftree-vectorize -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_global_array[1024];
float g_float_array[1024];

/* Function prototypes */
int test_raw_dependencies(int *arr, int n);
int test_war_waw_dependencies(int *arr, int n);
int test_memory_aliasing(int *arr, float *farr, int n);
int test_control_dependencies(int *arr, int n);
int test_mixed_dependencies(int *arr, float *farr, int n);
int test_loop_carried_dependencies(int *arr, int n);

/* Test 1: True Data Dependencies (RAW - Read After Write) */
int test_raw_dependencies(int *arr, int n) {
    int sum = 0;
    
    /* Multiple RAW dependencies with different distances */
    for (int i = 2; i < n; i++) {
        /* Flow dependency: arr[i] depends on arr[i-1] */
        arr[i] = arr[i-1] + g_volatile;
        
        /* Another flow dependency with distance 2 */
        arr[i] += arr[i-2] * 2;
        
        /* Floating point flow dependency */
        g_float_array[i] = g_float_array[i-1] * 1.5f;
        
        /* Complex expression with multiple dependencies */
        sum += arr[i] + (int)g_float_array[i];
    }
    
    return sum;
}

/* Test 2: Anti and Output Dependencies (WAR/WAW) */
int test_war_waw_dependencies(int *arr, int n) {
    int temp1, temp2;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* WAR (Anti-dependency): b reads a, then a is written */
        temp1 = arr[i] + 1;      /* Read arr[i] */
        arr[i] = temp1 * 2;      /* Write arr[i] - anti-dependency with above */
        
        /* WAW (Output-dependency): Multiple writes to same location */
        temp2 = g_global_array[i] + i;
        g_global_array[i] = temp2;          /* First write */
        g_global_array[i] = temp2 * 3;      /* Second write - output dependency */
        
        /* Mixed WAR/WAW with different variables */
        int old_val = arr[i];
        arr[i] = g_global_array[i] + old_val;
        g_global_array[i] = arr[i] / 2;
        
        sum += arr[i] + g_global_array[i];
    }
    
    return sum;
}

/* Test 3: Memory Aliasing Dependencies */
int test_memory_aliasing(int *arr, float *farr, int n) {
    int sum = 0;
    int *ptr1 = arr;
    int *ptr2 = &arr[n/2];  /* Potentially aliases with ptr1 when n is small */
    float *fptr1 = farr;
    float *fptr2 = &farr[1]; /* Overlapping access */
    
    for (int i = 0; i < n/2; i++) {
        /* Potential aliasing between ptr1 and ptr2 */
        *ptr1 = *ptr2 + i;
        *ptr2 = *ptr1 * 2;
        
        /* Potential aliasing with different types (via char*) */
        char *cptr = (char*)ptr1;
        cptr[0] = cptr[1] + 1;
        
        /* Floating point with potential aliasing */
        *fptr1 = *fptr2 + 0.5f;
        *fptr2 = *fptr1 * 2.0f;
        
        /* Array access with variable index (potential aliasing) */
        arr[i] = arr[n - i - 1] + g_volatile;
        
        sum += *ptr1 + *ptr2 + (int)*fptr1;
        
        ptr1++;
        ptr2++;
        fptr1 += 2;
        fptr2 += 2;
    }
    
    return sum;
}

/* Test 4: Control Dependencies */
int test_control_dependencies(int *arr, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Branch creates control dependencies */
        if (arr[i] > 0) {
            /* True path with dependencies */
            arr[i] = arr[i] * 2 + 1;
            g_global_array[i] = arr[i] - 5;
        } else {
            /* False path with different dependencies */
            arr[i] = arr[i] / 2 - 1;
            g_global_array[i] = arr[i] + 10;
        }
        
        /* Nested conditional */
        if (i % 3 == 0) {
            arr[i] += g_volatile;
            if (arr[i] > 100) {
                g_global_array[i] = arr[i] * 3;
            }
        }
        
        /* Loop with break condition (affects control flow) */
        if (sum > 1000000) {
            break;
        }
        
        sum += arr[i] + g_global_array[i];
    }
    
    return sum;
}

/* Test 5: Mixed Dependencies with Function Calls */
int test_mixed_dependencies(int *arr, float *farr, int n) {
    int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < n; i++) {
        arr[i] = i;
        farr[i] = i * 0.5f;
    }
    
    /* Complex loop with all dependency types */
    for (int i = 2; i < n - 2; i++) {
        /* RAW */
        int temp1 = arr[i-1] + arr[i-2];
        
        /* WAR */
        float old_float = farr[i];
        farr[i] = old_float * 2.0f + farr[i-1];
        
        /* WAW */
        arr[i] = temp1 * 3;
        arr[i] = arr[i] + (int)farr[i];  /* Overwrites previous value */
        
        /* Control dependency */
        if (arr[i] % 4 == 0) {
            farr[i] = farr[i] / 2.0f;
            arr[i] = arr[i] * 2;
        }
        
        /* Memory dependency with potential aliasing */
        g_global_array[i] = arr[i] + g_global_array[i-1];
        
        /* Loop-carried dependency with distance > 1 */
        if (i > 10) {
            arr[i] += arr[i-5] / 2;
        }
        
        sum += arr[i] + (int)farr[i] + g_global_array[i];
    }
    
    return sum;
}

/* Test 6: Loop-Carried Dependencies with Different Distances */
int test_loop_carried_dependencies(int *arr, int n) {
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        arr[i] = i % 100;
    }
    
    /* Loop with multiple carried dependencies */
    for (int i = 4; i < n; i++) {
        /* Distance 1 */
        arr[i] = arr[i-1] + g_volatile;
        
        /* Distance 2 */
        arr[i] += arr[i-2] * 2;
        
        /* Distance 3 (floating point) */
        g_float_array[i] = g_float_array[i-3] * 1.1f;
        
        /* Distance 4 with conditional */
        if (i > 10) {
            arr[i] += arr[i-4] / 3;
        }
        
        /* Multiple uses create complex DDG */
        int t1 = arr[i-1];
        int t2 = arr[i-2];
        int t3 = t1 + t2;
        arr[i] = arr[i] + t3;
        
        sum += arr[i] + (int)g_float_array[i];
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
    
    /* Allocate arrays */
    int *arr1 = (int*)malloc(n * sizeof(int));
    int *arr2 = (int*)malloc(n * sizeof(int));
    float *farr = (float*)malloc(n * sizeof(float));
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        arr1[i] = i % 50;
        arr2[i] = i % 30;
        farr[i] = i * 0.25f;
        g_global_array[i] = i % 20;
        g_float_array[i] = i * 0.1f;
    }
    
    int total_sum = 0;
    
    /* Run all tests to create various DDG edges */
    total_sum += test_raw_dependencies(arr1, n);
    total_sum += test_war_waw_dependencies(arr2, n);
    total_sum += test_memory_aliasing(arr1, farr, n);
    total_sum += test_control_dependencies(arr2, n);
    total_sum += test_mixed_dependencies(arr1, farr, n);
    total_sum += test_loop_carried_dependencies(arr2, n);
    
    /* Use volatile to prevent dead code elimination */
    g_volatile = total_sum % 1000;
    
    printf("Result: %d\n", total_sum);
    
    /* Clean up */
    free(arr1);
    free(arr2);
    free(farr);
    
    return total_sum % 256;
}
