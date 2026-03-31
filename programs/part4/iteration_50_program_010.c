/* test_ddg_edges.c - Program to trigger DDG edge creation in GCC scheduler */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_array[1024];
int g_result = 0;

/* Function prototypes */
int test_raw_dependencies(int *a, int *b, int n);
int test_war_waw_dependencies(int *a, int *b, int n);
int test_memory_aliasing(int *a, int *b, int *c, int n);
int test_control_dependencies(int *a, int *b, int n);
int test_mixed_dependencies(float *fa, int *ia, double *da, int n);
int test_nested_loops(int *a, int *b, int n);
int test_loop_carried_deps(int *a, int n);

/* Test 1: True Data Dependencies (RAW) with loop-carried dependencies */
int test_raw_dependencies(int *a, int *b, int n) {
    int sum = 0;
    
    /* Multiple RAW dependencies with different distances */
    for (int i = 2; i < n; i++) {
        /* Distance 1 RAW */
        a[i] = a[i-1] + b[i];
        
        /* Distance 2 RAW */
        b[i] = a[i-2] * 3;
        
        /* Distance 3 RAW with floating point */
        float temp = (float)a[i-3] / 2.0f;
        sum += (int)temp;
        
        /* Complex RAW chain */
        int t1 = a[i-1] + 1;
        int t2 = t1 * b[i-1];
        int t3 = t2 - a[i-2];
        a[i] = t3 + g_volatile;
    }
    
    return sum;
}

/* Test 2: Anti (WAR) and Output (WAW) Dependencies */
int test_war_waw_dependencies(int *a, int *b, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* WAR dependency: read a[i] before writing it */
        int temp = a[i] + b[i];
        
        /* WAW dependency: multiple writes to same location */
        a[i] = temp * 2;
        a[i] = a[i] + 1;  // WAW with previous write
        
        /* More complex WAR */
        b[i] = a[i] * 3;  // WAR: a[i] was written above
        
        /* Output dependency chain */
        int x = i * 2;
        x = x + temp;     // WAW on x
        x = x - b[i];     // Another WAW on x
        
        sum += x;
    }
    
    return sum;
}

/* Test 3: Memory Aliasing with pointers */
int test_memory_aliasing(int *a, int *b, int *c, int n) {
    int sum = 0;
    
    /* Create ambiguous pointer aliasing */
    int *p = a;
    int *q = b;
    
    /* Volatile to prevent reordering */
    volatile int *vp = &g_volatile;
    
    for (int i = 1; i < n - 1; i++) {
        /* Potential aliasing through pointer arithmetic */
        p = a + i;
        q = b + (i % 5);
        
        /* Memory dependencies with possible aliasing */
        *p = *q + c[i];
        *q = *p - 2;
        
        /* More complex aliasing pattern */
        *(p + 1) = *(q - 1) * 3;
        *(a + i) = *(b + i) + *(c + i);
        
        /* Volatile access creates hard dependency */
        sum += *vp;
        
        /* Pointer chasing */
        int *r = c + i;
        *r = *r + sum;
    }
    
    return sum;
}

/* Test 4: Control Dependencies with branching */
int test_control_dependencies(int *a, int *b, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Control-dependent computations */
        if (a[i] > 0) {
            b[i] = a[i] * 2;
            sum += b[i];
        } else if (a[i] < 0) {
            b[i] = a[i] / 2;
            sum -= b[i];
        } else {
            b[i] = g_volatile;
            sum += 1;
        }
        
        /* Nested conditional with dependencies */
        int temp = b[i];
        if (temp % 2 == 0) {
            a[i] = temp + 1;
        } else {
            a[i] = temp - 1;
        }
        
        /* Loop with break condition (creates control flow) */
        if (sum > 1000) {
            sum = sum % 1000;
        }
    }
    
    return sum;
}

