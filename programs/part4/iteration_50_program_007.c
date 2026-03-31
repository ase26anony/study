/* test_ddg_coverage.c
 * Designed to trigger DDG edge creation in GCC's instruction scheduler
 * Compile with: gcc -O2 -fmodulo-sched -funroll-loops -c test_ddg_coverage.c
 * Or: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -ftree-vectorize -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimization */
volatile int g_volatile_counter = 0;
int g_global_array[1024];
float g_float_array[1024];
double g_double_array[1024];

/* Function prototypes */
int test_raw_dependencies(int *arr, int n, int start);
int test_war_waw_dependencies(int *arr, float *farr, int n);
int test_memory_aliasing(int *arr1, int *arr2, int *ptr1, int *ptr2, int n);
int test_control_dependencies(int *arr, int n, int threshold);
int test_mixed_dependencies(double *darr, float *farr, int *iarr, int n);
int test_loop_carried_dependencies(int *arr, int n, int distance);

/* Test 1: True Data Dependencies (RAW - Read After Write) */
int test_raw_dependencies(int *arr, int n, int start) {
    int sum = start;
    
    /* Multiple RAW dependencies with different distances */
    for (int i = 2; i < n; i++) {
        /* Flow dependency with distance 1 */
        arr[i] = arr[i-1] + g_volatile_counter;
        
        /* Flow dependency with distance 2 */
        if (i >= 4) {
            arr[i] += arr[i-2] * 2;
        }
        
        /* Flow dependency with distance 3 (creates longer chain) */
        if (i >= 6) {
            arr[i] += arr[i-3] / 3;
        }
        
        /* Floating point RAW dependency */
        g_float_array[i] = g_float_array[i-1] * 1.5f;
        
        /* Double precision RAW dependency */
        g_double_array[i] = g_double_array[i-1] * 2.0;
        
        sum += arr[i];
    }
    
    return sum;
}

/* Test 2: Anti and Output Dependencies (WAR/WAW) */
int test_war_waw_dependencies(int *arr, float *farr, int n) {
    int temp1, temp2;
    float ftemp;
    
    for (int i = 0; i < n; i++) {
        /* WAR (Anti-dependency): Read after write to different location */
        temp1 = arr[i] + 1;      /* Read arr[i] */
        arr[i] = temp1 * 2;      /* Write arr[i] - creates WAR with next iteration's read */
        
        /* Another WAR example */
        ftemp = farr[i];         /* Read farr[i] */
        farr[i] = ftemp + 1.0f;  /* Write farr[i] */
        
        /* WAW (Output-dependency): Write after write */
        arr[i] = i * 3;          /* First write to arr[i] */
        arr[i] = arr[i] + 5;     /* Second write to arr[i] - WAW dependency */
        
        /* Complex WAR/WAW mixture */
        temp2 = arr[i];          /* Read arr[i] - WAR with previous write */
        arr[i] = temp2 + i;      /* Write arr[i] - WAW with previous write, WAR with next read */
        
        /* Prevent dead code elimination */
        g_global_array[i] = arr[i] + (int)ftemp;
    }
    
    return arr[n-1];
}

/* Test 3: Memory Aliasing Dependencies */
int test_memory_aliasing(int *arr1, int *arr2, int *ptr1, int *ptr2, int n) {
    int sum = 0;
    
    /* Force potential aliasing */
    ptr1 = arr1;
    ptr2 = arr2;
    
    /* Random offset to create ambiguous aliasing */
    int offset = g_volatile_counter % 10;
    
    for (int i = 1; i < n - 1; i++) {
        /* Potential aliasing through pointers */
        *ptr1 = *ptr2 + i;
        
        /* Array accesses with variable indices - compiler can't prove no alias */
        arr1[i] = arr2[i + offset] * 2;
        
        /* More complex aliasing pattern */
        arr1[i + 1] = arr1[i] + arr2[i - 1];
        
        /* Pointer arithmetic creating potential aliasing */
        ptr1 = &arr1[(i * 3) % n];
        ptr2 = &arr2[(i * 7) % n];
        
        /* Memory dependency through global */
        g_global_array[i] = arr1[i] + arr2[i];
        
        sum += *ptr1 + *ptr2;
    }
    
    return sum;
}

