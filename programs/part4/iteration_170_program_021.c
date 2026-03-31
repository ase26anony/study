/* ifcvt_coverage.c
 * Designed to trigger GCC's if-conversion pass and cover the validation
 * logic that checks if instructions in the "then" block modify the condition.
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent constant propagation */
volatile int global_cond = 1;

/* Function marked noinline to prevent early optimization */
__attribute__((noinline)) 
int process_data(int iterations) {
    volatile int cond = global_cond;  /* Condition variable - volatile read */
    int a = 0;                        /* Variable modified in then block */
    int b = 42;                       /* Another variable for operations */
    int result = 0;
    
    /* Volatile loop limit to prevent unrolling */
    volatile int N = iterations;
    
    for (int i = 0; i < N; i++) {
        /* Condition expression using cond (test_expr) */
        if (cond > 0) {
            /* THEN BLOCK: Operations that do NOT modify cond */
            /* These should pass the validation in ifcvt.cc lines 577-583 */
            a = b + 1;      /* First non-label, non-debug instruction */
            b = a * 2;      /* Second instruction */
            a = a ^ b;      /* Third instruction - bitwise operation */
            b = b & 0xFF;   /* Fourth instruction */
            
            /* More safe operations to ensure block has multiple instructions */
            result += a;
            result -= b;
        } else {
            /* ELSE BLOCK: Also doesn't modify cond */
            a = b - 1;
            b = a / 2;
            result += b;
        }
        
        /* Modify cond for next iteration - but NOT inside the then block */
        /* This ensures the condition changes but the validation still passes */
        cond = (cond * 1103515245 + 12345) & 0x7fffffff;
        
        /* Prevent loop optimization */
        asm volatile("" : : "r"(cond) : "memory");
    }
    
    return result + a + b;
}

/* Another test case with different condition pattern */
__attribute__((noinline))
int test_complex_condition(int seed) {
    volatile int cond1 = seed;
    volatile int cond2 = seed * 2;
    int x = 0, y = 100, z = 200;
    
    for (volatile int i = 0; i < 50; i++) {
        /* Compound condition */
        if (cond1 > 0 && cond2 < 1000) {
            /* Multiple arithmetic operations that don't touch cond1/cond2 */
            x = y + z;
            y = x - z;
            z = y * 2;
            x = x ^ y;
            z = z | 0x0F;
        } else {
            x = y - z;
            y = z * 2;
        }
        
        /* Update conditions outside the blocks */
        cond1 = (cond1 + 3571) % 1000;
        cond2 = (cond2 * 3) % 2000;
    }
    
    return x + y + z;
}

/* Test with pointer operations (still safe) */
__attribute__((noinline))
int test_with_pointers(int init) {
    volatile int cond = init;
    int data[4] = {1, 2, 3, 4};
    int temp = 0;
    
    for (volatile int i = 0; i < 30; i++) {
        if (cond != 0) {
            /* Pointer arithmetic that doesn't modify cond */
            data[0] = data[1] + data[2];
            data[1] = data[0] * data[3];
            data[2] = data[1] ^ data[0];
            temp = data[0] + data[1] + data[2] + data[3];
        } else {
            data[0] = data[3] - data[1];
            temp = data[0] * 2;
        }
        
        cond = (cond * 1664525 + 1013904223) & 0x7fffffff;
    }
    
    return temp;
}

int main() {
    int result1, result2, result3;
    
    /* Initialize with non-deterministic value */
    global_cond = rand() % 1000 + 1;
    
    /* Call test functions */
    result1 = process_data(100);
    result2 = test_complex_condition(global_cond);
    result3 = test_with_pointers(global_cond * 2);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d, %d\n", result1, result2, result3);
    
    return (result1 + result2 + result3) != 0 ? 0 : 1;
}
