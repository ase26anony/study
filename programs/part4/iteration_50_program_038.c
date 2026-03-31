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
int test_mixed_dependencies(int *a, int *b, float *fa, float *fb, int n);
int test_loop_carried_dependencies(int *a, int n);

/* Test 1: True Data Dependencies (RAW) with loop-carried dependencies */
int test_raw_dependencies(int *a, int *b, int n) {
    int sum = 0;
    
    /* Multiple RAW dependencies with different distances */
    for (int i = 2; i < n; i++) {
        /* Distance 1 RAW */
        a[i] = a[i-1] + b[i];
        
        /* Distance 2 RAW with arithmetic */
        b[i] = a[i-2] * 3 + g_volatile;
        
        /* Chain of RAW dependencies */
        int temp = a[i] + b[i];
        a[i] = temp * 2;
        b[i] = a[i] + 1;
        
        sum += a[i] + b[i];
    }
    
    /* Additional RAW with floating point */
    float fsum = 0.0f;
    for (int i = 1; i < n; i++) {
        float ftemp = (float)a[i-1] * 1.5f;
        fsum += ftemp + (float)b[i];
    }
    
    return sum + (int)fsum;
}

/* Test 2: Anti and Output Dependencies (WAR/WAW) */
int test_war_waw_dependencies(int *a, int *b, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* WAR (anti-dependency): read a[i], then write to it */
        int temp = a[i] + g_volatile;
        a[i] = temp * 2;  // WAR: a[i] read then written
        
        /* WAW (output-dependency): multiple writes to same location */
        b[i] = temp + 1;
        b[i] = b[i] * 3;  // WAW: b[i] written twice
        
        /* Complex WAR/WAW mixing */
        int x = a[i];
        int y = b[i];
        a[i] = x + y;     // WAR on a[i], WAW on a[i]
        b[i] = a[i] - x;  // WAR on b[i], WAW on b[i]
        
        sum += a[i] + b[i];
    }
    
    return sum;
}

/* Test 3: Memory Aliasing Dependencies */
int test_memory_aliasing(int *a, int *b, int *c, int n) {
    int sum = 0;
    
    /* Create ambiguous pointer aliasing */
    int *p = a;
    int *q = b;
    
    for (int i = 0; i < n; i++) {
        /* Potential aliasing through pointers */
        *p = *q + i;
        *q = *p * 2;
        
        /* Array accesses with variable indices - may alias */
        int idx1 = i % (n/2);
        int idx2 = (i + 1) % (n/2);
        a[idx1] = b[idx2] + c[i];
        b[idx2] = a[idx1] * 3;
        
        /* Pointer arithmetic creating potential aliasing */
        p = &a[(i * 7) % n];
        q = &b[(i * 13) % n];
        
        sum += *p + *q;
    }
    
    return sum;
}

/* Test 4: Control Dependencies */
int test_control_dependencies(int *a, int *b, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple conditional branches creating control dependencies */
        if (a[i] > 0) {
            b[i] = a[i] * 2;
            if (b[i] < 100) {
                a[i] = b[i] + g_volatile;
            } else {
                a[i] = b[i] - g_volatile;
            }
        } else {
            b[i] = a[i] / 2;
            if (i % 3 == 0) {
                a[i] = b[i] * 3;
            }
        }
        
        /* Nested conditions */
        switch (i % 4) {
            case 0:
                a[i] += 1;
                break;
            case 1:
                a[i] += 2;
                break;
            case 2:
                a[i] += 3;
                break;
            default:
                a[i] += 4;
                break;
        }
        
        sum += a[i] + b[i];
    }
    
    return sum;
}

/* Test 5: Mixed Data Types and Dependencies */
int test_mixed_dependencies(int *a, int *b, float *fa, float *fb, int n) {
    int sum = 0;
    float fsum = 0.0f;
    
    for (int i = 1; i < n; i++) {
        /* Integer RAW with distance */
        a[i] = a[i-1] + b[i];
        
        /* Floating point RAW */
        fa[i] = fa[i-1] * 1.1f + (float)b[i];
        
        /* Mixed type dependencies */
        fb[i] = (float)a[i] + fa[i];
        
        /* Integer WAR */
        int temp = b[i];
        b[i] = a[i] * 2;
        a[i] = temp + 1;
        
        /* Floating point WAW */
        fa[i] = fa[i] * 2.0f;
        fa[i] = fa[i] + 1.0f;
        
        sum += a[i] + b[i];
        fsum += fa[i] + fb[i];
    }
    
    return sum + (int)fsum;
}

/* Test 6: Complex Loop-Carried Dependencies */
int test_loop_carried_dependencies(int *a, int n) {
    int sum = 0;
    
    /* Multiple loop-carried dependencies with different distances */
    for (int i = 4; i < n; i++) {
        /* Distance 1, 2, and 3 dependencies */
        a[i] = a[i-1] + a[i-2] + a[i-3] + a[i-4];
        
        /* Chain of dependencies across iterations */
        int t1 = a[i-1] * 2;
        int t2 = t1 + a[i-2];
        int t3 = t2 - a[i-3];
        a[i] = a[i] + t3;
        
        /* Recurrence with computation */
        a[i] = (a[i] * 3 + g_volatile) % 1000;
        
        sum += a[i];
    }
    
    /* Nested loop with dependencies */
    for (int i = 0; i < n/2; i++) {
        for (int j = 1; j < 8; j++) {
            a[i*8 + j] = a[i*8 + j-1] + i + j;
            sum += a[i*8 + j];
        }
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
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    float *fa = (float*)malloc(n * sizeof(float));
    float *fb = (float*)malloc(n * sizeof(float));
    
    for (int i = 0; i < n; i++) {
        a[i] = (i * 3) % 97;
        b[i] = (i * 7) % 113;
        c[i] = (i * 11) % 157;
        fa[i] = (float)(i % 89) * 0.1f;
        fb[i] = (float)(i % 73) * 0.2f;
    }
    
    /* Run all tests to create various DDG edges */
    int result = 0;
    
    result += test_raw_dependencies(a, b, n);
    result += test_war_waw_dependencies(a, b, n);
    result += test_memory_aliasing(a, b, c, n);
    result += test_control_dependencies(a, b, n);
    result += test_mixed_dependencies(a, b, fa, fb, n);
    result += test_loop_carried_dependencies(a, n);
    
    /* Store to global to prevent optimization */
    g_result = result;
    
    /* Print result to ensure code isn't eliminated */
    printf("Result: %d\n", result);
    
    /* Clean up */
    free(a);
    free(b);
    free(c);
    free(fa);
    free(fb);
    
    return 0;
}
