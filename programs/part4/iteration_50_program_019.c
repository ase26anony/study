/* test_ddg_coverage.c
 * Program designed to trigger DDG edge creation in GCC's scheduler
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
int test_raw_dependencies(int *arr, int *brr, int n);
int test_war_waw_dependencies(int *arr, int *brr, int n);
int test_memory_aliasing(int *arr, int *brr, int *crr, int n);
int test_control_dependencies(int *arr, int *brr, int n);
int test_mixed_dependencies(float *farr, int *iarr, int n);
int test_nested_loop_dependencies(int *arr, int n);

/* Test 1: True Data Dependencies (RAW) with loop-carried dependencies */
int test_raw_dependencies(int *arr, int *brr, int n) {
    int sum = 0;
    
    /* Multiple RAW dependencies with different distances */
    for (int i = 2; i < n; i++) {
        /* Distance 1 RAW dependency */
        arr[i] = arr[i-1] + brr[i];
        
        /* Distance 2 RAW dependency */
        brr[i] = arr[i-2] * 3;
        
        /* Mixed distance RAW with floating point */
        float temp = (float)arr[i] / 2.0f;
        arr[i] = (int)temp + brr[i-1];
        
        sum += arr[i];
    }
    
    /* Additional loop with different pattern */
    for (int i = 1; i < n; i++) {
        /* Chain of RAW dependencies */
        int t1 = arr[i] + brr[i];
        int t2 = t1 * arr[i-1];
        arr[i] = t2 + g_volatile;  /* volatile prevents elimination */
        sum += t2;
    }
    
    return sum;
}

/* Test 2: Anti (WAR) and Output (WAW) Dependencies */
int test_war_waw_dependencies(int *arr, int *brr, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* WAR (anti-dependency): read after write */
        int temp = arr[i];      /* Read arr[i] */
        arr[i] = i * 2;         /* Write arr[i] - anti-dependent on previous read */
        brr[i] = temp + arr[i]; /* Uses old value of arr[i] */
        
        /* WAW (output-dependency) */
        arr[i] = brr[i] * 3;    /* First write to arr[i] */
        arr[i] = arr[i] + 1;    /* Second write to arr[i] - output dependent */
        
        /* Mixed WAR/WAW with volatile */
        volatile int v = g_volatile;
        int x = arr[i];         /* Read */
        arr[i] = v;             /* Write - WAR */
        arr[i] = x + v;         /* Write - WAW with previous, WAR with x read */
        
        sum += arr[i] + brr[i];
    }
    
    return sum;
}

/* Test 3: Memory Aliasing Dependencies */
int test_memory_aliasing(int *arr, int *brr, int *crr, int n) {
    int sum = 0;
    
    /* Create ambiguous pointer aliasing */
    int *p = arr;
    int *q = brr;
    
    for (int i = 0; i < n; i++) {
        /* Potential aliasing through pointer arithmetic */
        *(p + i) = *(q + i) + i;
        
        /* More complex aliasing pattern */
        int *r = (i % 2) ? arr : brr;
        *r = *r + crr[i];
        
        /* Pointer chasing with unknown aliasing */
        int *s = &arr[(i * 7) % n];
        int *t = &brr[(i * 13) % n];
        *s = *t + g_volatile;
        *t = *s - 1;
        
        sum += arr[i] + brr[i];
    }
    
    /* Loop with array index aliasing */
    for (int i = 1; i < n - 1; i++) {
        /* Compiler can't prove arr[i+1] doesn't alias brr[i-1] */
        arr[i+1] = brr[i-1] * 2;
        brr[i] = arr[i] + crr[i];
        
        sum += arr[i];
    }
    
    return sum;
}

/* Test 4: Control Dependencies */
int test_control_dependencies(int *arr, int *brr, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Control-dependent computations */
        if (arr[i] > 0) {
            brr[i] = arr[i] * 2;
            sum += brr[i];
        } else {
            brr[i] = -arr[i];
            sum -= brr[i];
        }
        
        /* Nested control flow */
        if (i % 3 == 0) {
            arr[i] = brr[i] + 1;
            if (brr[i] > 10) {
                arr[i] *= 2;
            }
        } else if (i % 3 == 1) {
            arr[i] = brr[i] - 1;
        } else {
            arr[i] = brr[i] * brr[i-1];
        }
        
        /* Control-dependent memory access */
        volatile int *ptr = &g_global_array[i % 1024];
        if (arr[i] % 2 == 0) {
            *ptr = arr[i];
        } else {
            *ptr = brr[i];
        }
        
        sum += arr[i];
    }
    
    return sum;
}

