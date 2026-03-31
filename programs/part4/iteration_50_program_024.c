/* test_ddg_coverage.c
 * This program creates various loop patterns to trigger DDG edge creation
 * in GCC's instruction scheduler, specifically targeting create_ddg_edge()
 * lines 749-757 in ddg.cc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_array[1024];
int g_result = 0;

/* Function prototypes */
int test_raw_dep(int *a, int *b, int n);
int test_war_waw_dep(int *a, int *b, int *c, int n);
int test_memory_aliasing(int *arr1, int *arr2, int *ptr1, int *ptr2, int n);
int test_control_dep(int *a, int *b, int *c, int n);
int test_mixed_deps(float *fa, int *ia, double *da, int n);
int test_nested_loops(int *a, int *b, int *c, int n);
int test_loop_carried_dep(int *a, int n, int distance);

/* Test 1: True Data Dependencies (RAW) with loop-carried dependencies */
int test_raw_dep(int *a, int *b, int n) {
    int sum = 0;
    
    /* Multiple RAW dependencies with different distances */
    for (int i = 2; i < n; i++) {
        /* Distance 1 RAW */
        a[i] = a[i-1] + b[i] + g_volatile;
        
        /* Distance 2 RAW with floating point */
        float temp = (float)a[i-2] * 1.5f;
        sum += (int)temp;
        
        /* Another RAW chain */
        b[i] = a[i] + i;
    }
    
    /* Additional RAW with different data types */
    for (int i = 1; i < n; i++) {
        double dval = (double)a[i-1] * 0.25;
        sum += (int)dval;
    }
    
    return sum;
}

/* Test 2: Anti (WAR) and Output (WAW) Dependencies */
int test_war_waw_dep(int *a, int *b, int *c, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* WAR (Anti-dependency): read after write */
        int temp = a[i] + g_volatile;  /* Read a[i] */
        a[i] = b[i] * 2;               /* Write a[i] - anti-dep with previous read */
        sum += temp;
        
        /* WAW (Output dependency): write after write */
        c[i] = i * 3;                  /* First write to c[i] */
        c[i] = c[i] + a[i];            /* Second write to c[i] - output dep */
        
        /* Mixed WAR/WAW with floating point */
        float f1 = (float)a[i] / 2.0f;
        a[i] = (int)f1;                /* WAW on a[i] */
        float f2 = f1 * 3.0f;          /* WAR on f1 */
        sum += (int)f2;
    }
    
    return sum;
}

/* Test 3: Memory Aliasing Dependencies */
int test_memory_aliasing(int *arr1, int *arr2, int *ptr1, int *ptr2, int n) {
    int sum = 0;
    
    /* Force potential aliasing */
    ptr1 = arr1;
    ptr2 = arr1 + 1;  /* ptr2 may alias with ptr1+1 */
    
    for (int i = 1; i < n-1; i++) {
        /* Memory operations that may alias */
        *ptr1 = arr2[i] + g_volatile;
        ptr1 = &arr1[i];
        
        *ptr2 = arr1[i-1] * 2;
        ptr2 = &arr2[i];
        
        /* More complex aliasing pattern */
        arr1[i] = arr2[i] + *ptr1;
        arr2[i] = arr1[i-1] + *ptr2;
        
        sum += arr1[i] + arr2[i];
    }
    
    /* Additional loop with pointer arithmetic */
    int *p = arr1;
    int *q = arr2;
    for (int i = 0; i < n; i++) {
        *p++ = *q++ + i;
        sum += *(p-1);
    }
    
    return sum;
}

/* Test 4: Control Dependencies */
int test_control_dep(int *a, int *b, int *c, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Control-dependent computations */
        if (a[i] > 0) {
            b[i] = a[i] * 2 + g_volatile;
            sum += b[i];
        } else {
            b[i] = a[i] / 2;
            sum -= b[i];
        }
        
        /* Nested control flow */
        if (i % 3 == 0) {
            c[i] = b[i] + 5;
            if (c[i] > 100) {
                c[i] = 100;
                sum += 10;
            }
        } else if (i % 3 == 1) {
            c[i] = b[i] - 5;
            sum += c[i];
        } else {
            c[i] = b[i] * 2;
            sum += c[i] / 2;
        }
        
        /* Loop with break/continue */
        if (sum > 1000000) {
            break;
        }
    }
    
    return sum;
}

