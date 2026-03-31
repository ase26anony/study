/* Program to trigger GCC modulo scheduler debug logging */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling for this function */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops", "O2")))
int modulo_sched_test(int *a, int *b, int size) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    int i, j;
    
    /* Outer loop to provide multiple scheduling contexts */
    for (j = 0; j < 10; j++) {
        /* Inner loop with carried dependency for modulo scheduling */
        /* Fixed small iteration count for manageable scheduling */
        for (i = 0; i < 32; i++) {
            /* Complex recurrence with multiple operations */
            /* sum = (sum * a[i] + b[i]) >> 1 creates true dependency */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional operations to increase instruction count */
            sum ^= (a[i] & 0xFF);      /* Bitwise operation */
            sum += (b[i] % 17);        /* Modulo operation */
            sum = (sum << 3) | (sum >> 29);  /* Rotation */
        }
        
        /* Modify input slightly for outer loop variation */
        /* Prevents complete loop invariant code motion */
        b[0] += sum & 0xF;
        a[31] ^= sum;
    }
    
    return sum;
}

/* Another test function with different pattern */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops", "O2")))
int modulo_sched_test2(int *arr1, int *arr2, int n) {
    int acc1 = 7, acc2 = 13;
    int i, k;
    
    for (k = 0; k < 8; k++) {
        /* Loop with multiple interleaved recurrences */
        for (i = 1; i < 64; i++) {
            /* Two separate carried dependencies */
            acc1 = (acc1 * 3 + arr1[i]) - (acc2 >> 2);
            acc2 = (acc2 * 5 + arr2[i]) ^ (acc1 & 0xFF);
            
            /* Cross-dependency between accumulators */
            int temp = acc1 + acc2;
            arr1[i-1] = temp & 0xFFFF;
            arr2[i] = (temp >> 16) & 0xFFFF;
        }
        
        /* Outer loop variation */
        arr1[0] += k;
        arr2[63] -= acc1;
    }
    
    return acc1 + acc2;
}

int main(void) {
    int a[128], b[128];
    int arr1[128], arr2[128];
    int i, result1, result2;
    
    /* Seed for pseudo-random but reproducible values */
    srand(42);
    
    /* Initialize arrays with volatile-like behavior */
    for (i = 0; i < 128; i++) {
        /* Use rand() to prevent constant propagation */
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        arr1[i] = rand() % 500;
        arr2[i] = rand() % 500;
    }
    
    /* Call test functions */
    result1 = modulo_sched_test(a, b, 128);
    result2 = modulo_sched_test2(arr1, arr2, 128);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    return 0;
}
