/* test_modulo_sched.c
 * Designed to trigger move edge logging in GCC's modulo scheduler
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fdump-rtl-sms -fdump-rtl-sms-details test_modulo_sched.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Loop-invariant value that will be used across iterations */
static int GLOBAL_INVARIANT = 7;

/* Non-inlineable function with higher latency */
__attribute__((noinline, cold)) 
int high_latency_op(int x, int y) {
    /* Division has higher latency than basic arithmetic */
    return (x % y) ? (x / y) : (x * y);
}

/* Hot function containing the target loop */
__attribute__((hot, noinline))
int target_loop(int* a, int* b, int* c, int n, int invariant) {
    int result = 0;
    
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries to create register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain of dependent operations with loop-invariant usage */
        t0 = a[i] + invariant;           /* Use loop-invariant */
        t1 = t0 * b[i];                  /* Dependent on t0 */
        t2 = t1 - c[i];                  /* Dependent on t1 */
        t3 = t2 + a[i-1];                /* Array access with offset */
        t4 = t3 * b[i+1];                /* Array access with offset */
        
        /* High latency operation in the chain */
        t5 = high_latency_op(t4, invariant);  /* Uses invariant again */
        
        t6 = t5 + c[i-1];                /* Another offset access */
        t7 = t6 * t0;                    /* Cross-dependency */
        t8 = t7 - t2;                    /* More dependencies */
        t9 = t8 + a[i+1];                /* Forward offset access */
        
        /* Conditional to create multiple basic blocks */
        if (t5 & 1) {
            /* Path 1: More computations */
            t9 = t9 * 3;
            result += t9 - t3;
        } else {
            /* Path 2: Different computations */
            t9 = t9 / 2;
            result += t9 + t6;
        }
        
        /* Additional dependent operations after conditional */
        int t10 = t9 * invariant;        /* Use invariant again */
        int t11 = t10 + b[i];
        int t12 = t11 - c[i+1];
        int t13 = t12 * a[i];
        int t14 = t13 + t7;
        int t15 = t14 / (invariant + 1);
        
        result += t15;
    }
    
    return result;
}

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int* a, int* b, int* c, int n) {
    int seed = 12345;
    for (int i = 0; i < n; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        a[i] = (seed >> 16) & 0xFF;
        b[i] = (seed >> 8) & 0xFF;
        c[i] = seed & 0xFF;
    }
}

int main() {
    const int N = 1024;
    int* a = malloc(N * sizeof(int));
    int* b = malloc(N * sizeof(int));
    int* c = malloc(N * sizeof(int));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(a, b, c, N);
    
    /* Call hot function multiple times to ensure it's compiled */
    int total_result = 0;
    for (int iter = 0; iter < 10; iter++) {
        /* Vary the invariant slightly each call */
        GLOBAL_INVARIANT = 7 + (iter % 5);
        total_result += target_loop(a, b, c, N, GLOBAL_INVARIANT);
        
        /* Modify arrays slightly to prevent complete optimization */
        a[iter % N] = iter;
        b[(iter + 1) % N] = iter * 2;
    }
    
    printf("Result: %d\n", total_result);
    
    free(a);
    free(b);
    free(c);
    
    return 0;
}