/* Test 4: Control Dependencies */
int test_control_dependencies(int *arr, int n, int threshold) {
    int count = 0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple conditional branches creating control dependencies */
        if (arr[i] > threshold) {
            arr[i] = arr[i] * 2;
            count++;
            
            /* Nested condition */
            if (arr[i] > threshold * 2) {
                arr[i] = arr[i] / 3;
                g_float_array[i] = (float)arr[i];
            }
        } else if (arr[i] < -threshold) {
            arr[i] = arr[i] - 10;
            g_double_array[i] = (double)arr[i];
        } else {
            arr[i] = arr[i] + 1;
        }
        
        /* Another condition with side effects */
        if (i % 3 == 0) {
            g_global_array[i] = arr[i];
        } else if (i % 3 == 1) {
            g_global_array[i] = arr[i] * 2;
        } else {
            g_global_array[i] = arr[i] * 3;
        }
        
        /* Loop with break condition (creates control flow) */
        if (count > n / 2) {
            arr[i] = 0;
            break;
        }
    }
    
    return count;
}

/* Test 5: Mixed Dependencies */
int test_mixed_dependencies(double *darr, float *farr, int *iarr, int n) {
    double dsum = 0.0;
    float fsum = 0.0f;
    int isum = 0;
    
    for (int i = 1; i < n; i++) {
        /* Mixed type RAW */
        farr[i] = (float)darr[i-1] + 1.0f;
        
        /* Integer RAW with different operation */
        iarr[i] = iarr[i-1] * 2 + i;
        
        /* Double precision computation with dependency */
        darr[i] = darr[i-1] * 1.01 + (double)farr[i];
        
        /* WAR across different types */
        float temp_f = farr[i];      /* Read farr[i] */
        farr[i] = temp_f * 2.0f;     /* Write farr[i] */
        
        /* WAW on integer array */
        iarr[i] = i * 3;             /* First write */
        iarr[i] = iarr[i] + 5;       /* Second write */
        
        /* Control dependency */
        if (darr[i] > 100.0) {
            farr[i] = farr[i] / 2.0f;
            iarr[i] = iarr[i] - 1;
        }
        
        /* Accumulate results */
        dsum += darr[i];
        fsum += farr[i];
        isum += iarr[i];
        
        /* Memory operation to global */
        g_global_array[i % 1024] = iarr[i];
    }
    
    return isum + (int)fsum + (int)dsum;
}

/* Test 6: Loop-Carried Dependencies with Various Distances */
int test_loop_carried_dependencies(int *arr, int n, int distance) {
    int sum = 0;
    
    /* Multiple loop-carried dependencies */
    for (int i = distance; i < n; i++) {
        /* Distance = 1 */
        arr[i] = arr[i-1] + 1;
        
        /* Distance = 2 */
        if (i >= distance + 1) {
            arr[i] += arr[i-2] * 2;
        }
        
        /* Distance = 3 */
        if (i >= distance + 2) {
            arr[i] += arr[i-3] / 3;
        }
        
        /* Distance = parameter value (runtime determined) */
        if (i >= distance) {
            arr[i] += arr[i-distance] * 4;
        }
        
        /* Additional dependency chain */
        g_float_array[i] = g_float_array[i-1] + g_float_array[i-2];
        
        sum += arr[i];
    }
    
    return sum;
}

/* Main function that runs all tests */
int main(int argc, char *argv[]) {
    const int N = 1000;
    int result = 0;
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < 1024; i++) {
        g_global_array[i] = i * 3;
        g_float_array[i] = (float)i * 1.5f;
        g_double_array[i] = (double)i * 2.5;
    }
    
    /* Dynamic allocation to prevent compile-time analysis */
    int *array1 = (int*)malloc(N * sizeof(int));
    int *array2 = (int*)malloc(N * sizeof(int));
    float *farray = (float*)malloc(N * sizeof(float));
    double *darray = (double*)malloc(N * sizeof(double));
    
    if (!array1 || !array2 || !farray || !darray) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        array1[i] = i * 2;
        array2[i] = i * 3;
        farray[i] = (float)i * 0.5f;
        darray[i] = (double)i * 0.25;
    }
    
    /* Run all tests to create various DDG edges */
    printf("Running DDG edge creation tests...\n");
    
    /* Test 1: RAW dependencies */
    result += test_raw_dependencies(array1, N, 42);
    
    /* Test 2: WAR/WAW dependencies */
    result += test_war_waw_dependencies(array2, farray, N);
    
    /* Test 3: Memory aliasing */
    int *ptr1, *ptr2;
    result += test_memory_aliasing(array1, array2, ptr1, ptr2, N);
    
    /* Test 4: Control dependencies */
    result += test_control_dependencies(array1, N, 100);
    
    /* Test 5: Mixed dependencies */
    result += test_mixed_dependencies(darray, farray, array2, N);
    
    /* Test 6: Loop-carried dependencies */
    int distance = (g_volatile_counter % 5) + 1; /* Runtime distance */
    result += test_loop_carried_dependencies(array1, N, distance);
    
    /* Use result to prevent dead code elimination */
    printf("Final checksum: %d\n", result);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(farray);
    free(darray);
    
    return 0;
}
