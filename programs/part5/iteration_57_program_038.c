/* test_modulo_sched.c
 * Designed to trigger move edge logging in GCC's modulo scheduler
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fdump-rtl-sms -fdump-rtl-sms-details test_modulo_sched.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Loop-invariant value that will create cross-iteration dependencies */
static int GLOBAL_INVARIANT = 7;

/* Non-inlineable function with higher latency */
__attribute__((noinline, cold)) 
int high_latency_op(int x, int y) {
    /* Division has higher latency than basic arithmetic */
    return (x % y) ? (x / y) : (x * y);
}

/* Hot function containing the target loop */
__attribute__((hot, noinline))
int target_loop(int* a, int* b, int* c, int n) {
    int result = 0;
    
    /* Loop with high register pressure and dependencies */
    for (int i = 2; i < n - 2; i++) {
        /* Many scalar temporaries creating long dependency chain */
        int t0 = a[i] + GLOBAL_INVARIANT;      /* Uses invariant value */
        int t1 = t0 * b[i];
        int t2 = t1 - c[i];
        int t3 = t2 + a[i+1];                  /* Array access with offset */
        int t4 = t3 * b[i-1];                  /* Another offset access */
        int t5 = high_latency_op(t4, 3);       /* Higher latency operation */
        int t6 = t5 + c[i+1];
        int t7 = t6 * GLOBAL_INVARIANT;        /* Another invariant use */
        int t8 = t7 - a[i-1];
        int t9 = t8 * t5;
        
        /* Conditional to create multiple basic blocks */
        if (t9 & 1) {
            /* Path 1: More computations */
            int t10 = t9 + b[i+2];
            int t11 = t10 * c[i-1];
            int t12 = high_latency_op(t11, 5);
            result += t12;
        } else {
            /* Path 2: Different computations */
            int t10 = t9 - b[i-2];
            int t11 = t10 / 2;                 /* Division */
            int t12 = t11 * c[i+2];
            result += t12;
        }
        
        /* Additional computations to increase register pressure */
        int t13 = a[i] * b[i] + c[i];
        int t14 = t13 - result;
        int t15 = high_latency_op(t14, 2);
        result ^= t15;
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
    const int SIZE = 1024;
    int* a = malloc(SIZE * sizeof(int));
    int* b = malloc(SIZE * sizeof(int));
    int* c = malloc(SIZE * sizeof(int));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(a, b, c, SIZE);
    
    /* Call hot function multiple times to ensure it's compiled */
    int total_result = 0;
    for (int iter = 0; iter < 10; iter++) {
        /* Modify invariant slightly each iteration */
        GLOBAL_INVARIANT = 7 + (iter % 3);
        total_result += target_loop(a, b, c, SIZE);
    }
    
    printf("Result: %d\n", total_result);
    
    free(a);
    free(b);
    free(c);
    
    return 0;
}
