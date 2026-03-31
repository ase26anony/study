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
float g_float_array[1024];
int* g_ptr1;
int* g_ptr2;

/* Function to create true data dependencies (RAW) */
int test_raw_dep(int* arr, int* brr, int n) {
    int sum = 0;
    /* Loop with flow dependencies across iterations */
    for (int i = 2; i < n; i++) {
        arr[i] = arr[i-1] + brr[i];      /* RAW: arr[i-1] read, then arr[i] written */
        arr[i-1] = arr[i-2] + brr[i-1];  /* Another RAW with distance 2 */
        sum += arr[i];
    }
    return sum;
}

/* Function to create anti and output dependencies (WAR/WAW) */
float test_war_waw_dep(float* fa, float* fb, int n) {
    float temp = 0.0f;
    /* Mix of WAR and WAW dependencies */
    for (int i = 0; i < n; i++) {
        float x = fa[i] + fb[i];      /* Read fa[i], fb[i] */
        fa[i] = x * 2.0f;             /* WAR: fa[i] written after being read above */
        fb[i] = fa[i] / 3.0f;         /* WAR: fa[i] read after being written above */
        
        /* WAW chain */
        temp = fa[i] + 1.0f;
        temp = fb[i] * 2.0f;          /* WAW: temp written twice */
        fa[i] = temp;                 /* WAW: fa[i] written again */
        
        /* Cross-iteration WAR */
        if (i > 0) {
            fb[i-1] = fa[i] + fb[i];  /* WAR: fa[i] read, fb[i-1] written */
        }
    }
    return temp;
}

/* Function with memory aliasing dependencies */
int test_memory_aliasing(int* arr1, int* arr2, int n) {
    int* p = arr1;
    int* q = arr2;
    int result = 0;
    
    /* Force potential aliasing */
    if (g_volatile > 0) {
        q = arr1 + 1;  /* q might alias with p */
    }
    
    for (int i = 1; i < n - 1; i++) {
        /* Memory operations with possible aliasing */
        *p = i * 2;
        *q = *p + 1;           /* Could be RAW if p and q alias */
        arr1[i] = arr2[i-1];   /* Another memory dependency */
        result += *p + *q;
        
        /* Update pointers in a way that creates loop-carried dependencies */
        p = &arr1[i];
        q = &arr2[(i + g_volatile) % n];
    }
    return result;
}

/* Function with control dependencies */
int test_control_dep(int* arr, int n) {
    int count = 0;
    volatile int v = g_volatile;
    
    for (int i = 0; i < n; i++) {
        /* Multiple control-dependent paths */
        if (arr[i] > 0) {
            arr[i] = arr[i] * 2 + 1;
            count++;
        } else if (arr[i] < -10) {
            arr[i] = arr[i] / 2;
            count--;
        } else {
            arr[i] = v;
        }
        
        /* Nested control flow */
        for (int j = 0; j < 3; j++) {
            if ((i + j) % 2 == 0) {
                arr[i] += j;
            }
        }
    }
    return count;
}

/* Complex loop with mixed dependencies */
double test_mixed_deps(double* da, int* ia, int n) {
    double acc = 0.0;
    int idx = 0;
    
    /* Loop with multiple dependency types */
    for (int i = 1; i < n; i++) {
        /* RAW with floating point */
        da[i] = da[i-1] * 1.5 + ia[i];
        
        /* WAR with integer */
        int old_val = ia[i];
        ia[i] = (int)da[i] + old_val;
        
        /* WAW */
        acc = da[i];
        acc = acc + ia[i];
        
        /* Memory dependency with global */
        g_global_array[i % 1024] = ia[i];
        ia[i] = g_global_array[(i-1) % 1024] + 1;
        
        /* Control dependency affecting memory ops */
        if (acc > 1000.0) {
            da[i] = da[i] / 2.0;
        }
        
        /* Pointer chasing creating dependencies */
        idx = ia[idx % n];
    }
    
    return acc;
}

/* Function with nested loops to create complex DDG */
int test_nested_loops(int* arr, int n, int m) {
    int total = 0;
    
    for (int i = 1; i < n; i++) {
        int inner_sum = 0;
        
        /* Inner loop with dependencies */
        for (int j = 1; j < m; j++) {
            /* Loop-carried dependency in inner loop */
            arr[j] = arr[j-1] + i;
            
            /* Cross-loop dependency */
            inner_sum += arr[j] + g_global_array[(i+j) % 1024];
            
            /* Anti-dependency */
            int temp = arr[j];
            arr[j] = j * 2;
            inner_sum += temp;
        }
        
        /* Outer loop dependency */
        g_global_array[i % 1024] = inner_sum;
        total += inner_sum;
        
        /* Dependency between outer loop iterations */
        if (i > 1) {
            arr[i] = arr[i-2] + g_global_array[i-1];
        }
    }
    
    return total;
}

/* Main function that calls all test cases */
int main(int argc, char** argv) {
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 100) n = 100;
        if (n > 10000) n = 10000;
    }
    
    /* Initialize data */
    int* arr1 = (int*)malloc(n * sizeof(int));
    int* arr2 = (int*)malloc(n * sizeof(int));
    float* farr1 = (float*)malloc(n * sizeof(float));
    float* farr2 = (float*)malloc(n * sizeof(float));
    double* darr = (double*)malloc(n * sizeof(double));
    
    for (int i = 0; i < n; i++) {
        arr1[i] = (i * 3) % 97;
        arr2[i] = (i * 7) % 101;
        farr1[i] = (float)i * 0.5f;
        farr2[i] = (float)i * 1.5f;
        darr[i] = (double)i * 2.5;
        g_global_array[i % 1024] = i;
    }
    
    g_ptr1 = arr1;
    g_ptr2 = arr2;
    
    /* Call all test functions to build various DDG edges */
    int sum1 = test_raw_dep(arr1, arr2, n);
    float sum2 = test_war_waw_dep(farr1, farr2, n);
    int sum3 = test_memory_aliasing(arr1, arr2, n);
    int sum4 = test_control_dep(arr1, n);
    double sum5 = test_mixed_deps(darr, arr2, n);
    int sum6 = test_nested_loops(arr1, n/2, 10);
    
    /* Use results to prevent dead code elimination */
    int final_result = sum1 + (int)sum2 + sum3 + sum4 + (int)sum5 + sum6;
    
    printf("Result: %d (checksum to prevent optimization)\n", final_result % 1000);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    free(darr);
    
    return final_result % 256;
}
