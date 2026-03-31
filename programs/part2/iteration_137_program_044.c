/* Program to trigger GCC modulo scheduler debug output in ps_insn_find_column */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling to focus on modulo scheduling */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
void modulo_sched_loop(int *a, int *b, int n) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    volatile int v1, v2;  /* Volatile to prevent optimization */
    
    /* Outer loop to provide multiple scheduling contexts */
    for (int outer = 0; outer < 10; outer++) {
        /* Critical inner loop with carried dependency */
        for (int i = 0; i < n; i++) {
            /* Complex recurrence with multiple operations */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional operations to increase instruction count */
            sum ^= (a[i] & 0xFF);
            sum += (b[i] % 16);
        }
        
        /* Modify input slightly to create loop-variant behavior */
        if (outer % 2 == 0) {
            b[0] += sum;
        }
        
        /* Use volatile to prevent dead code elimination */
        v1 = sum;
        v2 = outer;
    }
    
    /* Final use to prevent elimination */
    printf("Final sum: %d\n", sum);
}

int main() {
    const int SIZE = 64;
    int a[SIZE], b[SIZE];
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    /* Call the function with compile-time constant size */
    modulo_sched_loop(a, b, 32);  /* Fixed small iteration count */
    
    return 0;
}
