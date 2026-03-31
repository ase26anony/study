/* test_ddg_coverage.c
 * Program designed to trigger DDG edge creation in GCC's scheduler
 * Compile with: gcc -O2 -fmodulo-sched -funroll-loops -c test_ddg_coverage.c
 * Or: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_array[1024];
int g_result = 0;

/* Function to create true data dependencies (RAW) */
int test_raw_dep(int *arr, int n) {
    int sum = 0;
    /* Loop with flow dependencies across iterations */
    for (int i = 2; i < n; i++) {
        /* True dependency: arr[i] depends on arr[i-1] and arr[i-2] */
        arr[i] = arr[i-1] + arr[i-2] + i;
        /* Another flow dependency chain */
        sum += arr[i] * 3;
    }
    return sum + g_volatile;
}

/* Function to create anti and output dependencies (WAR/WAW) */
int test_war_waw_dep(float *fa, int *ia, int n) {
    float temp = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Anti-dependency (WAR): reading ia[i] before writing it */
        float x = (float)ia[i] * 0.5f;
        
        /* Output dependency (WAW): multiple writes to fa[i] */
        fa[i] = x * 2.0f;
        fa[i] = fa[i] * 3.0f + (float)i;  // Second write to same location
        
        /* Write to ia[i] creates WAR with earlier read */
        ia[i] = (int)fa[i] + i;
        
        temp += fa[i];
    }
    return (int)temp + g_volatile;
}

/* Function with memory aliasing dependencies */
int test_memory_aliasing(int *arr1, int *arr2, int *ptr1, int *ptr2, int n) {
    int sum = 0;
    
    /* ptr1 and ptr2 may alias with arr1/arr2 */
    *ptr1 = 42;
    *ptr2 = 84;
    
    for (int i = 1; i < n; i++) {
        /* Memory dependencies with possible aliasing */
        arr1[i] = arr2[i-1] + *ptr1;
        arr2[i] = arr1[i] + *ptr2;
        
        /* Pointer arithmetic that may alias */
        *(ptr1 + (i % 4)) = i;
        *(ptr2 + ((i+1) % 4)) = i * 2;
        
        sum += arr1[i] + arr2[i];
    }
    return sum + g_volatile;
}

/* Function with control dependencies */
int test_control_dep(int *arr, int n, int threshold) {
    int count = 0;
    
    for (int i = 0; i < n; i++) {
        /* Control-dependent operations */
        if (arr[i] > threshold) {
            arr[i] = arr[i] * 2;
            count++;
        } else if (arr[i] < -threshold) {
            arr[i] = arr[i] / 2;
            count--;
        } else {
            arr[i] = arr[i] + threshold;
        }
        
        /* Nested control flow */
        for (int j = 0; j < 3; j++) {
            if ((i + j) % 5 == 0) {
                arr[i] += j;
            }
        }
    }
    return count + g_volatile;
}

/* Complex loop with mixed dependencies */
int test_mixed_dependencies(double *darr, int *iarr, int n) {
    double acc = 0.0;
    
    /* Loop with multiple dependency types and distances */
    for (int i = 4; i < n; i++) {
        /* Flow dependency with distance 2 */
        darr[i] = darr[i-2] * 1.5 + (double)iarr[i];
        
        /* Anti-dependency */
        double old_val = darr[i-1];
        darr[i-1] = (double)iarr[i-1] * 2.0;
        
        /* Output dependency */
        iarr[i] = (int)darr[i];
        iarr[i] = (int)(darr[i] * 0.75);  // Second write
        
        /* Memory operation that may alias */
        g_array[i % 1024] = iarr[i] + (int)old_val;
        
        /* Complex expression with multiple operations */
        acc += darr[i] * 0.3 + darr[i-1] * 0.7 + (double)g_array[(i+1) % 1024];
    }
    
    return (int)acc + g_volatile;
}

/* Function with volatile accesses creating hard dependencies */
int test_volatile_deps(int *arr, int n) {
    volatile int local_volatile = 0;
    int sum = 0;
    
    for (int i = 1; i < n; i++) {
        /* Volatile read creates dependency barrier */
        int v = local_volatile;
        
        /* Operations dependent on volatile read */
        arr[i] = arr[i-1] + v + i;
        
        /* Volatile write creates another barrier */
        local_volatile = arr[i] % 100;
        
        /* Mix of operations */
        sum += arr[i];
        
        /* Function call acts as memory clobber */
        if (i % 100 == 0) {
            sum += g_volatile;  // External volatile access
        }
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
    
    /* Allocate and initialize arrays */
    int *arr1 = (int*)malloc(n * sizeof(int));
    int *arr2 = (int*)malloc(n * sizeof(int));
    float *farr = (float*)malloc(n * sizeof(float));
    double *darr = (double*)malloc(n * sizeof(double));
    int *iarr = (int*)malloc(n * sizeof(int));
    
    for (int i = 0; i < n; i++) {
        arr1[i] = (i * 3) % 97;
        arr2[i] = (i * 7) % 113;
        farr[i] = (float)(i % 79) * 0.7f;
        darr[i] = (double)(i % 131) * 1.3;
        iarr[i] = (i * 11) % 151;
    }
    
    int total = 0;
    
    /* Run all test functions to create various DDG edge types */
    total += test_raw_dep(arr1, n);
    total += test_war_waw_dep(farr, arr2, n);
    total += test_memory_aliasing(arr1, arr2, &arr1[10], &arr2[20], n);
    total += test_control_dep(arr1, n, 50);
    total += test_mixed_dependencies(darr, iarr, n);
    total += test_volatile_deps(arr2, n);
    
    /* Use results to prevent dead code elimination */
    printf("Result checksum: %d\n", total);
    
    /* Store to global to ensure side effects */
    g_result = total;
    
    free(arr1);
    free(arr2);
    free(farr);
    free(darr);
    free(iarr);
    
    return g_result != 0 ? 0 : 1;
}
