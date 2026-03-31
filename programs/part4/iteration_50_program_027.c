/* test_ddg_edges.c
 * Program designed to trigger DDG edge creation in GCC's scheduler
 * Compile with: gcc -O2 -funroll-loops -fmodulo-sched -c test_ddg_edges.c
 * Or: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -ftree-vectorize -c test_ddg_edges.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimizations */
volatile int g_volatile = 0;
int g_global_array[1024];
int g_result = 0;

/* Function prototypes */
int test_raw_dependencies(int *arr1, int *arr2, int n);
int test_war_waw_dependencies(int *arr, int n);
int test_memory_aliasing(int *arr1, int *arr2, int *arr3, int n);
int test_control_dependencies(int *arr, int n);
int test_mixed_dependencies(double *darr, int *iarr, int n);
int test_loop_carried_dependencies(int *arr, int n);

/* Test 1: True Data Dependencies (RAW - Read After Write) */
int test_raw_dependencies(int *arr1, int *arr2, int n) {
    int sum = 0;
    
    /* Multiple RAW dependencies with different distances */
    for (int i = 2; i < n; i++) {
        /* Flow dependency: arr1[i] depends on arr1[i-1] */
        arr1[i] = arr1[i-1] + arr2[i] + g_volatile;
        
        /* Another flow dependency with distance 2 */
        arr2[i] = arr1[i-2] * 3 + i;
        
        /* Mixed integer operations creating dependency chain */
        sum += arr1[i] + arr2[i];
    }
    
    /* Additional loop with floating point RAW dependencies */
    double temp = 0.0;
    for (int i = 1; i < n; i++) {
        double *dptr = (double*)&arr1[i];
        *dptr = *dptr * 1.5 + temp;
        temp = *dptr;
        sum += (int)temp;
    }
    
    return sum;
}

/* Test 2: Anti and Output Dependencies (WAR/WAW) */
int test_war_waw_dependencies(int *arr, int n) {
    int sum = 0;
    int temp1, temp2;
    
    for (int i = 0; i < n; i++) {
        /* WAR (Anti-dependency): Read arr[i] before writing to it */
        temp1 = arr[i] + i;
        
        /* WAW (Output-dependency): Multiple writes to same location */
        arr[i] = temp1 * 2;
        arr[i] = arr[i] + g_volatile;  // Second write creates WAW
        
        /* Another WAR example with different variables */
        temp2 = arr[i] * 3;
        arr[i] = temp2 / 2;  // Write after read of arr[i]
        
        sum += arr[i];
    }
    
    /* Nested loop with output dependencies */
    for (int i = 0; i < n/2; i++) {
        for (int j = 0; j < 4; j++) {
            /* Multiple writes to same array element */
            g_global_array[i] = i + j;
            g_global_array[i] = g_global_array[i] * (j + 1);
            sum += g_global_array[i];
        }
    }
    
    return sum;
}

/* Test 3: Memory Aliasing Dependencies */
int test_memory_aliasing(int *arr1, int *arr2, int *arr3, int n) {
    int sum = 0;
    
    /* Use pointers that may alias */
    int *ptr1 = arr1;
    int *ptr2 = arr1 + 1;  // Overlapping with arr1
    
    for (int i = 1; i < n - 1; i++) {
        /* Potential memory dependency: ptr1 and ptr2 may alias */
        *ptr1 = *ptr2 + i + g_volatile;
        *ptr2 = *ptr1 * 2;
        
        /* Array accesses with non-linear indices */
        arr3[i] = arr1[i/2] + arr2[i%10];
        
        /* Update pointers - creates complex memory dependencies */
        ptr1++;
        ptr2++;
        
        sum += arr3[i];
    }
    
    /* Loop with function calls that clobber memory */
    for (int i = 0; i < n; i++) {
        /* External function call acts as memory barrier */
        arr1[i] = arr2[i] + rand() % 10;
        sum += arr1[i];
    }
    
    return sum;
}

