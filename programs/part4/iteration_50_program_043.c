/* test_ddg_coverage.c
 * Program designed to trigger DDG edge creation in GCC's scheduler
 * Compile with: gcc -O2 -funroll-loops -fmodulo-sched -c test_ddg_coverage.c
 * Or: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -ftree-vectorize -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_array[1024];
int g_result = 0;

/* Function prototypes */
int test_raw_dep(int *a, int *b, int n);
int test_war_waw_dep(int *a, int *b, int *c, int n);
int test_memory_aliasing(int *arr1, int *arr2, int n);
int test_control_dep(int *data, int n);
int test_mixed_deps(float *fa, int *ia, int n);
int test_nested_loops(int *a, int *b, int n);
int test_loop_carried_dep(int *a, int n);

/* Test 1: True Data Dependencies (RAW) with loop-carried dependency */
int test_raw_dep(int *a, int *b, int n) {
    int sum = 0;
    
    /* Multiple RAW dependencies with different distances */
    for (int i = 2; i < n; i++) {
        /* Distance 1 RAW dependency */
        a[i] = a[i-1] + b[i];
        
        /* Distance 2 RAW dependency */
        b[i] = a[i-2] * 3;
        
        /* Floating point RAW to create different data type edges */
        float temp = (float)a[i] / 2.0f;
        sum += (int)temp;
        
        /* Another RAW chain */
        a[i-1] = b[i] + g_volatile;
    }
    
    return sum;
}

/* Test 2: Anti (WAR) and Output (WAW) Dependencies */
int test_war_waw_dep(int *a, int *b, int *c, int n) {
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        /* WAR (anti-dependency): read a[i], then write to it */
        int temp = a[i] + c[i];
        a[i] = b[i] * 2;  // This creates WAR with previous read
        
        /* WAW (output-dependency): multiple writes to same location */
        b[i] = temp + i;
        b[i] = b[i] * 3;  // WAW dependency
        
        /* More complex WAR chain */
        c[i] = a[i] + b[i];
        a[i] = c[i] - i;  // Another WAR
        
        result += b[i];
    }
    
    return result;
}

/* Test 3: Memory Aliasing Dependencies */
int test_memory_aliasing(int *arr1, int *arr2, int n) {
    /* Use pointers that may alias */
    int *p = arr1;
    int *q = arr2;
    
    /* q might point to arr1 + offset, creating potential aliasing */
    if (n > 10) {
        q = arr1 + 5;
    }
    
    int sum = 0;
    for (int i = 1; i < n-1; i++) {
        /* Memory operations that may alias */
        p[i] = q[i-1] + g_volatile;
        q[i] = p[i+1] * 2;
        
        /* Additional pointer arithmetic */
        *(p + i) = *(q + i) + i;
        
        sum += p[i];
    }
    
    return sum;
}

/* Test 4: Control Dependencies */
int test_control_dep(int *data, int n) {
    int count = 0;
    
    for (int i = 0; i < n; i++) {
        /* Branch creates control dependencies */
        if (data[i] > 0) {
            data[i] = data[i] * 2;
            count++;
        } else if (data[i] < -10) {
            data[i] = data[i] / 2;
            count--;
        } else {
            data[i] = g_volatile;
        }
        
        /* Nested condition */
        if (count > 100) {
            data[i] = 0;
        }
        
        /* Another dependent operation */
        g_array[i % 1024] = data[i] + count;
    }
    
    return count;
}

/* Test 5: Mixed Data Types and Dependencies */
int test_mixed_deps(float *fa, int *ia, int n) {
    float fsum = 0.0f;
    int isum = 0;
    
    for (int i = 1; i < n; i++) {
        /* Mixed float/int operations */
        fa[i] = fa[i-1] * 1.5f + (float)ia[i];
        
        /* Integer RAW */
        ia[i] = ia[i-1] + (int)fa[i];
        
        /* Float WAR */
        float temp = fa[i];
        fa[i] = temp * 2.0f;
        
        /* Integer WAW */
        ia[i] = ia[i] * 3;
        ia[i] = ia[i] + 1;  // WAW
        
        fsum += fa[i];
        isum += ia[i];
    }
    
    return isum + (int)fsum;
}

/* Test 6: Nested Loops */
int test_nested_loops(int *a, int *b, int n) {
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        int inner_sum = 0;
        
        /* Inner loop with dependencies */
        for (int j = 1; j < 10; j++) {
            /* Loop-carried in inner loop */
            a[j] = a[j-1] + b[i];
            
            /* Anti-dependency */
            int temp = b[i];
            b[i] = j * 2;
            inner_sum += temp;
            
            /* Control in inner loop */
            if (j % 2 == 0) {
                a[j] = a[j] * 3;
            }
        }
        
        /* Outer loop dependency */
        b[i] = inner_sum + g_volatile;
        total += b[i];
    }
    
    return total;
}

/* Test 7: Complex Loop-Carried Dependencies */
int test_loop_carried_dep(int *a, int n) {
    /* Multiple loop-carried dependencies with different distances */
    int sum = 0;
    
    for (int i = 4; i < n; i++) {
        /* Distance 3 dependency */
        a[i] = a[i-3] * 2 + g_volatile;
        
        /* Distance 1 with computation */
        a[i-1] = a[i] + a[i-2];
        
        /* Distance 2 with condition */
        if (a[i-2] > 100) {
            a[i] = a[i] / 2;
        }
        
        /* Distance 4 dependency chain */
        int temp = a[i-4] + i;
        a[i-2] = temp * 3;
        
        sum += a[i];
    }
    
    return sum;
}

/* Main function that calls all tests */
int main(int argc, char **argv) {
    /* Use command line argument for iteration count to prevent compile-time optimization */
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 100) n = 100;
        if (n > 10000) n = 10000;
    }
    
    /* Allocate arrays with dynamic size */
    int size = n + 10;
    int *arr1 = (int*)malloc(size * sizeof(int));
    int *arr2 = (int*)malloc(size * sizeof(int));
    int *arr3 = (int*)malloc(size * sizeof(int));
    float *farr = (float*)malloc(size * sizeof(float));
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < size; i++) {
        arr1[i] = (i % 37) + 1;
        arr2[i] = (i % 23) + 1;
        arr3[i] = (i % 47) + 1;
        farr[i] = (float)(i % 19) + 0.5f;
    }
    
    int final_result = 0;
    
    /* Run all tests to trigger different DDG edge types */
    final_result += test_raw_dep(arr1, arr2, n);
    final_result += test_war_waw_dep(arr1, arr2, arr3, n);
    final_result += test_memory_aliasing(arr1, arr2, n);
    final_result += test_control_dep(arr3, n);
    final_result += test_mixed_deps(farr, arr1, n);
    final_result += test_nested_loops(arr2, arr3, n/10);
    final_result += test_loop_carried_dep(arr1, n);
    
    /* Use the result to prevent dead code elimination */
    printf("Final checksum: %d\n", final_result);
    
    /* Store to global to ensure side effects */
    g_result = final_result;
    
    free(arr1);
    free(arr2);
    free(arr3);
    free(farr);
    
    return g_result != 0 ? 0 : 1;
}
