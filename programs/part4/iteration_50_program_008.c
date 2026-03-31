/* test_ddg_coverage.c
 * Designed to trigger DDG edge creation in GCC's instruction scheduler
 * Compile with: gcc -O2 -fmodulo-sched -funroll-loops -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_array[1024];
int g_result = 0;

/* Function prototypes */
int test_raw_dep(int *a, int *b, int n);
int test_war_waw_dep(int *a, int *b, int n);
int test_memory_aliasing(int *a, int *b, int *c, int n);
int test_control_dep(int *a, int *b, int n);
int test_mixed_deps(int *a, int *b, float *fa, float *fb, int n);
int test_loop_carried(int *a, int n);
int test_nested_loops(int *a, int *b, int n, int m);

/* Test 1: True Data Dependencies (RAW) with loop-carried dependency */
int test_raw_dep(int *a, int *b, int n) {
    int sum = 0;
    
    /* Loop with RAW dependencies and distance > 0 */
    for (int i = 2; i < n; i++) {
        /* Flow dependency: a[i] depends on a[i-1] and a[i-2] */
        a[i] = a[i-1] + a[i-2] + b[i];
        
        /* Another RAW chain with different distance */
        b[i] = a[i-3] * 2 + g_volatile;
        
        /* Accumulate to prevent elimination */
        sum += a[i] + b[i];
    }
    
    return sum;
}

/* Test 2: Anti and Output Dependencies (WAR/WAW) */
int test_war_waw_dep(int *a, int *b, int n) {
    int temp, sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* WAR (anti-dependency): b[i] reads a[i] before it's overwritten */
        temp = a[i] + 1;
        b[i] = temp * 2;
        
        /* WAW (output-dependency): a[i] written twice */
        a[i] = b[i] + g_volatile;
        a[i] = a[i] * 3;  // Second write to same location
        
        /* Create register pressure to force spills */
        int t1 = a[i] + b[i];
        int t2 = t1 * 2;
        int t3 = t2 - b[i];
        
        sum += t3;
    }
    
    return sum;
}

/* Test 3: Memory aliasing with pointers */
int test_memory_aliasing(int *a, int *b, int *c, int n) {
    int sum = 0;
    int *p = a;
    int *q = b;
    
    /* Use pointers that may alias */
    for (int i = 0; i < n; i++) {
        /* Memory dependencies with potential aliasing */
        *p = *q + i;
        p = &a[(i + 1) % n];  /* Change pointer each iteration */
        q = &b[(i + 2) % n];  /* Different stride for aliasing confusion */
        
        /* Access through array with non-linear index */
        c[i * 2 % n] = a[i] + b[i];
        
        /* Volatile access creates hard memory dependency */
        sum += g_volatile + *p;
    }
    
    return sum;
}

/* Test 4: Control dependencies with branching */
int test_control_dep(int *a, int *b, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Control-dependent computation */
        if (a[i] > 0) {
            b[i] = a[i] * 2 + g_volatile;
            
            /* Nested condition for more complex CFG */
            if (b[i] < 100) {
                a[i] = b[i] / 3;
            } else {
                a[i] = b[i] % 7;
            }
        } else {
            b[i] = a[i] - 5;
            a[i] = b[i] * b[i];
        }
        
        /* Loop with break/continue for additional control flow */
        if (i % 7 == 0) {
            sum += a[i] * 3;
            continue;
        }
        
        if (i % 13 == 0) {
            sum -= b[i];
            /* Early exit from loop body */
            if (sum > 1000) break;
        }
        
        sum += a[i] + b[i];
    }
    
    return sum;
}

