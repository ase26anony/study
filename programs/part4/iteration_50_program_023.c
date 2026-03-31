/* test_ddg_coverage.c
 * This program creates various loop patterns to trigger DDG edge creation
 * in GCC's instruction scheduler, specifically targeting create_ddg_edge()
 * lines 749-757 in ddg.cc
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
int test_war_waw_dep(int *a, int *b, int n);
int test_memory_aliasing(int *a, int *b, int *c, int n);
int test_control_dep(int *a, int *b, int n);
int test_mixed_deps(int *a, int *b, int *c, int n);
int test_loop_carried(int *a, int *b, int n);
int test_complex_nested(int *a, int *b, int n);

/* Test 1: True Data Dependencies (RAW) with loop-carried dependency */
int test_raw_dep(int *a, int *b, int n) {
    int sum = 0;
    
    /* Multiple RAW dependencies with different distances */
    for (int i = 2; i < n; i++) {
        /* Distance 1 RAW dependency */
        a[i] = a[i-1] + b[i];
        
        /* Distance 2 RAW dependency */
        b[i] = a[i-2] * 3;
        
        /* Floating point RAW to create different data type edges */
        float temp = (float)a[i] / 2.0f;
        sum += (int)temp;
        
        /* Another RAW chain */
        a[i] = a[i] + g_volatile;
    }
    
    return sum;
}

/* Test 2: Anti (WAR) and Output (WAW) Dependencies */
int test_war_waw_dep(int *a, int *b, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        int t1 = a[i];      /* Read a[i] */
        a[i] = b[i] + 1;    /* Write a[i] - WAR with previous read */
        
        int t2 = a[i];      /* Read a[i] again */
        a[i] = t1 + t2;     /* WAW: Second write to a[i] in same iteration */
        
        /* More complex WAR/WAW patterns */
        b[i] = a[i] * 2;    /* WAR: a[i] was just written, now read */
        a[i] = b[i] / 3;    /* WAW: a[i] written again */
        
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
    
    /* Randomize which pointer we use to create ambiguity */
    for (int i = 0; i < n; i++) {
        /* The compiler can't tell if p and q alias */
        if (i % 3 == 0) {
            q = a + (i % 10);  /* q might point to a */
        } else if (i % 3 == 1) {
            q = b + (i % 10);  /* q might point to b */
        } else {
            q = c + (i % 10);  /* q might point to c */
        }
        
        /* Memory operations with potential aliasing */
        *p = *q + i;
        p[i % 5] = q[i % 3] * 2;
        
        /* Volatile access creates hard memory dependency */
        g_array[i % 1024] = *p + g_volatile;
        
        sum += *p + *q;
        
        /* Move p around */
        p = a + ((i + 1) % n);
    }
    
    return sum;
}

/* Test 4: Control Dependencies */
int test_control_dep(int *a, int *b, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Complex condition creating control dependencies */
        if (a[i] > 0) {
            if (b[i] % 2 == 0) {
                a[i] = a[i] * 2 + g_volatile;
                sum += a[i];
            } else {
                a[i] = a[i] / 2 - g_volatile;
                sum -= a[i];
            }
            
            /* Nested control flow */
            for (int j = 0; j < 3; j++) {
                if ((i + j) % 4 == 0) {
                    b[i] += j * a[i];
                }
            }
        } else {
            a[i] = b[i] * 3;
            sum += b[i];
            
            /* Switch-like control */
            switch (i % 4) {
                case 0: b[i] += 1; break;
                case 1: b[i] += 2; break;
                case 2: b[i] += 3; break;
                default: b[i] += 4; break;
            }
        }
        
        /* Loop with break/continue creating control edges */
        for (int k = 0; k < 5; k++) {
            if (k == a[i] % 5) {
                break;
            }
            sum += k;
        }
    }
    
    return sum;
}

