/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 100

/* Loop-invariant value that will be used across iterations */
static int GLOBAL_INVARIANT = 7;

/* Hot function with complex loop for modulo scheduling */
__attribute__((hot, noinline))
int compute_hot_loop(int* a, int* b, int* c, int n) {
    int result = 0;
    
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries creating register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        int t10, t11, t12, t13, t14, t15, t16, t17, t18, t19;
        
        /* Start chain with loop-invariant value - may create move edges */
        t0 = GLOBAL_INVARIANT + a[i];
        
        /* Long dependency chain with mixed operations */
        t1 = t0 * b[i];
        t2 = t1 - c[i];
        t3 = t2 + a[i-1];      /* Array access with offset */
        t4 = t3 * GLOBAL_INVARIANT;  /* Another use of invariant */
        t5 = t4 / 3;           /* Higher latency division */
        t6 = t5 + b[i+1];      /* Array access with forward offset */
        t7 = t6 * t0;          /* Cross-iteration dependency potential */
        t8 = t7 - c[i-1];      /* Array access with backward offset */
        t9 = t8 * 2;
        
        /* More operations to increase register pressure */
        t10 = t9 + a[i];
        t11 = t10 * t1;
        t12 = t11 - t2;
        t13 = t12 + t3;
        t14 = t13 * t4;
        t15 = t14 / 5;         /* Another higher latency operation */
        t16 = t15 + t5;
        t17 = t16 * t6;
        t18 = t17 - t7;
        t19 = t18 + t8;
        
        /* Conditional creating multiple basic blocks */
        if (t19 & 1) {
            /* True path with more computations */
            t19 = t19 * 3 + GLOBAL_INVARIANT;
            t19 = t19 - a[i] / 2;  /* Mixed latency operations */
        } else {
            /* False path also with computations */
            t19 = t19 / 2 + b[i];
            t19 = t19 * GLOBAL_INVARIANT;  /* Another invariant use */
        }
        
        /* Final accumulation with cross-iteration dependency */
        result += t19 * (i % 8);
    }
    
    return result;
}

/* Non-inlineable function with higher latency */
__attribute__((noinline, cold))
int higher_latency_op(int x, int y) {
    /* Complex enough to not be inlined */
    return (x * y) / (x - y + 1);
}

/* Another hot function with different pattern */
__attribute__((hot, noinline))
int compute_hot_loop2(int* a, int* b, int* c, int n) {
    int result = 0;
    int invariant = n * 2;  /* Function argument as invariant */
    
    for (int i = 2; i < n - 2; i++) {
        /* Another set of temporaries */
        int s0, s1, s2, s3, s4, s5, s6, s7, s8, s9;
        
        /* Chain starting with invariant */
        s0 = invariant + a[i];
        s1 = s0 * b[i-1];
        s2 = higher_latency_op(s1, c[i]);  /* Higher latency call */
        s3 = s2 + a[i+1];
        s4 = s3 * invariant;
        s5 = s4 % 17;          /* Modulo operation - potentially higher latency */
        s6 = s5 + b[i];
        s7 = s6 * s0;
        s8 = s7 - c[i-2];
        s9 = s8 * 3;
        
        /* Conditional with both paths having computations */
        if (s9 > 0) {
            s9 = s9 + higher_latency_op(a[i], b[i]);
        } else {
            s9 = s9 - (c[i] % 5);
        }
        
        result ^= s9;  /* Different accumulation */
    }
    
    return result;
}

/* Simple deterministic RNG for array initialization */
static unsigned int seed = 12345;
static inline int rand_int(void) {
    seed = seed * 1103515245 + 12345;
    return (int)(seed >> 16) & 0x7FFF;
}

int main(void) {
    /* Allocate and initialize arrays */
    int* a = malloc(SIZE * sizeof(int));
    int* b = malloc(SIZE * sizeof(int));
    int* c = malloc(SIZE * sizeof(int));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with deterministic pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand_int() % 1000;
        b[i] = rand_int() % 1000;
        c[i] = rand_int() % 1000;
    }
    
    /* Call hot functions multiple times to ensure execution */
    int total_result = 0;
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Vary the invariant slightly each iteration */
        GLOBAL_INVARIANT = (iter % 13) + 5;
        
        /* Call both hot functions */
        total_result += compute_hot_loop(a, b, c, SIZE);
        total_result += compute_hot_loop2(a, b, c, SIZE);
        
        /* Modify arrays slightly to prevent complete optimization */
        if (iter % 10 == 0) {
            a[iter % SIZE] = iter;
            b[iter % SIZE] = iter * 2;
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Final result: %d\n", total_result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
