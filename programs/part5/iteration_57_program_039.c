/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERS 100

/* Global invariant value to force move creation */
int GLOBAL_INVARIANT = 7;

/* Hot function with complex loop for modulo scheduling */
__attribute__((hot, noinline))
int compute_hot(int* a, int* b, int* c, int n) {
    int result = 0;
    
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries creating register pressure */
        int t0 = a[i];
        int t1 = t0 + GLOBAL_INVARIANT;  /* Use invariant - may create move */
        int t2 = t1 * b[i];
        int t3 = t2 - c[i];
        int t4 = t3 + a[i-1];
        int t5 = t4 * b[i+1];
        int t6 = t5 - c[i-1];
        int t7 = t6 + GLOBAL_INVARIANT;  /* Another invariant use */
        int t8 = t7 * 3;
        int t9 = t8 / 2;                 /* Higher latency division */
        int t10 = t9 + a[i];
        int t11 = t10 * 2;
        int t12 = t11 - b[i];
        int t13 = t12 + c[i];
        int t14 = t13 * GLOBAL_INVARIANT; /* Another invariant use */
        
        /* Conditional to create multiple basic blocks */
        if (t5 & 1) {
            /* Path 1: More computations */
            int t15 = t14 + t3;
            int t16 = t15 * 2;
            int t17 = t16 - t7;
            result += t17 % 5;           /* Higher latency modulo */
        } else {
            /* Path 2: Different computations */
            int t15 = t14 - t3;
            int t16 = t15 / 3;           /* Higher latency division */
            int t17 = t16 + t7;
            result += t17 % 3;           /* Higher latency modulo */
        }
        
        /* Additional dependent operations after conditional */
        int t18 = t14 + result;
        int t19 = t18 * GLOBAL_INVARIANT;
        result = t19 & 0xFF;  /* Keep result bounded */
    }
    
    return result;
}

/* Non-inlineable function with higher latency */
__attribute__((noinline, cold))
int slow_operation(int x, int y) {
    /* Complex enough to not be inlined */
    return (x * y) / (x + y + 1);
}

/* Another hot function with different pattern */
__attribute__((hot, noinline))
int compute_hot2(int* a, int* b, int* c, int n) {
    int result = 0;
    int inv = GLOBAL_INVARIANT * 2;  /* Derived invariant */
    
    for (int i = 0; i < n; i++) {
        /* Long dependency chain */
        int t0 = a[i] + inv;
        int t1 = t0 * b[i];
        int t2 = t1 - c[i];
        int t3 = t2 + inv;
        int t4 = t3 * 3;
        int t5 = slow_operation(t4, inv);  /* High latency call */
        int t6 = t5 + a[i];
        int t7 = t6 * 2;
        int t8 = t7 - b[i];
        int t9 = t8 + c[i];
        
        /* Conditional with computations in both paths */
        if (t9 > 0) {
            int t10 = t9 * inv;
            int t11 = t10 % 13;           /* Higher latency */
            result += t11;
        } else {
            int t10 = t9 / 5;             /* Higher latency */
            int t11 = t10 + inv;
            result -= t11;
        }
    }
    
    return result;
}

/* Simple deterministic RNG for array initialization */
unsigned int simple_rand(unsigned int* seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7FFFFFFF;
    return *seed;
}

int main() {
    int a[SIZE], b[SIZE], c[SIZE];
    unsigned int seed = 123456789;
    
    /* Initialize arrays with deterministic pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = simple_rand(&seed) % 100;
        b[i] = simple_rand(&seed) % 100;
        c[i] = simple_rand(&seed) % 100;
    }
    
    int total = 0;
    
    /* Call hot functions multiple times to ensure execution */
    for (int iter = 0; iter < ITERS; iter++) {
        total += compute_hot(a, b, c, SIZE);
        total += compute_hot2(a, b, c, SIZE);
        
        /* Modify arrays slightly to avoid complete optimization */
        a[iter % SIZE] = iter;
        b[iter % SIZE] = total & 0xFF;
    }
    
    printf("Result: %d\n", total);
    return 0;
}