/* Test 5: Mixed Data Types and Operations */
int test_mixed_dependencies(float *farr, int *iarr, int n) {
    float fsum = 0.0f;
    int isum = 0;
    
    for (int i = 1; i < n; i++) {
        /* Mixed int/float RAW */
        float f1 = (float)iarr[i-1] * 0.5f;
        farr[i] = f1 + farr[i-1];  /* Float RAW */
        
        /* Float to int conversion dependency */
        iarr[i] = (int)farr[i] + iarr[i-1];  /* Mixed type RAW */
        
        /* Complex mixed operations */
        double dtemp = (double)farr[i] * 1.5;
        farr[i] = (float)dtemp + (float)iarr[i] * 0.1f;
        
        /* Volatile forces dependency */
        farr[i] += (float)g_volatile;
        
        fsum += farr[i];
        isum += iarr[i];
    }
    
    return isum + (int)fsum;
}

/* Test 6: Nested Loops with Complex Dependencies */
int test_nested_loop_dependencies(int *arr, int n) {
    int sum = 0;
    
    /* Outer loop with carried dependency */
    for (int i = 1; i < n; i++) {
        int inner_sum = 0;
        
        /* Inner loop with multiple dependency types */
        for (int j = 1; j < 10; j++) {
            /* RAW within inner loop */
            int temp = arr[j] + i;
            arr[j] = temp * 2;
            
            /* WAR between iterations */
            inner_sum += arr[j-1];  /* Read arr[j-1] */
            arr[j-1] = j + i;       /* Write arr[j-1] - WAR */
            
            /* Control in inner loop */
            if (temp % 2 == 0) {
                arr[j] += inner_sum;
            }
            
            inner_sum += arr[j];
        }
        
        /* Loop-carried dependency to next outer iteration */
        arr[i] = arr[i-1] + inner_sum;
        sum += arr[i];
    }
    
    return sum;
}

/* Main function that runs all tests */
int main(int argc, char **argv) {
    /* Use command line or default size */
    int n = (argc > 1) ? atoi(argv[1]) : 1000;
    if (n < 10) n = 1000;
    
    /* Allocate arrays with different alignments */
    int *arr1 = (int*)aligned_alloc(64, n * sizeof(int));
    int *arr2 = (int*)aligned_alloc(64, n * sizeof(int));
    int *arr3 = (int*)aligned_alloc(64, n * sizeof(int));
    float *farr = (float*)aligned_alloc(64, n * sizeof(float));
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < n; i++) {
        arr1[i] = (i * 3) % 97;
        arr2[i] = (i * 7) % 113;
        arr3[i] = (i * 11) % 151;
        farr[i] = (float)(i % 29) * 0.7f;
    }
    
    int total_result = 0;
    
    /* Run each test multiple times to increase coverage chance */
    for (int iter = 0; iter < 3; iter++) {
        g_volatile = iter;  /* Change volatile each iteration */
        
        total_result += test_raw_dependencies(arr1, arr2, n);
        total_result += test_war_waw_dependencies(arr1, arr2, n);
        total_result += test_memory_aliasing(arr1, arr2, arr3, n);
        total_result += test_control_dependencies(arr1, arr2, n);
        total_result += test_mixed_dependencies(farr, arr1, n);
        total_result += test_nested_loop_dependencies(arr3, n);
        
        /* Rearrange data to create different patterns */
        for (int i = 0; i < n; i++) {
            arr1[i] = (arr1[i] + arr2[i]) % 256;
            arr2[i] = (arr2[i] + arr3[i]) % 256;
            arr3[i] = (arr3[i] + i) % 256;
        }
    }
    
    /* Store final result to prevent dead code elimination */
    g_result = total_result;
    
    /* Print result to ensure code isn't optimized away */
    printf("Result: %d\n", total_result % 1000000);
    
    free(arr1);
    free(arr2);
    free(arr3);
    free(farr);
    
    return 0;
}
