/* test_ddg_coverage.c
 * Program designed to trigger DDG edge creation in GCC's scheduler
 * Compile with: gcc -O2 -fmodulo-sched -funroll-loops -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to prevent optimizations */
volatile int g_volatile = 0;
int g_array[1024];
int g_result = 0;

/* Function prototypes */
int test_raw_dependencies(int *a, int *b, int n);
int test_war_waw_dependencies(int *a, int *b, int n);
int test_memory_aliasing(int *a, int *b, int *c, int n);
int test_control_dependencies(int *a, int *b, int n);
int test_mixed_dependencies(float *fa, int *ia, double *da, int n);
int test_loop_carried_dependencies(int *a, int *b, int n);

/* Test 1: True Data Dependencies (RAW) with loop-carried dependencies */
int test_raw_dependencies(int *a, int *b, int n) {
    int sum = 0;
    
    /* Multiple RAW dependencies with different distances */
    for (int i = 2; i < n; i++) {
        /* Distance 1 RAW dependency */
        a[i] = a[i-1] + b[i];
        
        /* Distance 2 RAW dependency */
        b[i] = a[i-2] * 3;
        
        /* Distance 3 RAW dependency with floating point */
        float temp = (float)a[i-3] * 1.5f;
        sum += (int)temp;
        
        /* Volatile access to prevent reordering */
        g_volatile = i;
    }
    
    return sum;
}

/* Test 2: Anti (WAR) and Output (WAW) Dependencies */
int test_war_waw_dependencies(int *a, int *b, int n) {
    int sum = 0;
    
    for (int i = 1; i < n; i++) {
        /* WAR (anti-dependency): read a[i] before writing it */
        int temp = a[i] + b[i];
        
        /* WAW (output-dependency): multiple writes to same location */
        a[i] = temp * 2;
        a[i] = a[i] + 1;  /* Second write creates WAW */
        
        /* Another WAR with b[i] */
        b[i] = temp - a[i-1];
        
        /* Complex WAW chain */
        float f = (float)i * 0.5f;
        f = f * 2.0f;  /* WAW on floating point */
        f = f + 1.0f;  /* Another WAW */
        
        sum += (int)f + a[i];
    }
    
    return sum;
}

/* Test 3: Memory Aliasing Dependencies */
int test_memory_aliasing(int *a, int *b, int *c, int n) {
    int sum = 0;
    
    /* Use pointers that may alias */
    int *p = a;
    int *q = b;
    
    for (int i = 0; i < n; i++) {
        /* Pointer accesses that may alias */
        *p = *q + i;
        p = &a[(i + 1) % n];  /* Change pointer target */
        q = &b[(i + 2) % n];  /* Change pointer target */
        
        /* Array accesses with variable indices - may alias */
        a[(i * 3) % n] = b[(i * 5) % n] + c[i];
        
        /* Memory clobber via function call simulation */
        asm volatile("" : : "r"(a), "r"(b) : "memory");
        
        sum += a[i % n];
    }
    
    return sum;
}

/* Test 4: Control Dependencies */
int test_control_dependencies(int *a, int *b, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Control-dependent computations */
        if (i % 3 == 0) {
            a[i] = b[i] * 2;
            sum += a[i];
        } else if (i % 3 == 1) {
            a[i] = b[i] / 2;
            sum -= a[i];
        } else {
            a[i] = b[i] + g_volatile;
            sum ^= a[i];
        }
        
        /* Nested control flow */
        for (int j = 0; j < 3; j++) {
            if ((i + j) % 2 == 0) {
                b[i] += j;
            } else {
                b[i] -= j;
            }
        }
        
        /* Volatile read creates memory dependency */
        g_volatile = sum % 256;
    }
    
    return sum;
}

/* Test 5: Mixed Data Types and Operations */
int test_mixed_dependencies(float *fa, int *ia, double *da, int n) {
    float fsum = 0.0f;
    int isum = 0;
    double dsum = 0.0;
    
    for (int i = 1; i < n; i++) {
        /* Mixed type RAW dependencies */
        fa[i] = fa[i-1] * 1.1f + (float)ia[i];
        
        /* Integer to float conversion dependency */
        ia[i] = (int)fa[i] + ia[i-1];
        
        /* Double precision with dependency */
        da[i] = da[i-1] * 1.01 + (double)fa[i];
        
        /* Cross-type dependencies */
        fsum += fa[i] * 0.5f;
        isum += ia[i] ^ (i * 7);
        dsum += da[i];
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    
    return isum + (int)fsum + (int)dsum;
}

/* Test 6: Complex Loop-Carried Dependencies */
int test_loop_carried_dependencies(int *a, int *b, int n) {
    int sum = 0;
    
    /* Multiple interleaved loop-carried dependencies */
    for (int i = 4; i < n; i++) {
        /* Dependency chain 1: distance 1, 2, and 4 */
        a[i] = a[i-1] + a[i-2] + a[i-4];
        
        /* Dependency chain 2: with computation */
        b[i] = b[i-3] * 2 - b[i-1];
        
        /* Cross dependency between arrays */
        a[i] = a[i] ^ b[i-2];
        b[i] = b[i] & a[i-1];
        
        /* Reduction with dependency */
        sum = sum + a[i] - b[i-1];
        
        /* Conditional with carried dependency */
        if (sum > 1000) {
            a[i] = a[i] / 2;
            sum = sum % 1000;
        }
        
        /* Volatile write to prevent reordering */
        g_array[i % 1024] = sum;
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
    int *a = (int*)aligned_alloc(64, n * sizeof(int));
    int *b = (int*)aligned_alloc(64, n * sizeof(int));
    int *c = (int*)aligned_alloc(64, n * sizeof(int));
    float *fa = (float*)aligned_alloc(64, n * sizeof(float));
    double *da = (double*)aligned_alloc(64, n * sizeof(double));
    
    /* Initialize arrays with non-zero values */
    for (int i = 0; i < n; i++) {
        a[i] = (i * 3) % 97;
        b[i] = (i * 5) % 113;
        c[i] = (i * 7) % 151;
        fa[i] = (float)(i % 89) * 0.7f;
        da[i] = (double)(i % 73) * 1.3;
    }
    
    int total_result = 0;
    
    /* Run all tests multiple times to increase coverage probability */
    for (int iter = 0; iter < 3; iter++) {
        total_result ^= test_raw_dependencies(a, b, n);
        total_result += test_war_waw_dependencies(a, b, n);
        total_result ^= test_memory_aliasing(a, b, c, n);
        total_result += test_control_dependencies(a, b, n);
        total_result ^= test_mixed_dependencies(fa, a, da, n);
        total_result += test_loop_carried_dependencies(a, b, n);
        
        /* Modify inputs slightly each iteration */
        a[iter % n] = total_result;
        b[iter % n] = total_result ^ 0x5555;
    }
    
    /* Store final result to prevent dead code elimination */
    g_result = total_result;
    
    /* Print result to ensure code isn't optimized away */
    printf("Result: %d\n", total_result);
    
    /* Free allocated memory */
    free(a);
    free(b);
    free(c);
    free(fa);
    free(da);
    
    return total_result != 0 ? 0 : 1;
}
