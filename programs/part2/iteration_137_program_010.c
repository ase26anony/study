/* Program to trigger modulo scheduling debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling to focus on modulo scheduling */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
void modulo_sched_loop(int *a, int *b, int n) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    volatile int temp;  /* Prevent optimizations */
    
    /* Outer loop to provide multiple scheduling contexts */
    for (int outer = 0; outer < 10; outer++) {
        /* Critical inner loop with carried dependency */
        /* Fixed small iteration count for modulo scheduling analysis */
        for (int i = 0; i < 32; i++) {
            /* Complex recurrence with multiple operations */
            /* sum = (sum * a[i] + b[i]) >> 1 */
            int prod = sum * a[i];
            int sum_temp = prod + b[i];
            sum = sum_temp >> 1;
            
            /* Additional operations to increase instruction count */
            sum = sum ^ (a[i] & 0xFF);  /* XOR operation */
            sum = sum + (i & 1);        /* Conditional add */
        }
        
        /* Modify input slightly for outer loop variation */
        b[0] += sum;
        
        /* Use volatile to prevent dead code elimination */
        temp = sum;
    }
    
    /* Final output to prevent elimination */
    printf("Final sum: %d\n", sum);
}

int main() {
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    
    /* Arrays with compile-time known size */
    int a[128], b[128];
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < 128; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    /* Call the function with carried dependency loop */
    modulo_sched_loop(a, b, 128);
    
    return 0;
}
