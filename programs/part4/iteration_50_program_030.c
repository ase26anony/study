/* test_ddg_coverage.c
 * Designed to trigger DDG edge creation in GCC's scheduler
 * Compile with: gcc -O2 -fmodulo-sched -funroll-loops -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_array[1024];
int g_result = 0;

/* Function prototypes */
int test_raw_dep(int *a, int *b, int n);
int test_war_waw_dep(int *a, int *b, int n);
int test_memory_aliasing(int *a, int *b, int *c, int n);
int test_control_dep(int *a, int *b, int n);
int test_mixed_deps(float *fa, int *ia, int n);
int test_loop_carried(int *a, int n, int distance);

/* 1. Test with true data dependencies (RAW) */
int test_raw_dep(int *a, int *b, int n) {
    int sum = 0;
    
    /* Loop with flow dependencies across iterations */
    for (int i = 2; i < n; i++) {
        /* RAW dependency: a[i] depends on a[i-1] and a[i-2] */
        a[i] = a[i-1] + a[i-2] + b[i];
        
        /* Another RAW chain with different latency */
        b[i] = a[i] * 3 + g_volatile;
        
        /* Accumulate to prevent dead code elimination */
        sum += a[i] + b[i];
    }
    
    return sum;
}

/* 2. Test with anti (WAR) and output (WAW) dependencies */
int test_war_waw_dep(int *a, int *b, int n) {
    int temp, sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* WAR (anti-dependency): b[i] read before a[i] written */
        temp = b[i] + i;
        
        /* WAW (output-dependency): a[i] written twice */
        a[i] = temp * 2;
        a[i] = a[i] + g_volatile;  /* Second write creates WAW */
        
        /* Another WAR: using a[i] after it was written */
        b[i] = a[i] / 3;
        
        sum += a[i] + b[i] + temp;
    }
    
    return sum;
}

/* 3. Test with memory aliasing dependencies */
int test_memory_aliasing(int *a, int *b, int *c, int n) {
    int *p = a;
    int *q = b;
    int sum = 0;
    
    /* Use pointers that may alias */
    for (int i = 0; i < n; i++) {
        /* Memory dependencies with potential aliasing */
        *p = *q + c[i];
        p = &a[(i + 1) % n];  /* Change pointer each iteration */
        q = &b[(i + 2) % n];  /* Different stride for aliasing confusion */
        
        /* More memory operations */
        c[i] = *p + *q + g_volatile;
        
        sum += *p + *q + c[i];
    }
    
    return sum;
}

/* 4. Test with control dependencies */
int test_control_dep(int *a, int *b, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Control-dependent computations */
        if (a[i] > 0) {
            b[i] = a[i] * 2 + g_volatile;
            
            /* Nested condition for more complex control flow */
            if (b[i] < 1000) {
                a[i] = b[i] / 3;
            } else {
                a[i] = b[i] % 7;
            }
        } else {
            b[i] = a[i] - 5;
            a[i] = b[i] * b[i];
        }
        
        /* Loop-carried dependency across control flow */
        if (i > 0) {
            a[i] += a[i-1] % 17;
        }
        
        sum += a[i] + b[i];
    }
    
    return sum;
}

/* 5. Test with mixed data types (int and float) */
int test_mixed_deps(float *fa, int *ia, int n) {
    float fsum = 0.0f;
    int isum = 0;
    
    for (int i = 1; i < n; i++) {
        /* Mixed-type dependencies */
        fa[i] = fa[i-1] * 1.5f + (float)ia[i];
        
        /* Integer dependency chain */
        ia[i] = (int)fa[i] + ia[i-1] * 2;
        
        /* Cross-type dependency */
        fa[i] += (float)(ia[i] % 10);
        
        fsum += fa[i];
        isum += ia[i];
    }
    
    return isum + (int)fsum;
}

/* 6. Test with explicit loop-carried dependencies at various distances */
int test_loop_carried(int *a, int n, int distance) {
    int sum = 0;
    
    /* Distance > 0 creates loop-carried dependencies */
    for (int i = distance; i < n; i++) {
        /* Dependency at specified distance */
        a[i] = a[i - distance] + i * 3 + g_volatile;
        
        /* Another dependency chain with different distance */
        if (i >= 2) {
            a[i] += a[i-2] % 13;
        }
        
        sum += a[i];
    }
    
    return sum;
}

/* 7. Complex nested loop with multiple dependency types */
int test_nested_loops(int *a, int *b, int n, int m) {
    int sum = 0;
    
    for (int i = 1; i < n; i++) {
        /* Outer loop dependency */
        int outer_acc = a[i-1];
        
        for (int j = 1; j < m; j++) {
            /* Inner loop dependencies */
            b[j] = b[j-1] + outer_acc;
            
            /* Cross-iteration dependency in inner loop */
            a[i] += b[j] * (i + j);
            
            /* Memory dependency with outer loop */
            outer_acc = (outer_acc + b[j]) % 256;
            
            sum += b[j];
        }
        
        /* Loop-carried in outer loop */
        a[i] = a[i] + a[i-1] + sum % 100;
        sum += a[i];
    }
    
    return sum;
}

/* 8. Function calls create memory clobbering dependencies */
static int helper_func(int x, int y) {
    /* Access volatile global to prevent inlining */
    return x * y + g_volatile;
}

int test_with_calls(int *a, int n) {
    int sum = 0;
    
    for (int i = 1; i < n; i++) {
        /* Function call creates memory dependency */
        a[i] = helper_func(a[i-1], i);
        
        /* More operations after call */
        a[i] += a[i] % 7 + g_volatile;
        
        sum += a[i];
    }
    
    return sum;
}

int main(void) {
    const int N = 1000;
    const int M = 100;
    int total_sum = 0;
    
    /* Initialize arrays with non-zero values */
    int *array1 = (int*)malloc(N * sizeof(int));
    int *array2 = (int*)malloc(N * sizeof(int));
    int *array3 = (int*)malloc(N * sizeof(int));
    float *farray = (float*)malloc(N * sizeof(float));
    
    for (int i = 0; i < N; i++) {
        array1[i] = (i * 3) % 97;
        array2[i] = (i * 7) % 113;
        array3[i] = (i * 11) % 151;
        farray[i] = (float)(i % 23) * 1.7f;
    }
    
    /* Call all test functions to create various DDG edges */
    total_sum += test_raw_dep(array1, array2, N);
    total_sum += test_war_waw_dep(array1, array2, N);
    total_sum += test_memory_aliasing(array1, array2, array3, N);
    total_sum += test_control_dep(array1, array2, N);
    total_sum += test_mixed_deps(farray, array1, N);
    total_sum += test_loop_carried(array1, N, 3);
    total_sum += test_nested_loops(array1, array2, M, M);
    total_sum += test_with_calls(array1, N);
    
    /* Store to global to prevent optimization */
    g_result = total_sum;
    
    /* Print result to ensure code isn't eliminated */
    printf("Result: %d\n", total_sum % 1000000);
    
    free(array1);
    free(array2);
    free(array3);
    free(farray);
    
    return 0;
}
