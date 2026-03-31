/* test_ddg_coverage.c
 * Designed to trigger DDG edge creation in GCC's scheduler
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
int test_raw_dep(int *a, int *b, int n);
int test_war_waw_dep(int *a, int *b, int *c, int n);
int test_memory_aliasing(int *arr1, int *arr2, int *ptr1, int *ptr2, int n);
int test_control_dep(int *a, int *b, int n);
int test_mixed_deps(float *fa, int *ia, double *da, int n);
int test_nested_loops(int *a, int *b, int *c, int n);

/* Test 1: True Data Dependencies (RAW) with loop-carried dependencies */
int test_raw_dep(int *a, int *b, int n) {
    int sum = 0;
    
    /* Multiple RAW dependencies with different distances */
    for (int i = 2; i < n; i++) {
        a[i] = a[i-1] + b[i];          /* Distance 1 RAW */
        a[i] += a[i-2] * 2;            /* Distance 2 RAW */
        b[i] = a[i] + g_volatile;      /* Volatile prevents elimination */
    }
    
    /* Another loop with floating point RAW */
    float temp = 0.0f;
    for (int i = 1; i < n; i++) {
        temp = temp * 1.5f + a[i];     /* Loop-carried RAW */
        sum += (int)temp;
    }
    
    return sum;
}

/* Test 2: Anti (WAR) and Output (WAW) Dependencies */
int test_war_waw_dep(int *a, int *b, int *c, int n) {
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        int t1 = a[i] + b[i];          /* Read a[i], b[i] */
        a[i] = t1 * 2;                 /* WAR: Write a[i] after read */
        int t2 = a[i] + c[i];          /* Read a[i] again */
        
        /* WAW: Multiple writes to same location */
        b[i] = t1 + t2;
        b[i] = b[i] * 3;               /* WAW on b[i] */
        
        /* More complex WAR */
        c[i] = a[i] + b[i];            /* Read a[i], b[i] */
        a[i] = c[i] / 2;               /* WAR on a[i] */
        
        result += b[i] + c[i];
    }
    
    return result;
}

/* Test 3: Memory Aliasing Dependencies */
int test_memory_aliasing(int *arr1, int *arr2, int *ptr1, int *ptr2, int n) {
    int sum = 0;
    
    /* Pointers that may alias */
    int *p = ptr1;
    int *q = ptr2;
    
    for (int i = 0; i < n; i++) {
        /* Memory operations with potential aliasing */
        *p = arr1[i] + arr2[i];
        *q = *p * 2;                    /* May depend on previous store if p == q */
        
        /* Array accesses with variable indices - may alias */
        arr1[i] = arr2[(i + 1) % n] + 1;
        arr2[i] = arr1[(i + 2) % n] * 2;
        
        /* Update pointers - creates complex memory dependencies */
        p = &arr1[(i + 3) % n];
        q = &arr2[(i + 4) % n];
        
        sum += arr1[i] + arr2[i];
    }
    
    return sum;
}

/* Test 4: Control Dependencies */
int test_control_dep(int *a, int *b, int n) {
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        /* Branch creates control dependencies */
        if (a[i] > 0) {
            b[i] = a[i] * 2;
            total += b[i];
        } else {
            b[i] = a[i] / 2;
            total -= b[i];
        }
        
        /* Nested conditional */
        if (i % 3 == 0) {
            a[i] = total + g_volatile;
        } else if (i % 3 == 1) {
            b[i] = total * 2;
        } else {
            a[i] = b[i] + 1;
        }
        
        /* Loop with break condition */
        if (total > 1000000) {
            break;
        }
    }
    
    return total;
}

/* Test 5: Mixed Data Types and Dependencies */
int test_mixed_deps(float *fa, int *ia, double *da, int n) {
    double sum = 0.0;
    
    for (int i = 1; i < n; i++) {
        /* Mixed type dependencies */
        fa[i] = fa[i-1] * 1.1f;                /* Float RAW */
        ia[i] = (int)fa[i] + ia[i-1];          /* Int RAW with type conversion */
        da[i] = (double)ia[i] * 0.5;           /* Double dependency */
        
        /* Cross-type dependencies */
        fa[i] += (float)da[i-1];
        ia[i] += (int)fa[i];
        
        sum += da[i] + fa[i] + ia[i];
    }
    
    /* Prevent dead code elimination */
    g_volatile = (int)sum;
    return (int)sum;
}

/* Test 6: Nested Loops with Complex Dependencies */
int test_nested_loops(int *a, int *b, int *c, int n) {
    int result = 0;
    
    /* Outer loop with carried dependency */
    for (int i = 1; i < n; i++) {
        int outer_acc = a[i-1];
        
        /* Inner loop with multiple dependency types */
        for (int j = 0; j < 10; j++) {
            /* RAW within inner loop */
            b[j] = b[j] + outer_acc;
            
            /* WAR in inner loop */
            int temp = c[j];
            c[j] = b[j] * 2;
            result += temp;
            
            /* WAW in inner loop */
            a[i] = j * 3;
            a[i] = a[i] + 1;
        }
        
        /* Loop-carried dependency across outer iterations */
        a[i] = a[i] + outer_acc + g_volatile;
        result += a[i];
    }
    
    return result;
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
    int *a1 = (int*)malloc(n * sizeof(int) + 16);
    int *b1 = (int*)malloc(n * sizeof(int) + 16);
    int *c1 = (int*)malloc(n * sizeof(int) + 16);
    float *fa = (float*)malloc(n * sizeof(float) + 16);
    double *da = (double*)malloc(n * sizeof(double) + 16);
    
    /* Initialize data */
    for (int i = 0; i < n; i++) {
        a1[i] = (i * 3) % 97;
        b1[i] = (i * 7) % 101;
        c1[i] = (i * 11) % 103;
        fa[i] = (float)(i % 17) * 0.5f;
        da[i] = (double)(i % 23) * 0.25;
        g_global_array[i % 1024] = i;
    }
    
    int total_result = 0;
    
    /* Run all tests multiple times to ensure execution */
    for (int iter = 0; iter < 3; iter++) {
        total_result += test_raw_dep(a1, b1, n);
        total_result += test_war_waw_dep(a1, b1, c1, n);
        total_result += test_memory_aliasing(a1, b1, &a1[n/2], &b1[n/3], n);
        total_result += test_control_dep(a1, b1, n);
        total_result += test_mixed_deps(fa, a1, da, n);
        total_result += test_nested_loops(a1, b1, c1, n);
        
        /* Modify inputs slightly each iteration */
        a1[iter % n] += total_result;
        g_volatile = iter;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Final checksum: %d\n", total_result % 1000000);
    
    /* Cleanup */
    free(a1);
    free(b1);
    free(c1);
    free(fa);
    free(da);
    
    return total_result % 256;
}
