#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent interprocedural optimizations from removing the OpenMP construct */
static void compute_target(int *a, int *b, int size, int *sum) 
    __attribute__((noinline, noipa));

static void compute_target(int *a, int *b, int size, int *sum)
{
    /* Use volatile to prevent constant propagation of threshold */
    volatile int threshold = 50;
    int local_threshold = threshold;
    
    /* Complex loop with data-dependent conditionals and arithmetic operations
       to create non-trivial control flow graph suitable for SIMT transformation */
    #pragma omp target teams distribute parallel for simd \
                reduction(+:*sum) \
                map(to: a[0:size], b[0:size]) \
                map(tofrom: *sum)
    for (int i = 0; i < size; i++) {
        /* Multiple data-dependent conditionals to create basic block structure */
        int val_a = a[i];
        int val_b = b[i];
        
        /* First conditional branch - creates basic block split */
        if (val_a > local_threshold) {
            /* Nested arithmetic operations */
            int product = val_a * val_b;
            
            /* Second conditional with different operation */
            if (val_b > 25) {
                product += val_a;
            } else {
                product -= val_b;
            }
            
            /* Third conditional with bitwise operations */
            if ((val_a & 0x1) == 0) {
                /* Even index special handling */
                *sum += product >> 1;
            } else {
                *sum += product;
            }
        } else if (val_a > local_threshold / 2) {
            /* Alternative path with different computation */
            *sum += val_a + val_b;
        }
        
        /* Additional conditional to ensure complex CFG */
        if (i % 4 == 0) {
            /* Modulo operation creates additional control flow */
            *sum += 1;
        }
    }
}

int main(void)
{
    /* Seed random number generator for reproducible results */
    srand(42);
    
    /* Declare and initialize arrays with pseudo-random data */
    int a[SIZE];
    int b[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;      /* Values 0-99 */
        b[i] = rand() % 100;
    }
    
    /* Use volatile for size to prevent compile-time loop unrolling */
    volatile int size = SIZE;
    int sum = 0;
    
    /* Call the target computation function */
    compute_target(a, b, (int)size, &sum);
    
    /* Use the result to prevent dead code elimination */
    printf("Result sum = %d\n", sum);
    
    /* Additional volatile store to ensure code isn't optimized away */
    volatile int check = sum;
    (void)check;
    
    return 0;
}
