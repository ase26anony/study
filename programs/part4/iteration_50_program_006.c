/* test_ddg_coverage.c
 * Program designed to trigger DDG edge creation in GCC's scheduler
 * Compile with: gcc -O2 -fmodulo-sched -funroll-loops -c test_ddg_coverage.c
 * Or: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -ftree-vectorize -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_global_array[1024];
float g_float_array[1024];

/* Function to create true data dependencies (RAW) */
int test_raw_dep(int *arr, int n, int seed) {
    int sum = seed;
    
    /* Loop with multiple RAW dependencies */
    for (int i = 2; i < n; i++) {
        /* Flow dependency chain: a[i] depends on a[i-1] and a[i-2] */
        arr[i] = arr[i-1] + arr[i-2] + i;
        
        /* Another flow dependency with different distance */
        g_global_array[i] = g_global_array[i-3] + arr[i];
        
        /* Mixed integer/float operations */
        g_float_array[i] = g_float_array[i-1] * 1.5f + arr[i];
        
        sum += arr[i] + (int)g_float_array[i];
    }
    
    /* Prevent dead code elimination */
    g_volatile = sum;
    return sum;
}

/* Function to create anti (WAR) and output (WAW) dependencies */
int test_war_waw_dep(int *arr, float *farr, int n, int seed) {
    int temp = seed;
    float ftemp = seed * 0.5f;
    
    for (int i = 1; i < n; i++) {
        /* Anti-dependency (WAR): read arr[i] before writing it */
        int read_before_write = arr[i] + temp;
        
        /* Output dependency (WAW): multiple writes to same location */
        arr[i] = read_before_write * 2;
        arr[i] = arr[i] + i;  // Second write creates WAW
        
        /* WAR with floating point */
        float fread = farr[i];
        farr[i] = ftemp * 2.0f;
        ftemp = fread + 1.0f;
        
        /* Another WAW pattern */
        g_global_array[i] = i * 3;
        g_global_array[i] = g_global_array[i] + read_before_write;
    }
    
    g_volatile = (int)ftemp;
    return temp + (int)ftemp;
}

/* Function with memory aliasing dependencies */
int test_memory_aliasing(int *arr1, int *arr2, int n, int seed) {
    int *ptr1 = arr1;
    int *ptr2 = arr2;
    
    /* Make pointers potentially alias */
    if (seed % 2 == 0) {
        ptr2 = arr1 + n/2;
    }
    
    int sum = seed;
    
    for (int i = 1; i < n-1; i++) {
        /* Memory operations with potential aliasing */
        ptr1[i] = ptr2[i-1] * 2;
        ptr2[i] = ptr1[i] + i;
        
        /* Additional memory operations with different strides */
        arr1[i*2 % n] = arr2[i*3 % n] + sum;
        
        sum += ptr1[i] - ptr2[i];
    }
    
    /* Volatile access to create hard memory dependency */
    sum += g_volatile;
    
    return sum;
}

/* Function with control dependencies */
int test_control_dep(int *arr, int n, int seed) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Control-dependent computation */
        if (arr[i] > 0) {
            /* Branch creates control dependencies */
            sum += arr[i] * 2;
            
            /* Nested condition for more complex control flow */
            if (i % 3 == 0) {
                arr[i] = sum / 3;
            } else if (i % 3 == 1) {
                arr[i] = sum + g_volatile;
            } else {
                arr[i] = sum - i;
            }
        } else {
            sum -= arr[i];
            arr[i] = i * seed;
        }
        
        /* Loop-carried dependency across control flow */
        g_global_array[i] = g_global_array[i-1] + sum;
    }
    
    return sum;
}

/* Complex nested loop with mixed dependencies */
int test_nested_loops(int *arr, int n, int seed) {
    int sum = seed;
    
    /* Outer loop */
    for (int i = 1; i < n/2; i++) {
        int inner_sum = 0;
        
        /* Inner loop with dependencies */
        for (int j = 0; j < 8; j++) {
            /* RAW dependency across inner iterations */
            inner_sum += arr[i*8 + j - 1] + j;
            
            /* WAW in inner loop */
            arr[i*8 + j] = inner_sum;
            arr[i*8 + j] = arr[i*8 + j] * 2;
            
            /* Memory dependency with outer loop */
            g_float_array[j] = g_float_array[j] + inner_sum * 0.1f;
        }
        
        /* Loop-carried dependency in outer loop */
        sum += inner_sum + g_global_array[i-1];
        g_global_array[i] = sum;
        
        /* Control flow in outer loop */
        if (sum % 7 == 0) {
            g_volatile = inner_sum;
        }
    }
    
    return sum;
}

/* Function with volatile accesses creating hard dependencies */
int test_volatile_deps(int *arr, int n, int seed) {
    volatile int local_volatile = seed;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Volatile read creates memory barrier-like effect */
        int vol_val = local_volatile;
        
        /* Computation dependent on volatile */
        arr[i] = vol_val + i * 3;
        
        /* Volatile write */
        local_volatile = arr[i] % 256;
        
        /* Dependency through volatile global */
        sum += g_volatile + arr[i];
        
        /* Update global volatile occasionally */
        if (i % 16 == 0) {
            g_volatile = sum;
        }
    }
    
    return sum + local_volatile;
}

/* Main function that calls all test patterns */
int main(int argc, char **argv) {
    int n = 1024;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 100) n = 100;
        if (n > 10000) n = 10000;
    }
    
    /* Initialize arrays with non-zero values */
    int *arr1 = (int*)malloc(n * sizeof(int));
    int *arr2 = (int*)malloc(n * sizeof(int));
    float *farr = (float*)malloc(n * sizeof(float));
    
    for (int i = 0; i < n; i++) {
        arr1[i] = (i * 3) % 97;
        arr2[i] = (i * 7) % 113;
        farr[i] = i * 0.7f;
        g_global_array[i] = i;
        g_float_array[i] = i * 1.3f;
    }
    
    int seed = argc > 2 ? atoi(argv[2]) : 12345;
    int total = 0;
    
    /* Call each test function to create different DDG edge types */
    total += test_raw_dep(arr1, n, seed);
    total += test_war_waw_dep(arr2, farr, n, seed + 1);
    total += test_memory_aliasing(arr1, arr2, n, seed + 2);
    total += test_control_dep(arr1, n, seed + 3);
    total += test_nested_loops(arr2, n, seed + 4);
    total += test_volatile_deps(arr1, n, seed + 5);
    
    /* Use results to prevent optimization */
    printf("Result checksum: %d\n", total);
    
    /* Verify array contents were modified */
    int verify = 0;
    for (int i = 0; i < n && i < 10; i++) {
        verify += arr1[i] + arr2[i];
    }
    printf("Array verify: %d\n", verify);
    
    free(arr1);
    free(arr2);
    free(farr);
    
    return total % 256;
}
