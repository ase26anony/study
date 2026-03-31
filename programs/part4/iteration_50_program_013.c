/* test_ddg_edges.c
 * Program designed to trigger DDG edge creation in GCC's scheduler
 * Compile with: gcc -O2 -fmodulo-sched -funroll-loops -c test_ddg_edges.c
 * Or: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -c test_ddg_edges.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_array[1024];
int g_result = 0;

/* Function prototypes */
int test_raw_dependencies(int *a, int *b, int n);
int test_war_waw_dependencies(int *a, int *b, int n);
int test_memory_aliasing(int *a, int *b, int *c, int n);
int test_control_dependencies(int *a, int *b, int n);
int test_mixed_dependencies(float *fa, int *ia, int n);
int test_loop_carried_dependencies(int *a, int *b, int n);

/* Test 1: True Data Dependencies (RAW/flow dependencies) */
int test_raw_dependencies(int *a, int *b, int n) {
    int sum = 0;
    
    /* Multiple RAW dependencies with different distances */
    for (int i = 2; i < n; i++) {
        /* Flow dependency with distance 1 */
        a[i] = a[i-1] + b[i] + g_volatile;
        
        /* Flow dependency with distance 2 */
        b[i] = a[i-2] * 3 + g_volatile;
        
        /* Chained flow dependencies */
        int temp = a[i] + b[i];
        a[i] = temp * 2;
        b[i] = a[i] + temp;
        
        sum += a[i] + b[i];
    }
    
    /* Another loop with floating point RAW dependencies */
    float fa[256];
    float fb[256];
    for (int i = 1; i < 256; i++) {
        fa[i] = fa[i-1] * 1.5f + fb[i];
        fb[i] = fa[i] * 0.7f;
        sum += (int)fa[i];
    }
    
    return sum;
}

/* Test 2: Anti and Output Dependencies (WAR/WAW) */
int test_war_waw_dependencies(int *a, int *b, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* WAR (anti-dependency): read after write */
        int temp = a[i] + g_volatile;  /* Read a[i] */
        a[i] = b[i] * 2;               /* Write a[i] - anti-dep with previous read */
        b[i] = temp + a[i];            /* Use the read value */
        
        /* WAW (output dependency): write after write */
        a[i] = temp * 3;               /* First write to a[i] */
        a[i] = b[i] + 1;               /* Second write to a[i] - output dep */
        
        /* Mixed dependencies */
        int x = a[i];
        a[i] = x + b[i];               /* WAW with previous a[i] write */
        b[i] = x * 2;                  /* WAR with x read */
        
        sum += a[i] + b[i];
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
        /* Potential memory dependencies due to aliasing */
        *p = *q + g_volatile;      /* Read from q, write to p */
        *q = *p * 2;               /* Read from p (may alias with q), write to q */
        
        /* Array accesses with variable indices - may alias */
        int idx1 = i % 64;
        int idx2 = (i * 7) % 64;
        a[idx1] = b[idx2] + c[i];
        b[idx2] = a[idx1] * 3;
        
        /* Pointer arithmetic creating potential aliasing */
        p = &a[i % 16];
        q = &b[i % 16];
        *p = *q + i;
        *q = *p - i;
        
        sum += a[i % 16] + b[i % 16];
    }
    
    return sum;
}

/* Test 4: Control Dependencies */
int test_control_dependencies(int *a, int *b, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Control-dependent computations */
        if (a[i] > 0) {
            b[i] = a[i] * 2 + g_volatile;
            a[i] = b[i] / 3;
        } else {
            b[i] = a[i] * 3 - g_volatile;
            a[i] = b[i] / 2;
        }
        
        /* Nested control flow */
        for (int j = 0; j < 4; j++) {
            if ((i + j) % 3 == 0) {
                a[i] += j * b[i];
            } else {
                b[i] -= j * a[i];
            }
        }
        
        /* Conditional with side effects */
        int temp = (i % 2 == 0) ? a[i] * 2 : b[i] * 3;
        sum += temp + g_volatile;
    }
    
    return sum;
}

/* Test 5: Mixed Data Types and Dependencies */
int test_mixed_dependencies(float *fa, int *ia, int n) {
    float fsum = 0.0f;
    int isum = 0;
    
    for (int i = 1; i < n; i++) {
        /* Mixed float/int operations creating various edge types */
        fa[i] = fa[i-1] * 1.1f + (float)ia[i];
        ia[i] = (int)fa[i] * 2 + g_volatile;
        
        /* Cross-type dependencies */
        float ftemp = fa[i] * 0.5f;
        ia[i] = (int)ftemp + ia[i-1];  /* RAW with ia[i-1] */
        fa[i] = ftemp + (float)ia[i];  /* WAR with ftemp */
        
        /* Output dependency on float array */
        fa[i] = ftemp * 2.0f;
        fa[i] = fa[i] + 1.0f;          /* WAW on fa[i] */
        
        fsum += fa[i];
        isum += ia[i];
    }
    
    return isum + (int)fsum;
}

/* Test 6: Loop-Carried Dependencies with Different Distances */
int test_loop_carried_dependencies(int *a, int *b, int n) {
    int sum = 0;
    
    /* Distance 1 loop-carried dependency */
    for (int i = 1; i < n; i++) {
        a[i] = a[i-1] + b[i] * 2;
        sum += a[i];
    }
    
    /* Distance 2 loop-carried dependency */
    for (int i = 2; i < n; i++) {
        b[i] = a[i-2] * 3 + b[i-1];
        sum += b[i];
    }
    
    /* Distance 3 with multiple dependencies */
    for (int i = 3; i < n; i++) {
        int temp = a[i-1] + b[i-2];
        a[i] = temp * a[i-3];
        b[i] = a[i] + b[i-1];
        sum += a[i] + b[i];
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
    
    /* Initialize arrays with non-constant values */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    float *fa = (float*)malloc(n * sizeof(float));
    
    for (int i = 0; i < n; i++) {
        a[i] = (i * 3) % 97;
        b[i] = (i * 7) % 113;
        c[i] = (i * 11) % 151;
        fa[i] = (float)(i % 79) * 0.7f;
    }
    
    /* Run all tests to create various DDG edges */
    int result = 0;
    
    result += test_raw_dependencies(a, b, n);
    result += test_war_waw_dependencies(a, b, n);
    result += test_memory_aliasing(a, b, c, n);
    result += test_control_dependencies(a, b, n);
    result += test_mixed_dependencies(fa, a, n);
    result += test_loop_carried_dependencies(a, b, n);
    
    /* Store to global to prevent dead code elimination */
    g_result = result;
    
    /* Print result to ensure code isn't optimized away */
    printf("Result: %d\n", result);
    
    free(a);
    free(b);
    free(c);
    free(fa);
    
    return 0;
}
