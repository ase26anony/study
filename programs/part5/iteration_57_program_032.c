/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 100

/* Global invariant value to create cross-iteration dependencies */
int GLOBAL_INVARIANT = 7;

/* Hot function with complex loop for modulo scheduling */
__attribute__((hot, noinline))
int compute_hot_loop(int* a, int* b, int* c, int n) {
    int result = 0;
    
    for (int i = 1; i < n - 1; i++) {
        /* Declare many temporaries to create register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        int t10, t11, t12, t13, t14, t15, t16, t17, t18, t19;
        
        /* Chain of dependent operations with loop-invariant */
        t0 = a[i] + GLOBAL_INVARIANT;          /* Use invariant */
        t1 = t0 * b[i];                        /* Dependent on t0 */
        t2 = t1 - c[i];                        /* Dependent on t1 */
        t3 = t2 / 3;                           /* Division - higher latency */
        t4 = t3 + a[i-1];                      /* Array access with offset */
        t5 = t4 * t0;                          /* Mix with earlier temp */
        t6 = t5 - b[i+1];                      /* Forward array access */
        t7 = t6 % 17;                          /* Modulo - higher latency */
        t8 = t7 + GLOBAL_INVARIANT;            /* Another invariant use */
        t9 = t8 * t2;                          /* Cross dependency */
        
        /* More operations to extend dependency chain */
        t10 = t9 / 5;
        t11 = t10 + c[i-1];
        t12 = t11 * t5;
        t13 = t12 - a[i+1];
        t14 = t13 % 13;
        t15 = t14 + t8;
        t16 = t15 * t3;
        t17 = t16 - b[i];
        t18 = t17 / 7;
        t19 = t18 + t11;
        
        /* Conditional to create multiple basic blocks */
        if (t5 & 1) {
            /* True path with computations */
            t19 = t19 * 2 + GLOBAL_INVARIANT;
            t19 = t19 - t7;
            result += t19 / 3;  /* Division in conditional path */
        } else {
            /* False path with different computations */
            t19 = t19 / 2 + GLOBAL_INVARIANT;
            t19 = t19 * t14;
            result += t19 % 5;  /* Modulo in conditional path */
        }
        
        /* Additional invariant use after conditional */
        result += t19 * GLOBAL_INVARIANT;
    }
    
    return result;
}

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int* a, int* b, int* c, int n) {
    unsigned int seed = 12345;
    for (int i = 0; i < n; i++) {
        seed = seed * 1103515245 + 12345;
        a[i] = (seed >> 16) & 0x7FFF;
        seed = seed * 1103515245 + 12345;
        b[i] = (seed >> 16) & 0x7FFF;
        seed = seed * 1103515245 + 12345;
        c[i] = (seed >> 16) & 0x7FFF;
    }
}

int main() {
    int a[SIZE], b[SIZE], c[SIZE];
    int total_result = 0;
    
    /* Initialize arrays */
    init_arrays(a, b, c, SIZE);
    
    /* Call hot function multiple times to ensure it's compiled as hot */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Vary inputs slightly each iteration */
        a[iter % SIZE] += iter;
        b[iter % SIZE] -= iter;
        
        total_result += compute_hot_loop(a, b, c, SIZE);
        
        /* Prevent compiler from optimizing away the loop */
        if (total_result & 0x10000000) {
            total_result >>= 1;
        }
    }
    
    printf("Result: %d\n", total_result);
    return 0;
}