/* Test 5: Mixed data types and operations */
int test_mixed_deps(int *a, int *b, float *fa, float *fb, int n) {
    float fsum = 0.0f;
    int isum = 0;
    
    for (int i = 1; i < n; i++) {
        /* Integer RAW with distance 2 */
        a[i] = a[i-2] * 3 + b[i];
        
        /* Floating point RAW */
        fa[i] = fa[i-1] + fb[i] * 2.5f;
        
        /* Mixed type operations causing conversions */
        b[i] = (int)fa[i] + a[i];
        
        /* Floating point control dependency */
        if (fa[i] > 100.0f) {
            fb[i] = fa[i] / 2.0f;
        } else {
            fb[i] = fa[i] * 3.0f;
        }
        
        /* Accumulate with different types */
        fsum += fa[i] + fb[i];
        isum += a[i] + b[i];
    }
    
    return isum + (int)fsum;
}

/* Test 6: Complex loop-carried dependencies */
int test_loop_carried(int *a, int n) {
    int sum = 0;
    
    /* Multiple interleaved dependency chains */
    for (int i = 4; i < n; i++) {
        /* Chain 1: distance 1 */
        int t1 = a[i-1] + i;
        
        /* Chain 2: distance 2 */
        int t2 = a[i-2] * 2 + g_volatile;
        
        /* Chain 3: distance 3 */
        int t3 = a[i-3] - i * 3;
        
        /* Chain 4: distance 4 */
        int t4 = a[i-4] % 17;
        
        /* Combine chains with different latencies */
        a[i] = t1 + t2 + t3 + t4;
        
        /* Create output dependency */
        a[i] = a[i] * 2;  // WAW on a[i]
        
        /* Anti-dependency through temporary */
        int temp = a[i];
        a[i] = i * 7;
        sum += temp + a[i];
    }
    
    return sum;
}

/* Test 7: Nested loops for complex DDG */
int test_nested_loops(int *a, int *b, int n, int m) {
    int sum = 0;
    
    /* Outer loop */
    for (int i = 1; i < n; i++) {
        /* Inner loop with dependencies */
        for (int j = 1; j < m; j++) {
            /* Cross-iteration dependencies in inner loop */
            b[j] = b[j-1] + a[i] + g_volatile;
            
            /* Dependency on outer loop index */
            a[i] += b[j] * (i + j);
            
            /* Memory dependency with outer array */
            g_array[j % 1024] = a[i] % 256;
            
            sum += b[j];
        }
        
        /* Inter-iteration dependency in outer loop */
        a[i] = a[i-1] + sum % 100;
    }
    
    return sum;
}

/* Main function that runs all tests */
int main(int argc, char **argv) {
    int n = 1000;
    int m = 100;
    int result = 0;
    
    /* Allocate arrays with dynamic size to prevent static analysis */
    int *a1 = (int*)malloc(n * sizeof(int));
    int *b1 = (int*)malloc(n * sizeof(int));
    int *c1 = (int*)malloc(n * sizeof(int));
    float *fa1 = (float*)malloc(n * sizeof(float));
    float *fb1 = (float*)malloc(n * sizeof(float));
    
    /* Initialize with non-constant values */
    for (int i = 0; i < n; i++) {
        a1[i] = (i * 3) % 97;
        b1[i] = (i * 7) % 113;
        c1[i] = (i * 11) % 151;
        fa1[i] = (float)(i % 89) * 1.5f;
        fb1[i] = (float)(i % 73) * 2.5f;
    }
    
    /* Run all tests to create various DDG edges */
    result += test_raw_dep(a1, b1, n);
    result += test_war_waw_dep(a1, b1, n);
    result += test_memory_aliasing(a1, b1, c1, n);
    result += test_control_dep(a1, b1, n);
    result += test_mixed_deps(a1, b1, fa1, fb1, n);
    result += test_loop_carried(a1, n);
    result += test_nested_loops(a1, b1, n, m);
    
    /* Use result to prevent dead code elimination */
    printf("Result checksum: %d\n", result);
    
    /* Store to global to create memory side effects */
    g_result = result;
    
    free(a1);
    free(b1);
    free(c1);
    free(fa1);
    free(fb1);
    
    return g_result > 0 ? 0 : 1;
}
