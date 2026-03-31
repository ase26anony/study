/* Program to trigger modulo scheduling debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling for this function */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
void modulo_sched_loop(int *a, int *b, int n) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    volatile int temp;  /* Prevent optimization */
    
    /* Outer loop to provide multiple scheduling contexts */
    for (int outer = 0; outer < 10; outer++) {
        /* Critical inner loop with carried dependency */
        /* Fixed small iteration count for manageable scheduling */
        for (int i = 0; i < 32; i++) {
            /* Complex arithmetic with true data dependency chain */
            /* sum depends on previous iteration's sum */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional operations to increase instruction count */
            sum = sum ^ (a[i] & 0xFF);
            sum = sum + (b[i] % 3);
        }
        
        /* Modify input slightly to create loop-variant behavior */
        /* This prevents the outer loop from being optimized away */
        temp = sum;
        b[0] += temp % 7;
        a[outer % 32] = sum & 0xFF;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Final sum: %d\n", sum);
}

int main() {
    int a[128], b[128];
    
    /* Initialize with pseudo-random values */
    /* Using volatile to prevent constant propagation */
    srand(time(NULL));
    volatile int seed = rand();
    
    for (int i = 0; i < 128; i++) {
        a[i] = (rand() ^ seed) % 100;
        b[i] = (rand() ^ seed) % 100;
    }
    
    /* Call the function with modulo scheduling candidate */
    modulo_sched_loop(a, b, 128);
    
    return 0;
}
