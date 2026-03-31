/* test_ddg_coverage.c
 * This program creates various loop patterns to trigger DDG edge creation
 * in GCC's instruction scheduler, specifically targeting create_ddg_edge()
 * function in ddg.cc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_array[1024];
int g_result = 0;

/* Function to prevent dead code elimination */
static int use_value(int x) {
    g_volatile = x;
    return x;
}

/* Test 1: True Data Dependencies (RAW) with loop-carried dependencies */
int test_raw_dep(int *a, int *b, int n) {
    int sum = 0;
    /* RAW dependencies with distance 1 and 2 */
    for (int i = 2; i < n; i++) {
        a[i] = a[i-1] + b[i];          /* Distance 1 RAW */
        a[i] += a[i-2] * 2;            /* Distance 2 RAW */
        sum += a[i];
    }
    /* Additional RAW with floating point */
    float *fa = (float*)a;
    for (int i = 1; i < n/2; i++) {
        fa[i] = fa[i-1] * 1.5f + (float)b[i];
        sum += (int)fa[i];
    }
    return use_value(sum);
}

/* Test 2: Anti (WAR) and Output (WAW) Dependencies */
int test_war_waw_dep(int *a, int *b, int *c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        int temp = a[i] + b[i];        /* Read a[i] */
        a[i] = c[i] * 2;               /* WAR: Write a[i] after read */
        b[i] = temp + a[i];            /* Use temp */
        
        /* WAW chain */
        a[i] = temp;                   /* First write */
        a[i] = a[i] * 3;               /* Second write - WAW */
        a[i] = a[i] + 1;               /* Third write - WAW */
        
        sum += a[i] + b[i];
    }
    return use_value(sum);
}

/* Test 3: Memory Aliasing with pointers */
int test_memory_aliasing(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    int *q = arr + n/2;
    
    /* Potentially aliasing pointers */
    for (int i = 0; i < n/2; i++) {
        *p = *q + i;                   /* May alias with q */
        *q = *p * 2;                   /* May alias with p */
        sum += *p + *q;
        p++;
        q--;
    }
    
    /* Array indices with non-linear access */
    for (int i = 0; i < n; i++) {
        int idx1 = (i * 7) % n;
        int idx2 = (i * 13) % n;
        arr[idx1] = arr[idx2] + arr[i];
        sum += arr[idx1];
    }
    
    return use_value(sum);
}

/* Test 4: Control Dependencies with branching */
int test_control_dep(int *a, int *b, int *c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        /* Control-dependent computations */
        if (a[i] > 0) {
            b[i] = c[i] * 2;
            sum += b[i];
        } else {
            b[i] = -c[i];
            sum -= b[i];
        }
        
        /* Nested control flow */
        if (i % 3 == 0) {
            a[i] = b[i] + 1;
            if (b[i] > 100) {
                c[i] = a[i] * 3;
            } else {
                c[i] = a[i] / 2;
            }
        } else if (i % 3 == 1) {
            a[i] = b[i] - 1;
            c[i] = a[i] + b[i];
        } else {
            a[i] = b[i] * 2;
            c[i] = a[i] - b[i];
        }
        
        sum += a[i] + c[i];
    }
    return use_value(sum);
}

/* Test 5: Mixed dependencies in nested loops */
int test_nested_loops(int *a, int *b, int n) {
    int sum = 0;
    /* Outer loop with carried dependency */
    for (int i = 1; i < n; i++) {
        /* Inner loop with various dependencies */
        for (int j = 1; j < 10; j++) {
            /* RAW within inner loop */
            b[j] = a[j-1] + i;
            
            /* WAR within inner loop */
            int temp = a[j];
            a[j] = b[j] * j;
            b[j] = temp + a[j];
            
            /* WAW */
            a[j] = temp;
            a[j] = a[j] + b[j];
            
            sum += a[j] + b[j];
        }
        /* Loop-carried dependency across outer iterations */
        a[i] = a[i-1] + sum % 100;
    }
    return use_value(sum);
}

/* Test 6: Function calls creating memory dependencies */
static int helper_func(int x, int *p) {
    *p = x * 2;
    return x + *p;
}

int test_with_calls(int *a, int *b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        /* Function calls act as memory clobbers */
        a[i] = helper_func(b[i], &a[(i+1)%n]);
        
        /* Volatile access creates hard dependency */
        g_volatile = a[i];
        b[i] = g_volatile + i;
        
        sum += a[i] + b[i];
    }
    return use_value(sum);
}

/* Test 7: Complex loop with all dependency types */
int test_complex_all(int *a, int *b, int *c, int n) {
    int sum = 0;
    for (int i = 2; i < n; i++) {
        /* RAW with distance */
        int t1 = a[i-1] + b[i-2];
        
        /* WAR */
        int t2 = a[i];
        a[i] = t1 * 3;
        
        /* WAW */
        c[i] = t2 + 1;
        c[i] = c[i] * 2;
        
        /* Control dependency */
        if (t1 > t2) {
            b[i] = a[i] + c[i];
        } else {
            b[i] = a[i] - c[i];
        }
        
        /* Memory alias concern */
        int *p = (i % 2) ? &a[i] : &b[i];
        *p = *p + sum;
        
        /* Another RAW */
        a[i] = b[i-1] + c[i];
        
        sum += a[i] + b[i] + c[i];
    }
    return use_value(sum);
}

int main(void) {
    const int N = 1000;
    int *array1 = malloc(N * sizeof(int));
    int *array2 = malloc(N * sizeof(int));
    int *array3 = malloc(N * sizeof(int));
    
    /* Initialize with random data */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        array1[i] = rand() % 100;
        array2[i] = rand() % 100;
        array3[i] = rand() % 100;
        g_array[i] = rand() % 100;
    }
    
    int total = 0;
    
    /* Run all tests to create various DDG patterns */
    total += test_raw_dep(array1, array2, N);
    total += test_war_waw_dep(array1, array2, array3, N);
    total += test_memory_aliasing(g_array, N);
    total += test_control_dep(array1, array2, array3, N);
    total += test_nested_loops(array1, array2, N/10);
    total += test_with_calls(array1, array2, N);
    total += test_complex_all(array1, array2, array3, N);
    
    /* Use the result to prevent optimization */
    printf("Result checksum: %d\n", total);
    printf("Volatile guard: %d\n", g_volatile);
    
    free(array1);
    free(array2);
    free(array3);
    
    return total != 0 ? 0 : 1;
}
