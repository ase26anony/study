/* test_ddg_coverage.c
 * Program designed to trigger DDG edge creation in GCC's scheduler
 * Compile with: gcc -O2 -funroll-loops -fmodulo-sched -c test_ddg_coverage.c
 * Or: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -ftree-vectorize -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_global_array[1024];
int g_result = 0;

/* Function prototypes */
int test_raw_dep(int *a, int *b, int n);
int test_war_waw_dep(int *a, int *b, int n);
int test_memory_aliasing(int *a, int *b, int *c, int n);
int test_control_dep(int *a, int *b, int n);
int test_mixed_deps(int *a, int *b, float *fa, float *fb, int n);
int test_nested_loop_deps(int *a, int *b, int n, int m);
int test_loop_carried_deps(int *a, int *b, int n);

/* Test 1: True Data Dependencies (RAW) with loop-carried dependency */
int test_raw_dep(int *a, int *b, int n) {
    int sum = 0;
    
    /* Multiple RAW dependencies with different distances */
    for (int i = 2; i < n; i++) {
        /* Distance 1 RAW dependency */
        a[i] = a[i-1] + b[i];
        
        /* Distance 2 RAW dependency */
        b[i] = a[i-2] * 3 + b[i-1];
        
        /* Floating point RAW to create different data type edges */
        float temp = (float)a[i] / 2.0f;
        sum += (int)temp;
    }
    
    /* Prevent dead code elimination */
    g_volatile = sum;
    return sum;
}

/* Test 2: Anti (WAR) and Output (WAW) Dependencies */
int test_war_waw_dep(int *a, int *b, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        int t1 = a[i];          /* Read a[i] */
        a[i] = b[i] + 1;        /* Write a[i] - WAR with above read */
        
        int t2 = b[i];          /* Read b[i] */
        b[i] = t1 * 2;          /* Write b[i] - WAR with above read */
        
        /* WAW dependencies */
        a[i] = a[i] * 3;        /* Second write to a[i] - WAW */
        a[i] = a[i] + 5;        /* Third write to a[i] - WAW */
        
        /* Mixed integer/float operations */
        float f1 = (float)a[i];
        float f2 = f1 * 1.5f;   /* RAW on float */
        f1 = f2 + 2.0f;         /* WAR on f1 */
        
        sum += (int)f1;
    }
    
    g_volatile = sum;
    return sum;
}

/* Test 3: Memory Aliasing Dependencies */
int test_memory_aliasing(int *a, int *b, int *c, int n) {
    int sum = 0;
    
    /* Use pointers that may alias */
    int *p = a;
    int *q = b;
    
    /* Compiler can't determine if p and q alias */
    for (int i = 0; i < n; i++) {
        *p = *q + i;            /* Memory read from q, write to p */
        p = &a[(i + 1) % n];    /* Change p pointer */
        q = &b[(i + 2) % n];    /* Change q pointer */
        
        /* Additional memory operations with array indices */
        c[i] = a[i] + b[i];     /* May alias with p/q operations */
        
        sum += c[i];
    }
    
    /* Force memory barrier */
    asm volatile("" ::: "memory");
    
    g_volatile = sum;
    return sum;
}

/* Test 4: Control Dependencies */
int test_control_dep(int *a, int *b, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Control-dependent computations */
        if (a[i] > 0) {
            b[i] = a[i] * 2;
            sum += b[i];
        } else {
            b[i] = a[i] / 2;
            sum -= b[i];
        }
        
        /* Nested control flow */
        if (i % 3 == 0) {
            a[i] = sum % 100;
        } else if (i % 3 == 1) {
            a[i] = (sum * 2) % 100;
        } else {
            a[i] = (sum / 2) % 100;
        }
        
        /* Loop with break condition (creates control dep edges) */
        if (sum > 10000) {
            sum = sum % 1000;
        }
    }
    
    g_volatile = sum;
    return sum;
}