/* Test 4: Control Dependencies */
int test_control_dependencies(int *arr, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple conditional branches inside loop */
        if (i % 3 == 0) {
            arr[i] = arr[i] * 2 + g_volatile;
        } else if (i % 3 == 1) {
            arr[i] = arr[i] / 2;
        } else {
            arr[i] = arr[i] + 1;
        }
        
        /* Nested conditionals */
        if (arr[i] > 100) {
            arr[i] = 100;
            sum += 5;
        } else if (arr[i] < 0) {
            arr[i] = 0;
            sum -= 3;
        }
        
        /* Switch statement for more control flow */
        switch (i % 4) {
            case 0: arr[i] += 10; break;
            case 1: arr[i] -= 5; break;
            case 2: arr[i] *= 2; break;
            case 3: arr[i] /= 2; break;
        }
        
        sum += arr[i];
    }
    
    return sum;
}

/* Test 5: Mixed Data Types and Operations */
int test_mixed_dependencies(double *darr, int *iarr, int n) {
    double dsum = 0.0;
    int isum = 0;
    
    for (int i = 1; i < n; i++) {
        /* Mix float and int operations */
        darr[i] = darr[i-1] * 1.1 + (double)iarr[i];
        
        /* Type conversions create dependencies */
        iarr[i] = (int)(darr[i] * 2.0) + i;
        
        /* Volatile access breaks optimization */
        darr[i] += (double)g_volatile;
        
        dsum += darr[i];
        isum += iarr[i];
    }
    
    /* Loop with different data type dependencies */
    for (int i = 2; i < n; i += 2) {
        float ftemp = (float)darr[i];
        darr[i-1] = (double)ftemp * 0.5;
        iarr[i] = (int)darr[i-1] + iarr[i-2];
        
        isum += iarr[i];
    }
    
    return isum + (int)dsum;
}

/* Test 6: Loop-Carried Dependencies with Various Distances */
int test_loop_carried_dependencies(int *arr, int n) {
    int sum = 0;
    
    /* Distance 1 dependency */
    for (int i = 1; i < n; i++) {
        arr[i] = arr[i-1] + i * 2;
        sum += arr[i];
    }
    
    /* Distance 2 dependency */
    for (int i = 2; i < n; i++) {
        g_global_array[i] = g_global_array[i-2] * 3 + g_volatile;
        sum += g_global_array[i];
    }
    
    /* Distance 4 dependency with unrolling candidate */
    for (int i = 4; i < n; i++) {
        arr[i] = arr[i-4] / 2 + i;
        sum += arr[i];
    }
    
    /* Complex distance pattern */
    for (int i = 5; i < n; i++) {
        int idx = i % 5;
        arr[i] = arr[i-idx-1] + arr[i-3] + g_volatile;
        sum += arr[i];
    }
    
    return sum;
}

/* Main function that runs all tests */
int main(int argc, char *argv[]) {
    const int N = 1000;
    int total_result = 0;
    
    /* Initialize arrays with non-constant data */
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(N * sizeof(int));
    int *arr3 = (int*)malloc(N * sizeof(int));
    double *darr = (double*)malloc(N * sizeof(double));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < N; i++) {
        arr1[i] = (i * 13) % 97;
        arr2[i] = (i * 17) % 89;
        arr3[i] = (i * 19) % 101;
        darr[i] = (double)(i % 50) * 1.5;
        g_global_array[i] = i % 23;
    }
    
    /* Run all tests to create various DDG edges */
    total_result += test_raw_dependencies(arr1, arr2, N);
    total_result += test_war_waw_dependencies(arr3, N);
    total_result += test_memory_aliasing(arr1, arr2, arr3, N);
    total_result += test_control_dependencies(arr1, N);
    total_result += test_mixed_dependencies(darr, arr2, N);
    total_result += test_loop_carried_dependencies(arr3, N);
    
    /* Use volatile to prevent dead code elimination */
    g_volatile = total_result % 1000;
    
    /* Final computation that depends on all results */
    int final_result = 0;
    for (int i = 0; i < N; i++) {
        final_result += arr1[i] + arr2[i] + arr3[i] + (int)darr[i];
    }
    
    final_result += total_result;
    final_result += g_volatile;
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", final_result);
    
    free(arr1);
    free(arr2);
    free(arr3);
    free(darr);
    
    return final_result != 0 ? 0 : 1;
}