/* Test 5: Mixed data types and operations */
int test_mixed_dependencies(float *fa, int *ia, double *da, int n) {
    float fsum = 0.0f;
    int isum = 0;
    double dsum = 0.0;
    
    for (int i = 3; i < n; i++) {
        /* Mixed type RAW dependencies */
        fa[i] = fa[i-1] + (float)ia[i-2];
        ia[i] = (int)fa[i-1] * 2;
        da[i] = da[i-3] + (double)ia[i-1];
        
        /* Type conversions create dependencies */
        float temp_f = (float)ia[i] / 3.14f;
        int temp_i = (int)da[i];
        double temp_d = (double)fa[i];
        
        /* Cross-type dependencies */
        fa[i] = temp_f + (float)temp_d;
        ia[i] = temp_i * (int)temp_f;
        da[i] = temp_d - (double)temp_i;
        
        /* Accumulate results */
        fsum += fa[i];
        isum += ia[i];
        dsum += da[i];
    }
    
    return (int)(fsum + isum + dsum);
}

/* Test 6: Nested loops with dependencies */
int test_nested_loops(int *a, int *b, int n) {
    int sum = 0;
    
    for (int i = 1; i < n; i++) {
        /* Outer loop carried dependency */
        a[i] = a[i-1] + i;
        
        for (int j = 0; j < 8; j++) {  /* Fixed inner loop to prevent unrolling */
            /* Inner loop dependencies */
            b[j] = a[i] + j;
            
            /* Dependency across inner iterations */
            if (j > 0) {
                b[j] = b[j-1] * 2;
            }
            
            /* Memory dependency with outer loop */
            a[i] = a[i] + b[j];
            
            sum += b[j];
        }
        
        /* Dependency after inner loop */
        a[i] = a[i] + sum % 256;
    }
    
    return sum;
}

/* Test 7: Complex loop-carried dependencies */
int test_loop_carried_deps(int *a, int n) {
    int sum = 0;
    
    /* Multiple interleaved dependency chains */
    for (int i = 4; i < n; i++) {
        /* Chain 1: distance 1 */
        int x1 = a[i-1] + 1;
        
        /* Chain 2: distance 2 */
        int x2 = a[i-2] * 2;
        
        /* Chain 3: distance 3 */
        int x3 = a[i-3] - 3;
        
        /* Chain 4: distance 4 */
        int x4 = a[i-4] / 4;
        
        /* Cross-chain dependencies */
        a[i] = x1 + x2;
        int y1 = a[i] + x3;
        int y2 = y1 * x4;
        
        /* Recurrence with feedback */
        a[i] = (a[i] + y2) % 1000;
        
        /* Volatile to prevent optimization */
        sum += a[i] + g_volatile;
        
        /* Additional dependency to extend live ranges */
        for (int j = 0; j < 2; j++) {
            a[i] = a[i] + j + sum;
        }
    }
    
    return sum;
}

/* Main function that runs all tests */
int main(int argc, char **argv) {
    const int N = 1000;
    int total_sum = 0;
    
    /* Initialize arrays with non-constant values */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    float *fa = (float*)malloc(N * sizeof(float));
    double *da = (double*)malloc(N * sizeof(double));
    
    /* Initialize with pseudo-random values to prevent constant propagation */
    for (int i = 0; i < N; i++) {
        a[i] = (i * 13) % 97;
        b[i] = (i * 17) % 101;
        c[i] = (i * 19) % 103;
        fa[i] = (float)((i * 23) % 107);
        da[i] = (double)((i * 29) % 109);
    }
    
    /* Run all tests to create various DDG edge types */
    total_sum += test_raw_dependencies(a, b, N);
    total_sum += test_war_waw_dependencies(a, b, N);
    total_sum += test_memory_aliasing(a, b, c, N);
    total_sum += test_control_dependencies(a, b, N);
    total_sum += test_mixed_dependencies(fa, a, da, N);
    total_sum += test_nested_loops(a, b, N);
    total_sum += test_loop_carried_deps(a, N);
    
    /* Store to global to prevent dead code elimination */
    g_result = total_sum;
    
    /* Print result to ensure code isn't optimized away */
    printf("Result: %d\n", total_sum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(fa);
    free(da);
    
    return 0;
}
