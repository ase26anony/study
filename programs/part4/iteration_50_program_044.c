/* test_ddg_coverage.c
 * Program designed to trigger DDG edge creation in GCC's scheduler
 * Compile with: gcc -O2 -funroll-loops -fmodulo-sched -c test_ddg_coverage.c
 * Or: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_global_array[1024];
float g_float_array[1024];
int* g_ptr1;
int* g_ptr2;

/* Function prototypes */
int test_raw_dependencies(int* arr, int n);
int test_war_waw_dependencies(int* arr, float* farr, int n);
int test_memory_aliasing(int* arr1, int* arr2, int n);
int test_control_dependencies(int* arr, int n);
int test_mixed_dependencies(int* arr, float* farr, int n);
int test_loop_carried_dependencies(int* arr, int n);

/* Test 1: True Data Dependencies (RAW) */
int test_raw_dependencies(int* arr, int n) {
    int sum = 0;
    
    /* Loop with multiple RAW dependencies */
    for (int i = 2; i < n; i++) {
        /* Flow dependency chain: i-2 -> i-1 -> i */
        arr[i] = arr[i-1] + arr[i-2] + i;
        
        /* Another flow dependency with different distance */
        g_global_array[i] = g_global_array[i-3] * 2;
        
        /* Mixed integer/float flow dependencies */
        g_float_array[i] = g_float_array[i-1] * 1.5f + arr[i];
        
        sum += arr[i];
    }
    
    /* Prevent dead code elimination */
    return sum + g_volatile;
}

/* Test 2: Anti and Output Dependencies (WAR, WAW) */
int test_war_waw_dependencies(int* arr, float* farr, int n) {
    int temp;
    float ftemp;
    
    for (int i = 1; i < n; i++) {
        /* WAR (anti-dependency): read arr[i] before writing it */
        temp = arr[i] + i;
        
        /* WAW (output-dependency): multiple writes to same location */
        arr[i] = temp * 2;
        arr[i] = arr[i] + 1;  /* Overwrites previous value */
        
        /* WAR with floating point */
        ftemp = farr[i];
        farr[i] = ftemp * 2.0f;
        farr[i] = farr[i] + 1.0f;  /* Another WAW */
        
        /* Complex WAR/WAW mixing */
        int x = arr[i-1];
        arr[i-1] = x + farr[i];
    }
    
    return arr[n-1] + (int)farr[n-1];
}

/* Test 3: Memory Aliasing Dependencies */
int test_memory_aliasing(int* arr1, int* arr2, int n) {
    /* Setup pointers that may alias */
    int* p = arr1;
    int* q = arr2;
    
    /* Force potential aliasing through global pointers */
    g_ptr1 = arr1;
    g_ptr2 = arr1 + 1;  /* Overlap by 1 element */
    
    int sum = 0;
    
    for (int i = 2; i < n; i++) {
        /* Memory operations with potential aliasing */
        *p = *q + i;
        p++;
        q++;
        
        /* Access through potentially aliasing global pointers */
        *g_ptr1 = *g_ptr2 * 2;
        g_ptr1++;
        g_ptr2++;
        
        /* Array accesses with variable indices (hard to analyze) */
        arr1[i] = arr2[i-1] + arr1[i-2];
        arr2[i] = arr1[i] * 3;
        
        sum += arr1[i] + arr2[i];
    }
    
    return sum;
}

/* Test 4: Control Dependencies */
int test_control_dependencies(int* arr, int n) {
    int count = 0;
    
    for (int i = 0; i < n; i++) {
        /* Branch creates control dependencies */
        if (arr[i] > 0) {
            /* Instructions dependent on the branch */
            arr[i] = arr[i] * 2;
            count++;
            
            /* Nested control flow */
            if (i % 3 == 0) {
                arr[i] += g_volatile;
            } else {
                arr[i] -= 1;
            }
        } else {
            arr[i] = -arr[i];
        }
        
        /* Another branch with different condition */
        switch (i % 4) {
            case 0: arr[i] += 1; break;
            case 1: arr[i] += 2; break;
            case 2: arr[i] += 3; break;
            default: arr[i] += 4; break;
        }
    }
    
    return count;
}

/* Test 5: Mixed Dependencies */
int test_mixed_dependencies(int* arr, float* farr, int n) {
    int sum = 0;
    
    for (int i = 3; i < n; i++) {
        /* RAW */
        int t1 = arr[i-1] + arr[i-2];
        
        /* WAR */
        int t2 = farr[i];
        farr[i] = t1 * 0.5f;
        
        /* WAW */
        arr[i] = t1 + t2;
        arr[i] = arr[i] * 2;
        
        /* Memory dependency with potential aliasing */
        if (i % 2 == 0) {
            g_global_array[i] = arr[i];
        } else {
            g_global_array[i] = farr[i];
        }
        
        /* Loop-carried dependency with distance 2 */
        farr[i] = farr[i-2] + 1.0f;
        
        /* Control-dependent operation */
        int multiplier = (arr[i] > 100) ? 2 : 1;
        arr[i] = arr[i] * multiplier;
        
        sum += arr[i];
    }
    
    return sum;
}

/* Test 6: Loop-Carried Dependencies with Various Distances */
int test_loop_carried_dependencies(int* arr, int n) {
    /* Initialize with pattern */
    for (int i = 0; i < n; i++) {
        arr[i] = i;
    }
    
    int result = 0;
    
    /* Loop with multiple distances */
    for (int i = 4; i < n; i++) {
        /* Distance 1 */
        arr[i] += arr[i-1];
        
        /* Distance 2 */
        arr[i] += arr[i-2] * 2;
        
        /* Distance 3 (floating point) */
        g_float_array[i] = g_float_array[i-3] + 0.5f;
        
        /* Distance 4 through memory */
        g_global_array[i] = g_global_array[i-4] + arr[i];
        
        /* Variable distance (harder to analyze) */
        int dist = (i % 5) + 1;
        if (i >= dist) {
            arr[i] += arr[i-dist];
        }
        
        result += arr[i];
    }
    
    return result;
}

/* Main function that runs all tests */
int main(int argc, char** argv) {
    /* Use command line argument for variable iteration count */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 100) n = 100;
        if (n > 10000) n = 10000;
    }
    
    /* Allocate arrays */
    int* arr1 = (int*)malloc(n * sizeof(int));
    int* arr2 = (int*)malloc(n * sizeof(int));
    float* farr = (float*)malloc(n * sizeof(float));
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < n; i++) {
        arr1[i] = (i * 3) % 97;
        arr2[i] = (i * 7) % 101;
        farr[i] = (float)(i % 53) * 0.7f;
        g_global_array[i] = i;
        g_float_array[i] = (float)i * 0.3f;
    }
    
    /* Setup global pointers */
    g_ptr1 = arr1;
    g_ptr2 = arr2;
    
    /* Run all tests to create various DDG edges */
    int result = 0;
    
    result += test_raw_dependencies(arr1, n);
    result += test_war_waw_dependencies(arr2, farr, n);
    result += test_memory_aliasing(arr1, arr2, n);
    result += test_control_dependencies(arr1, n);
    result += test_mixed_dependencies(arr1, farr, n);
    result += test_loop_carried_dependencies(arr2, n);
    
    /* Add volatile to prevent optimization of final result */
    result += g_volatile;
    
    /* Print checksum to prevent dead code elimination */
    printf("Result checksum: %d\n", result);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr);
    
    return 0;
}