/* Test 5: Mixed Dependency Types */
int test_mixed_deps(int *a, int *b, float *fa, float *fb, int n) {
    int sum = 0;
    
    for (int i = 1; i < n; i++) {
        /* RAW with distance 3 */
        a[i] = a[i-3] + b[i];
        
        /* WAR */
        int temp = a[i];
        a[i] = b[i] * 2;
        b[i] = temp + 1;
        
        /* WAW */
        fa[i] = (float)a[i];
        fa[i] = fa[i] * 1.5f;
        
        /* Memory dependency with float */
        fb[i] = fa[i-1] + fa[i];
        
        /* Control dependency */
        if (fb[i] > 100.0f) {
            fb[i] = fb[i] - 50.0f;
        }
        
        sum += (int)fb[i];
    }
    
    g_volatile = sum;
    return sum;
}

/* Test 6: Nested Loops with Dependencies */
int test_nested_loop_deps(int *a, int *b, int n, int m) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Outer loop carried dependency */
        a[i] = (i > 0) ? a[i-1] + 1 : 0;
        
        for (int j = 0; j < m; j++) {
            /* Inner loop dependencies */
            b[j] = a[i] + j;
            
            /* Anti-dependency in inner loop */
            int temp = b[j];
            b[j] = temp * (j + 1);
            
            /* Output dependency */
            a[i] = b[j] % 256;
            a[i] = a[i] + j;
            
            sum += b[j];
        }
        
        /* Cross-iteration dependency */
        if (i > 0) {
            b[0] = a[i] - a[i-1];
        }
    }
    
    g_volatile = sum;
    return sum;
}

/* Test 7: Complex Loop-Carried Dependencies */
int test_loop_carried_deps(int *a, int *b, int n) {
    int sum = 0;
    
    /* Multiple loop-carried dependencies with different distances */
    for (int i = 4; i < n; i++) {
        /* Distance 1, 2, and 4 dependencies */
        a[i] = a[i-1] + a[i-2] + a[i-4];
        
        /* WAR with loop-carried */
        int t1 = b[i-1];
        b[i] = a[i] * t1;
        
        /* WAW with distance 2 */
        if (i % 2 == 0) {
            a[i-2] = b[i] + 1;
        }
        
        /* Memory operation that may alias */
        g_global_array[i % 1024] = a[i] + b[i];
        
        /* Volatile access creates hard dependency */
        sum += g_volatile;
        
        /* Complex floating point chain */
        double d1 = (double)a[i];
        double d2 = d1 * 1.61803398875; /* golden ratio */
        sum += (int)d2;
    }
    
    return sum;
}

/* Main function that runs all tests */
int main(int argc, char **argv) {
    /* Use command line or runtime value for iteration counts */
    int n = 1000;
    int m = 100;
    
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = 1000;
    }
    if (argc > 2) {
        m = atoi(argv[2]);
        if (m <= 0) m = 100;
    }
    
    /* Allocate arrays with dynamic sizes to prevent optimization */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    float *fa = (float*)malloc(n * sizeof(float));
    float *fb = (float*)malloc(n * sizeof(float));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < n; i++) {
        a[i] = (i * 3) % 97;
        b[i] = (i * 7) % 101;
        c[i] = (i * 11) % 103;
        fa[i] = (float)(i % 50);
        fb[i] = (float)(i % 60);
    }
    
    int total_sum = 0;
    
    /* Run all test functions to create various DDG edges */
    total_sum += test_raw_dep(a, b, n);
    total_sum += test_war_waw_dep(a, b, n);
    total_sum += test_memory_aliasing(a, b, c, n);
    total_sum += test_control_dep(a, b, n);
    total_sum += test_mixed_deps(a, b, fa, fb, n);
    total_sum += test_nested_loop_deps(a, b, n/10, m);
    total_sum += test_loop_carried_deps(a, b, n);
    
    /* Use the result to prevent dead code elimination */
    printf("Result checksum: %d\n", total_sum % 1000000);
    
    /* Store to global to ensure side effects */
    g_result = total_sum;
    
    free(a);
    free(b);
    free(c);
    free(fa);
    free(fb);
    
    return g_result != 0 ? 0 : 1;
}