/* Test 5: Mixed Data Types and Dependencies */
int test_mixed_deps(float *fa, int *ia, double *da, int n) {
    float fsum = 0.0f;
    int isum = 0;
    double dsum = 0.0;
    
    for (int i = 1; i < n; i++) {
        /* Cross-type dependencies */
        fa[i] = fa[i-1] * 1.1f + (float)g_volatile;  /* Float RAW */
        ia[i] = (int)fa[i] + ia[i-2];                /* Int RAW with distance 2 */
        da[i] = (double)ia[i] * 0.5 + da[i-1];       /* Double RAW */
        
        /* Mixed WAR */
        float ftemp = fa[i];
        fa[i] = (float)ia[i] * 0.25f;  /* WAR on fa[i] */
        
        /* Mixed WAW */
        double dtemp = da[i];
        da[i] = dtemp * 2.0;           /* WAW on da[i] */
        
        fsum += fa[i];
        isum += ia[i];
        dsum += da[i];
    }
    
    return isum + (int)fsum + (int)dsum;
}

/* Test 6: Nested Loops with Complex Dependencies */
int test_nested_loops(int *a, int *b, int *c, int n) {
    int sum = 0;
    
    for (int i = 1; i < n; i++) {
        /* Outer loop carried dependency */
        a[i] = a[i-1] + b[i] + g_volatile;
        
        for (int j = 1; j < 10; j++) {
            /* Inner loop with dependencies */
            b[j] = a[i] + c[j-1];
            c[j] = b[j] * 2;
            
            /* Memory dependency between inner and outer */
            a[i] += c[j] / 3;
            
            sum += b[j] + c[j];
        }
        
        /* Another inner loop with different pattern */
        for (int j = 0; j < 5; j++) {
            int idx = i * 5 + j;
            if (idx < n) {
                c[idx] = a[i] + b[j];
                sum += c[idx];
            }
        }
    }
    
    return sum;
}

/* Test 7: Explicit Loop-Carried Dependencies with Various Distances */
int test_loop_carried_dep(int *a, int n, int distance) {
    int sum = 0;
    
    /* Force specific distance values in DDG edges */
    for (int i = distance; i < n; i++) {
        /* Distance parameterized dependency */
        a[i] = a[i-distance] + i + g_volatile;
        
        /* Multiple distances in same loop */
        if (i >= 2) {
            a[i] += a[i-2] * 2;      /* Distance 2 */
        }
        if (i >= 3) {
            a[i] -= a[i-3] / 3;      /* Distance 3 */
        }
        if (i >= 4) {
            a[i] *= (a[i-4] % 5);    /* Distance 4 */
        }
        
        sum += a[i];
    }
    
    return sum;
}

/* Main function that runs all tests */
int main(int argc, char *argv[]) {
    const int N = 1000;
    int total_sum = 0;
    
    /* Initialize data */
    int *data1 = (int*)malloc(N * sizeof(int));
    int *data2 = (int*)malloc(N * sizeof(int));
    int *data3 = (int*)malloc(N * sizeof(int));
    float *fdata = (float*)malloc(N * sizeof(float));
    double *ddata = (double*)malloc(N * sizeof(double));
    
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        data1[i] = rand() % 100;
        data2[i] = rand() % 100;
        data3[i] = rand() % 100;
        fdata[i] = (float)(rand() % 100) / 10.0f;
        ddata[i] = (double)(rand() % 100) / 10.0;
    }
    
    /* Run all tests to create various DDG edges */
    printf("Running DDG edge creation tests...\n");
    
    total_sum += test_raw_dep(data1, data2, N);
    total_sum += test_war_waw_dep(data1, data2, data3, N);
    total_sum += test_memory_aliasing(data1, data2, data1, data2, N);
    total_sum += test_control_dep(data1, data2, data3, N);
    total_sum += test_mixed_deps(fdata, data1, ddata, N);
    total_sum += test_nested_loops(data1, data2, data3, N);
    total_sum += test_loop_carried_dep(data1, N, 3);
    
    /* Additional complex loop with all dependency types */
    for (int iter = 0; iter < 10; iter++) {
        for (int i = 2; i < N; i++) {
            /* RAW */
            int temp = data1[i-1] + g_volatile;
            
            /* WAR */
            data1[i-1] = data2[i] * iter;
            
            /* WAW */
            data3[i] = temp + iter;
            data3[i] = data3[i] * 2;
            
            /* Control */
            if (data3[i] > 100) {
                data2[i] = data3[i] / 2;
            } else {
                data2[i] = data3[i] * 2;
            }
            
            /* Memory alias */
            int *ptr = (iter % 2) ? &data1[i] : &data2[i];
            *ptr = data3[i] + i;
            
            total_sum += data1[i] + data2[i] + data3[i];
        }
    }
    
    printf("Final checksum: %d\n", total_sum);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    free(fdata);
    free(ddata);
    
    return total_sum != 0 ? 0 : 1;
}
