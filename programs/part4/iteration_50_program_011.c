/* test_ddg_coverage.c
 * Designed to trigger create_ddg_edge() logic in GCC's scheduler
 * Compile with: gcc -O2 -fmodulo-sched -funroll-loops -c test_ddg_coverage.c
 * Or: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_global_array[1024];
int g_result = 0;

/* Function prototypes */
int test_raw_dependencies(int *arr, int n, int start);
int test_war_waw_dependencies(int *arr, int *brr, int n);
int test_memory_aliasing(int *arr, int *brr, int n, int *ptr1, int *ptr2);
int test_control_dependencies(int *arr, int n, int threshold);
int test_mixed_dependencies(float *farr, int *iarr, int n);
int test_loop_carried_dependencies(int *arr, int n, int distance);

/* Test 1: True Data Dependencies (RAW) with loop-carried dependency */
int test_raw_dependencies(int *arr, int n, int start) {
    int sum = start;
    
    /* Loop with RAW dependencies and loop-carried dependency */
    for (int i = 2; i < n; i++) {
        /* Flow dependency: arr[i] depends on arr[i-1] */
        arr[i] = arr[i-1] + g_volatile;
        
        /* Another flow dependency chain */
        arr[i] = arr[i] * 2 - arr[i-2];
        
        /* Mix with global to prevent elimination */
        sum += arr[i] + g_global_array[i & 1023];
    }
    
    /* Prevent tail recursion optimization */
    asm volatile("" : "+r"(sum) : : "memory");
    return sum;
}

/* Test 2: Anti (WAR) and Output (WAW) Dependencies */
int test_war_waw_dependencies(int *arr, int *brr, int n) {
    int temp = 0;
    
    for (int i = 1; i < n - 1; i++) {
        /* WAR (anti-dependency): brr[i] reads after arr[i] was written */
        int x = arr[i];          /* Read arr[i] */
        arr[i] = brr[i] + 1;     /* Write arr[i] - anti-dep with line above */
        
        /* WAW (output-dependency): Multiple writes to same location */
        brr[i] = x * 2;          /* Write brr[i] */
        brr[i] = brr[i] + i;     /* Another write to brr[i] - output dep */
        
        /* Add complexity with floating point */
        float fx = (float)x;
        fx = fx * 1.5f;
        temp += (int)fx;
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
    }
    
    return temp;
}

/* Test 3: Memory Aliasing Dependencies */
int test_memory_aliasing(int *arr, int *brr, int n, int *ptr1, int *ptr2) {
    int sum = 0;
    
    /* Make pointers potentially alias */
    ptr1 = arr + (n / 2);
    ptr2 = brr + (n / 2) - 1;
    
    for (int i = 0; i < n; i++) {
        /* Memory operations with potential aliasing */
        *ptr1 = arr[i] + 1;
        *ptr2 = brr[i] * 2;
        
        /* Another potentially aliasing access */
        arr[i] = *ptr2 + g_volatile;
        brr[i] = *ptr1 - i;
        
        /* Complex addressing to confuse analyzer */
        int idx = (i * 7) % n;
        sum += arr[idx] + brr[n - idx - 1];
        
        /* Modify pointers to create varying aliasing */
        if (i % 3 == 0) {
            ptr1 = arr + ((i + 1) % n);
        }
    }
    
    return sum;
}

/* Test 4: Control Dependencies */
int test_control_dependencies(int *arr, int n, int threshold) {
    int count = 0;
    float fsum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Control-dependent computations */
        if (arr[i] > threshold) {
            /* Branch creates control dependencies */
            arr[i] = arr[i] * 2;
            count++;
            fsum += (float)arr[i];
        } else if (arr[i] < -threshold) {
            /* Another control-dependent path */
            arr[i] = arr[i] / 2;
            fsum -= (float)arr[i];
        } else {
            /* Default path */
            arr[i] = arr[i] + g_volatile;
        }
        
        /* Nested control flow */
        for (int j = 0; j < 3; j++) {
            if ((i + j) % 5 == 0) {
                arr[i] += j;
            }
        }
        
        /* Prevent if-conversion */
        asm volatile("" : : : "memory");
    }
    
    return count + (int)fsum;
}

/* Test 5: Mixed Data Types and Dependencies */
int test_mixed_dependencies(float *farr, int *iarr, int n) {
    float fsum = 0.0f;
    int isum = 0;
    
    for (int i = 1; i < n; i++) {
        /* Mixed float/int dependencies */
        float f1 = farr[i-1];
        int i1 = iarr[i];
        
        /* Cross-type dependencies */
        farr[i] = f1 * (float)i1 + (float)g_volatile;
        iarr[i] = (int)farr[i] + iarr[i-1];
        
        /* More complex mixed operations */
        fsum += farr[i] * 0.5f;
        isum += iarr[i] & 0xFF;
        
        /* Dependency through function call (memory clobber) */
        if (i % 7 == 0) {
            /* Inline asm acts as memory barrier */
            asm volatile("" : : : "memory");
        }
    }
    
    return isum + (int)fsum;
}

/* Test 6: Complex Loop-Carried Dependencies with Distance */
int test_loop_carried_dependencies(int *arr, int n, int distance) {
    int sum = 0;
    
    /* Multiple loop-carried dependencies with different distances */
    for (int i = distance; i < n; i++) {
        /* Distance = 1 */
        arr[i] = arr[i-1] + 1;
        
        /* Distance = 2 */
        if (i >= 2) {
            arr[i] += arr[i-2];
        }
        
        /* Distance = parameter */
        if (i >= distance) {
            arr[i] ^= arr[i-distance];
        }
        
        /* Non-linear access pattern */
        int idx = (i * 3) % n;
        sum += arr[idx];
        
        /* Volatile access creates hard dependency */
        arr[i] += g_volatile;
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
    float *farr = (float*)aligned_alloc(64, n * sizeof(float));
    
    /* Initialize with non-constant data */
    for (int i = 0; i < n; i++) {
        arr1[i] = (i * 3) % 97;
        arr2[i] = (i * 7) % 101;
        farr[i] = (float)(i % 17) * 0.1f;
        g_global_array[i & 1023] = i;
    }
    
    int total = 0;
    
    /* Run all tests multiple times to ensure execution */
    for (int iter = 0; iter < 3; iter++) {
        g_volatile = iter;  /* Change volatile to affect dependencies */
        
        total += test_raw_dependencies(arr1, n, iter);
        total += test_war_waw_dependencies(arr1, arr2, n);
        total += test_memory_aliasing(arr1, arr2, n, arr1 + 10, arr2 + 5);
        total += test_control_dependencies(arr1, n, 50);
        total += test_mixed_dependencies(farr, arr1, n);
        total += test_loop_carried_dependencies(arr2, n, 3);
        
        /* Alternate distance parameter */
        total += test_loop_carried_dependencies(arr1, n, 4 + (iter % 3));
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result checksum: %d\n", total);
    
    /* Store to global to ensure side effects */
    g_result = total;
    
    free(arr1);
    free(arr2);
    free(farr);
    
    return total != 0 ? 0 : 1;
}
