/* test_ddg_coverage.c
 * Designed to trigger DDG edge creation in GCC's scheduler
 * Compile with: gcc -O2 -fmodulo-sched -funroll-loops -c test_ddg_coverage.c
 * Or: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_array[1024];
int g_result = 0;

/* Function to prevent dead code elimination */
static int use_value(int v) {
    g_volatile = v;
    return v;
}

/* Test 1: True Data Dependencies (RAW) with loop-carried dependencies */
int test_raw_dep(int *a, int *b, int n) {
    int sum = 0;
    /* Multiple RAW dependencies with different distances */
    for (int i = 2; i < n; i++) {
        a[i] = a[i-1] + b[i];          /* Distance 1 RAW */
        a[i] += a[i-2] * 2;            /* Distance 2 RAW */
        sum += a[i];
    }
    
    /* Mixed floating point and integer for different data types */
    float *fa = (float*)a;
    for (int i = 3; i < n; i++) {
        fa[i] = fa[i-1] * 1.5f + fa[i-3];  /* FP RAW with distance 1 and 3 */
        sum += (int)fa[i];
    }
    
    return use_value(sum);
}

/* Test 2: Anti (WAR) and Output (WAW) Dependencies */
int test_war_waw_dep(int *a, int *b, int *c, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        int temp = a[i];          /* Read a[i] */
        a[i] = b[i] + c[i];       /* Write a[i] - WAR with previous read */
        b[i] = temp * 2;          /* Use temp (prevents elimination) */
        
        /* WAW chain */
        c[i] = i * 3;
        c[i] = c[i] + temp;       /* WAW on c[i] */
        
        /* Another WAR with different data types */
        float ftemp = (float)a[i];
        a[i] = (int)(ftemp * 1.5f);  /* WAR: read ftemp, write a[i] */
        
        sum += a[i] + b[i] + c[i];
    }
    
    return use_value(sum);
}

/* Test 3: Memory Aliasing Dependencies */
int test_memory_aliasing(int *arr, int n) {
    int sum = 0;
    int *p = arr;
    int *q = arr + (n/2);
    
    /* Pointers with potential aliasing */
    for (int i = 0; i < n/2; i++) {
        *p = *q + i;        /* Memory dependency: read q, write p */
        p[i] = q[i] * 2;    /* Array access with potential overlap */
        q[i] = p[i+1] + 1;  /* More complex aliasing pattern */
        
        /* Volatile access creates hard memory dependency */
        g_array[i] = *p;
        *q = g_volatile;
        
        sum += p[i] + q[i];
    }
    
    /* Restrict qualifier to create both aliasing and non-aliasing edges */
    int *restrict r1 = arr;
    int *restrict r2 = arr + 100;
    for (int i = 0; i < 50; i++) {
        r1[i] = r2[i] * 3;  /* No aliasing due to restrict */
        sum += r1[i];
    }
    
    return use_value(sum);
}

/* Test 4: Control Dependencies */
int test_control_dep(int *a, int *b, int n) {
    int sum = 0;
    
    /* Loop with internal branching */
    for (int i = 0; i < n; i++) {
        if (i % 3 == 0) {
            a[i] = b[i] * 2;
            sum += a[i];
        } else if (i % 3 == 1) {
            a[i] = b[i] + a[i-1];  /* RAW with control dep */
            sum -= a[i];
        } else {
            a[i] = b[i] / 2;
            sum *= (a[i] + 1);
        }
        
        /* Nested condition */
        if (a[i] > 100) {
            b[i] = a[i] - 50;
            sum += b[i] * 3;
        }
    }
    
    /* Loop with computed goto-like control flow */
    for (int i = 1; i < n; i++) {
        int cond = a[i] & 0xF;
        switch (cond) {
            case 0: a[i] = b[i] + 1; break;
            case 1: a[i] = b[i] * a[i-1]; break;  /* RAW across iterations */
            case 2: a[i] = b[i] - a[i]; break;    /* WAR */
            default: a[i] = b[i];
        }
        sum += a[i];
    }
    
    return use_value(sum);
}

/* Test 5: Complex Loop Nest with All Dependency Types */
int test_complex_nest(int *a, int *b, int *c, int n) {
    int sum = 0;
    
    /* Outer loop with multiple inner loops */
    for (int i = 1; i < n; i++) {
        /* Inner loop 1: RAW with distance */
        for (int j = 1; j < 10; j++) {
            a[i*10 + j] = a[i*10 + j - 1] + b[j];
        }
        
        /* Inner loop 2: WAR/WAW */
        for (int j = 0; j < 10; j++) {
            int temp = c[j];
            c[j] = a[i*10 + j] * 2;
            a[i*10 + j] = temp + i;
        }
        
        /* Memory aliasing between arrays */
        for (int j = 0; j < 5; j++) {
            b[j] = c[j+5] + a[i*10 + j];
            c[j+5] = b[j] * 3;
        }
        
        /* Control in inner loop */
        for (int j = 0; j < 10; j++) {
            if ((i + j) % 2 == 0) {
                a[i*10 + j] += g_volatile;
            } else {
                a[i*10 + j] -= b[j%5];
            }
            sum += a[i*10 + j];
        }
    }
    
    return use_value(sum);
}

/* Test 6: Function Calls Creating Memory Clobbering */
static int helper1(int x, int y) {
    g_volatile = x + y;
    return g_volatile;
}

static void helper2(int *p, int v) {
    *p = v + g_volatile;
    g_array[v % 100] = *p;
}

int test_func_calls(int *a, int n) {
    int sum = 0;
    
    for (int i = 1; i < n; i++) {
        /* Function calls create memory dependencies */
        int tmp = helper1(a[i-1], i);
        a[i] = tmp + a[i];
        
        /* Another call with side effects */
        helper2(&a[i], i);
        
        /* Inline asm for additional serialization */
        asm volatile ("" : : "r"(a[i]) : "memory");
        
        sum += a[i];
    }
    
    return use_value(sum);
}

/* Main driver that runs all tests */
int main(int argc, char **argv) {
    int n = 1000;
    if (argc > 1) n = atoi(argv[1]);
    if (n < 100) n = 100;
    
    /* Allocate arrays with different alignments */
    int *a1 = (int*)aligned_alloc(64, n * sizeof(int));
    int *a2 = (int*)aligned_alloc(64, n * sizeof(int));
    int *a3 = (int*)aligned_alloc(64, n * sizeof(int));
    int *b = (int*)aligned_alloc(64, n * sizeof(int));
    int *c = (int*)aligned_alloc(64, n * sizeof(int));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        a1[i] = (i * 3) % 97;
        a2[i] = (i * 5) % 101;
        a3[i] = (i * 7) % 103;
        b[i] = (i * 11) % 107;
        c[i] = (i * 13) % 109;
    }
    
    /* Run all tests to create various DDG edges */
    int result = 0;
    
    result += test_raw_dep(a1, b, n);
    result += test_war_waw_dep(a2, b, c, n/2);
    result += test_memory_aliasing(a3, n);
    result += test_control_dep(a1, b, n);
    result += test_complex_nest(a1, b, c, n/10);
    result += test_func_calls(a2, n);
    
    /* Use result to prevent elimination */
    printf("Result checksum: %d\n", use_value(result));
    
    free(a1);
    free(a2);
    free(a3);
    free(b);
    free(c);
    
    return result != 0;
}
