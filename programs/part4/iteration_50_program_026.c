/* test_ddg_coverage.c
 * Designed to trigger create_ddg_edge() logic in GCC's DDG module
 * Compile with: gcc -O2 -fmodulo-sched -funroll-loops -c test_ddg_coverage.c
 * Or: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -ftree-vectorize -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_array[1024];
int g_result = 0;

/* Function prototypes */
int test_raw_dep(int *a, int *b, int n);
int test_war_waw_dep(int *a, int *b, int n);
int test_memory_aliasing(int *a, int *b, int *c, int n);
int test_control_dep(int *a, int *b, int n);
int test_mixed_deps(float *fa, int *ia, int n);
int test_nested_loops(int *a, int *b, int n);
int test_loop_carried_dep(int *a, int n);

/* Test 1: True Data Dependencies (RAW/flow dependencies) */
int test_raw_dep(int *a, int *b, int n) {
    int sum = 0;
    
    /* Multiple RAW dependencies with different distances */
    for (int i = 2; i < n; i++) {
        /* Distance 1 RAW dependency */
        a[i] = a[i-1] + b[i];
        
        /* Distance 2 RAW dependency */
        b[i] = a[i-2] * 3;
        
        /* Floating point RAW with different latency */
        float temp = (float)a[i] / 2.0f;
        sum += (int)(temp * 100.0f);
    }
    
    /* Prevent dead code elimination */
    g_volatile = sum;
    return sum;
}

/* Test 2: Anti (WAR) and Output (WAW) Dependencies */
int test_war_waw_dep(int *a, int *b, int n) {
    int sum = 0;
    
    for (int i = 1; i < n; i++) {
        /* WAR (anti-dependency): read a[i] before writing it */
        int temp = a[i] + g_volatile;
        
        /* WAW (output-dependency): multiple writes to same location */
        a[i] = b[i] * 2;
        a[i] = a[i] + temp;  /* Overwrites previous value */
        
        /* Another WAR with b[i] */
        b[i-1] = a[i] + 1;
        
        /* Complex WAW with different expressions */
        if (i % 3 == 0) {
            a[i] = temp * 2;
        } else {
            a[i] = temp / 2;
        }
        
        sum += a[i];
    }
    
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
        /* Potential memory dependency */
        *p = i * 2;
        sum += *q;  /* Could read from same location if p==q */
        
        /* Alternate pointers to create ambiguity */
        if (i % 2 == 0) {
            p = &a[i];
            q = &b[i];
        } else {
            p = &b[i];
            q = &c[i];
        }
        
        /* More memory operations with potential aliasing */
        c[i] = a[i] + b[i];
        a[i] = c[i] * 2;
    }
    
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
        } else if (a[i] < -10) {
            b[i] = a[i] / 2;
            sum -= b[i];
        } else {
            b[i] = 0;
            /* Nested condition for more complex control flow */
            if (i % 7 == 0) {
                sum += g_volatile;
            }
        }
        
        /* Another control-dependent operation */
        switch (i % 4) {
            case 0: a[i] = sum + 1; break;
            case 1: a[i] = sum - 1; break;
            case 2: a[i] = sum * 2; break;
            default: a[i] = sum / 2; break;
        }
    }
    
    return sum;
}

/* Test 5: Mixed Data Types and Dependencies */
int test_mixed_deps(float *fa, int *ia, int n) {
    float fsum = 0.0f;
    int isum = 0;
    
    for (int i = 1; i < n; i++) {
        /* Mixed integer/float RAW */
        fa[i] = fa[i-1] * 1.5f;
        
        /* Integer to float conversion dependency */
        float temp = (float)ia[i] + fa[i];
        
        /* Float to integer conversion dependency */
        ia[i] = (int)(temp * 100.0f);
        
        /* More mixed operations */
        if (fa[i] > 100.0f) {
            ia[i] = ia[i-1] + 1;
        }
        
        fsum += fa[i];
        isum += ia[i];
    }
    
    /* Use both results to prevent elimination */
    g_volatile = isum;
    return (int)fsum + isum;
}

/* Test 6: Nested Loops with Complex Dependencies */
int test_nested_loops(int *a, int *b, int n) {
    int sum = 0;
    
    /* Outer loop with carried dependency */
    for (int i = 1; i < n; i++) {
        int inner_sum = 0;
        
        /* Inner loop with multiple dependency types */
        for (int j = 0; j < 8; j++) {
            /* RAW within inner loop */
            b[j] = a[j] + i;
            
            /* WAR in inner loop */
            a[j] = b[j] * 2;
            
            /* WAW in inner loop */
            inner_sum = inner_sum + a[j];
            inner_sum = inner_sum * 2;  /* Overwrites inner_sum */
        }
        
        /* Loop-carried dependency across outer iterations */
        a[i] = a[i-1] + inner_sum;
        sum += a[i];
    }
    
    return sum;
}

/* Test 7: Loop-Carried Dependencies with Different Distances */
int test_loop_carried_dep(int *a, int n) {
    int sum = 0;
    
    /* Multiple loop-carried dependencies */
    for (int i = 4; i < n; i++) {
        /* Distance 1, 2, and 4 dependencies */
        a[i] = a[i-1] + a[i-2] + a[i-4];
        
        /* More complex carried dependency chain */
        int temp1 = a[i-1] * 3;
        int temp2 = a[i-2] * 2;
        int temp3 = a[i-3] + g_volatile;
        
        /* Create longer dependency chain */
        for (int j = 0; j < 3; j++) {
            temp1 = temp1 + temp2;
            temp2 = temp3 - temp1;
            temp3 = temp1 * temp2;
        }
        
        a[i] = a[i] + temp1 + temp2 + temp3;
        sum += a[i];
    }
    
    return sum;
}

/* Main function that runs all tests */
int main(int argc, char **argv) {
    int n = 1000;
    int result = 0;
    
    /* Dynamically allocate to prevent static analysis */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    float *fa = (float*)malloc(n * sizeof(float));
    
    /* Initialize with non-constant values */
    for (int i = 0; i < n; i++) {
        a[i] = (i * 3) % 97;
        b[i] = (i * 7) % 101;
        c[i] = (i * 11) % 103;
        fa[i] = (float)(i % 50) * 1.1f;
    }
    
    /* Run all tests to create various DDG edges */
    result += test_raw_dep(a, b, n);
    result += test_war_waw_dep(a, b, n);
    result += test_memory_aliasing(a, b, c, n);
    result += test_control_dep(a, b, n);
    result += test_mixed_deps(fa, a, n);
    result += test_nested_loops(a, b, n / 10);
    result += test_loop_carried_dep(a, n);
    
    /* Store to global to prevent elimination */
    g_result = result;
    
    /* Print result to ensure code isn't dead */
    printf("Result: %d\n", result);
    
    /* Clean up */
    free(a);
    free(b);
    free(c);
    free(fa);
    
    return result != 0 ? 0 : 1;
}