/* Test 5: Mixed Dependencies */
int test_mixed_deps(int *a, int *b, int *c, int n) {
    int sum = 0;
    
    for (int i = 3; i < n; i++) {
        /* RAW with distance 3 */
        float f1 = (float)a[i-3] / 7.0f;
        
        /* WAR */
        int temp = b[i];
        b[i] = (int)f1 * 11;
        
        /* WAW */
        a[i] = temp + i;
        a[i] = a[i] * 2;  /* Second write to a[i] */
        
        /* Memory dependency with pointer */
        int *ptr = &c[i % 128];
        *ptr = a[i] + b[i];
        
        /* Control dependency */
        if (a[i] > 1000) {
            *ptr /= 2;
        }
        
        /* Function call acting as memory clobber */
        sum += *ptr + g_volatile;
        
        /* Another RAW chain mixing types */
        double d1 = (double)a[i] * 1.5;
        c[i % 128] = (int)d1;
    }
    
    return sum;
}

/* Test 6: Complex Loop-Carried Dependencies */
int test_loop_carried(int *a, int *b, int n) {
    int sum = 0;
    
    /* Multiple interleaved loop-carried dependencies */
    for (int i = 4; i < n; i++) {
        /* Distance 4 RAW */
        a[i] = a[i-4] + b[i-2];
        
        /* Distance 2 RAW with different data type */
        float f_val = (float)b[i-2] * 0.25f;
        b[i] = (int)f_val + a[i-1];
        
        /* Distance 3 anti-dependency */
        int temp = a[i-3];
        a[i-3] = b[i] * 2;
        sum += temp;
        
        /* Output dependency with volatile */
        g_array[i % 1024] = sum;
        g_array[i % 1024] = sum + g_volatile;  /* WAW */
        
        /* Complex expression with multiple uses */
        sum = (sum * 1103515245 + 12345) & 0x7fffffff;
    }
    
    return sum;
}

/* Test 7: Nested Loops with Complex Dependencies */
int test_complex_nested(int *a, int *b, int n) {
    int sum = 0;
    
    for (int i = 1; i < n; i++) {
        int inner_sum = 0;
        
        /* Inner loop with dependencies on outer loop index */
        for (int j = 0; j < 8; j++) {
            /* RAW across inner iterations */
            if (j > 0) {
                a[i * 8 + j] = a[i * 8 + j - 1] + b[j];
            } else {
                a[i * 8 + j] = b[j] + i;
            }
            
            /* WAR in inner loop */
            int temp = b[j];
            b[j] = a[i * 8 + j] * 3;
            inner_sum += temp;
            
            /* Control in inner loop */
            if ((i + j) % 3 == 0) {
                a[i * 8 + j] += inner_sum;
            }
            
            /* Memory operation */
            g_array[(i * 8 + j) % 1024] = a[i * 8 + j];
        }
        
        /* Outer loop dependency on inner computation */
        sum += inner_sum * i;
        
        /* Outer loop-carried dependency */
        b[i % 8] = sum % 100;
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    const int N = 1000;
    int total_result = 0;
    
    /* Allocate arrays with different alignments */
    int *array1 = (int*)aligned_alloc(64, N * sizeof(int));
    int *array2 = (int*)aligned_alloc(64, N * sizeof(int));
    int *array3 = (int*)aligned_alloc(64, N * sizeof(int));
    
    if (!array1 || !array2 || !array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
        array3[i] = rand() % 1000;
        g_array[i % 1024] = rand() % 1000;
    }
    
    /* Force initialization of volatile */
    g_volatile = rand() % 100;
    
    printf("Starting DDG edge creation tests...\n");
    
    /* Run all tests multiple times to increase coverage chance */
    for (int iter = 0; iter < 3; iter++) {
        total_result ^= test_raw_dep(array1, array2, N);
        total_result ^= test_war_waw_dep(array1, array2, N);
        total_result ^= test_memory_aliasing(array1, array2, array3, N);
        total_result ^= test_control_dep(array1, array2, N);
        total_result ^= test_mixed_deps(array1, array2, array3, N);
        total_result ^= test_loop_carried(array1, array2, N);
        total_result ^= test_complex_nested(array1, array2, N / 8);
        
        /* Modify inputs slightly each iteration */
        array1[iter] = total_result;
        g_volatile = (g_volatile + 1) % 256;
    }
    
    printf("Final checksum: %d\n", total_result);
    printf("Tests completed. Compile with:\n");
    printf("  gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -funroll-loops -c test_ddg_coverage.c\n");
    printf("  gcc -O2 -fmodulo-sched -march=native -c test_ddg_coverage.c\n");
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    return total_result != 0 ? 0 : 1;
}
