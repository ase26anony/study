/* Program to trigger modulo scheduling debug logging in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent vectorization and unrolling for this function */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int modulo_sched_test(int *a, int *b, int n_outer) {
    int sum = 1;  /* Non-zero initial value for recurrence */
    int i, j;
    
    /* Outer loop to provide multiple scheduling contexts */
    for (j = 0; j < n_outer; j++) {
        /* Inner loop with carried dependency for modulo scheduling */
        /* Fixed small iteration count for manageable scheduling */
        for (i = 0; i < 32; i++) {
            /* Complex recurrence with multiple operations:
               - Multiplication creates multi-cycle latency
               - Addition provides another operation
               - Shift breaks dependency chains slightly
               This creates scheduling challenges */
            sum = (sum * a[i] + b[i]) >> 1;
            
            /* Additional operations to increase instruction count */
            sum ^= (a[i] & 0xFF);  /* Bitwise operation */
            sum += (i & 1);        /* Simple dependency on loop index */
        }
        
        /* Modify input slightly to prevent complete optimization */
        b[0] += sum;
        a[31] ^= j;
    }
    
    return sum;
}

/* Another test function with different pattern */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int modulo_sched_test2(int *arr, int n) {
    int acc1 = 1, acc2 = 2;
    int i;
    
    /* Loop with two independent but interleaved recurrences */
    for (i = 1; i < 64; i++) {
        /* First recurrence chain */
        acc1 = (acc1 * 3 + arr[i]) % 1000;
        
        /* Second recurrence chain with cross-dependency */
        acc2 = (acc2 + acc1 * arr[i-1]) >> 2;
        
        /* Additional operation mixing both accumulators */
        arr[i] = (acc1 + acc2) & 0xFF;
    }
    
    return acc1 + acc2;
}

/* Simple test with pure recurrence */
__attribute__((optimize("no-tree-vectorize", "no-unroll-loops")))
int simple_recurrence(int *vals, int count) {
    int result = vals[0];
    int i;
    
    for (i = 1; i < count; i++) {
        /* Simple but strong dependency chain */
        result = result * 7 + vals[i];
    }
    
    return result;
}

int main(void) {
    int a[128], b[128];
    int i, result1, result2, result3;
    
    /* Seed RNG for unpredictable values */
    srand(time(NULL));
    
    /* Initialize arrays with pseudo-random values
       to prevent constant propagation */
    for (i = 0; i < 128; i++) {
        a[i] = rand() % 256;
        b[i] = rand() % 256;
    }
    
    /* Use volatile to prevent optimization */
    volatile int outer_iterations = 10;
    
    printf("Starting modulo scheduling tests...\n");
    
    /* Test 1: Main test with outer and inner loops */
    result1 = modulo_sched_test(a, b, outer_iterations);
    printf("Test 1 result: %d\n", result1);
    
    /* Test 2: Different pattern */
    result2 = modulo_sched_test2(a, 64);
    printf("Test 2 result: %d\n", result2);
    
    /* Test 3: Simple recurrence */
    result3 = simple_recurrence(b, 32);
    printf("Test 3 result: %d\n", result3);
    
    /* Final result prevents dead code elimination */
    return (result1 + result2 + result3) & 0xFF;
}
