/* test_ddg_edges.c
 * Designed to trigger DDG edge creation in GCC's instruction scheduler
 * Compile with: gcc -O2 -funroll-loops -fmodulo-sched -c test_ddg_edges.c
 * Or: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -ftree-vectorize -c test_ddg_edges.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_array[1024];
int g_result = 0;

/* Function to create true data dependencies (RAW) */
int test_raw_dep(int *a, int *b, int n) {
    int sum = 0;
    /* Loop with flow dependencies across iterations */
    for (int i = 2; i < n; i++) {
        a[i] = a[i-1] + b[i];      /* RAW: a[i-1] read, then a[i] written */
        a[i] += a[i-2] + 1;        /* Additional RAW with distance 2 */
        sum += a[i];
    }
    return sum;
}

/* Function to create anti and output dependencies (WAR/WAW) */
int test_war_waw_dep(float *fa, float *fb, int n) {
    float temp = 0.0f;
    /* Mix of WAR and WAW dependencies */
    for (int i = 0; i < n; i++) {
        float t1 = fa[i] + fb[i];  /* Read fa[i], fb[i] */
        fa[i] = t1 * 2.0f;         /* WAR: fa[i] written after being read above */
        float t2 = fa[i] * 0.5f;   /* Read fa[i] again */
        fa[i] = t2 + 1.0f;         /* WAW: fa[i] written again */
        temp += fa[i];
    }
    return (int)temp;
}

/* Function with memory aliasing dependencies */
int test_memory_aliasing(int *arr1, int *arr2, int *ptr1, int *ptr2, int n) {
    int sum = 0;
    /* arr1 and arr2 may alias if pointers overlap */
    for (int i = 1; i < n; i++) {
        arr1[i] = arr2[i-1] + g_volatile;  /* Potential memory dep with arr2 */
        *ptr1 = arr1[i] * 2;               /* Pointer access - may alias with anything */
        arr2[i] = *ptr2 + i;               /* Another pointer access */
        sum += arr1[i] + arr2[i];
    }
    return sum;
}

/* Function with control dependencies */
int test_control_dep(int *data, int *mask, int n) {
    int result = 0;
    /* Loop with internal branching creating control dependencies */
    for (int i = 0; i < n; i++) {
        if (mask[i] > 0) {                 /* Control dependency */
            data[i] = data[i] * 2 + 1;
            result += data[i];
        } else {
            data[i] = data[i] / 2;
            result -= data[i];
        }
        
        /* Nested condition for more complex control flow */
        if (i % 3 == 0) {
            data[i] += g_volatile;         /* Volatile access prevents optimization */
        }
    }
    return result;
}

/* Complex loop with mixed dependencies */
int test_mixed_dependencies(int *a, int *b, int *c, int n) {
    int total = 0;
    
    /* Outer loop with inner loop - creates nested DDG */
    for (int i = 1; i < n; i++) {
        int acc = 0;
        /* Inner loop with carried dependency */
        for (int j = 0; j < 8; j++) {
            a[j] = a[j] + b[i] * c[j];     /* RAW on a[j] across inner iterations */
            acc += a[j];
        }
        
        /* Cross-iteration dependencies */
        b[i] = b[i-1] + acc;               /* RAW with distance 1 */
        c[i] = c[i] * 2 - b[i];            /* Various dependencies */
        
        total += b[i] + c[i];
        
        /* Function call acts as memory clobber */
        total += g_volatile;               /* Volatile prevents reordering */
    }
    
    return total;
}

/* Loop with floating point and integer mixed operations */
double test_fp_int_mix(double *darr, int *iarr, int n) {
    double sum = 0.0;
    
    for (int i = 2; i < n; i++) {
        /* FP operations with dependencies */
        double temp = darr[i-1] * 1.5;     /* RAW FP dependency */
        darr[i] = temp + darr[i-2];        /* RAW with distance 2 */
        
        /* Integer operations interleaved */
        iarr[i] = iarr[i-1] + (int)darr[i]; /* RAW int + FP->int conversion */
        
        /* Mixed type dependencies */
        darr[i] += (double)iarr[i] * 0.1;
        
        sum += darr[i] + iarr[i];
    }
    
    return sum;
}

/* Main function that runs all tests */
int main(int argc, char **argv) {
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 1000;
    }
    
    /* Allocate and initialize arrays */
    int *arr1 = (int*)malloc(n * sizeof(int));
    int *arr2 = (int*)malloc(n * sizeof(int));
    int *arr3 = (int*)malloc(n * sizeof(int));
    float *farr = (float*)malloc(n * sizeof(float));
    double *darr = (double*)malloc(n * sizeof(double));
    int *mask = (int*)malloc(n * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        arr1[i] = i % 100 + 1;
        arr2[i] = (i * 3) % 100 + 1;
        arr3[i] = (i * 7) % 100 + 1;
        farr[i] = (float)(i % 100) * 0.1f;
        darr[i] = (double)(i % 100) * 0.01;
        mask[i] = i % 5;
    }
    
    int checksum = 0;
    
    /* Run all test functions to create various DDG edges */
    checksum += test_raw_dep(arr1, arr2, n);
    checksum += test_war_waw_dep(farr, farr + n/2, n/2);
    checksum += test_memory_aliasing(arr1, arr2, &arr1[10], &arr2[20], n-20);
    checksum += test_control_dep(arr3, mask, n);
    checksum += test_mixed_dependencies(arr1, arr2, arr3, n/4);
    
    double fp_result = test_fp_int_mix(darr, arr1, n/2);
    checksum += (int)fp_result;
    
    /* Use results to prevent dead code elimination */
    g_result = checksum;
    
    /* Print something to ensure code isn't optimized away */
    printf("DDG test checksum: %d\n", checksum % 1000);
    
    free(arr1);
    free(arr2);
    free(arr3);
    free(farr);
    free(darr);
    free(mask);
    
    return checksum % 256;
}
