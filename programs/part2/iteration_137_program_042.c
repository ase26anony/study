/* Program to trigger modulo scheduling debug output in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling for this function */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int modulo_sched_test(int *a, int *b, int size) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    int i, j;
    
    /* Outer loop to provide multiple scheduling contexts */
    for (j = 0; j < 10; j++) {
        /* Inner loop with carried dependency for modulo scheduling */
        /* Fixed small iteration count for manageable scheduling */
        for (i = 0; i < 32; i++) {
            /* Complex arithmetic with true data dependency chain */
            /* sum depends on previous iteration's sum */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional operations to increase instruction count */
            sum = sum ^ (a[i] & 0xFF);
            sum = sum + (b[i] % 16);
        }
        
        /* Modify input slightly to create loop-variant behavior */
        /* Prevents complete optimization away */
        b[0] += sum & 1;
        a[1] ^= sum & 0xF;
    }
    
    return sum;
}

/* Another test function with different pattern */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int modulo_sched_test2(int *arr, int n) {
    int acc1 = arr[0];
    int acc2 = arr[1];
    int i, j;
    
    for (j = 0; j < 8; j++) {
        /* Loop with multiple interleaved dependencies */
        for (i = 2; i < 34; i++) {
            /* Two parallel recurrence chains */
            acc1 = (acc1 * 3 + arr[i]) % 1000;
            acc2 = (acc2 + acc1 * 2) >> 1;
            
            /* Cross dependency between chains */
            int temp = acc1 ^ acc2;
            acc1 = acc1 + (temp & 0xF);
            acc2 = acc2 - (temp & 0x7);
        }
        
        /* Slight modification to prevent dead code elimination */
        arr[2] += acc1;
        arr[3] += acc2;
    }
    
    return acc1 + acc2;
}

int main() {
    int a[128], b[128];
    int i;
    
    /* Initialize with pseudo-random values */
    /* Using volatile to prevent constant propagation */
    volatile int seed = time(NULL);
    srand(seed);
    
    for (i = 0; i < 128; i++) {
        a[i] = rand() % 256;
        b[i] = rand() % 256;
    }
    
    /* Call test functions */
    int result1 = modulo_sched_test(a, b, 128);
    
    /* Re-initialize for second test */
    for (i = 0; i < 128; i++) {
        a[i] = rand() % 256;
        b[i] = rand() % 256;
    }
    
    int result2 = modulo_sched_test2(a, 128);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    return 0;
}
