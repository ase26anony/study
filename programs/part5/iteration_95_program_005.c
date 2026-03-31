#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure the loop structure remains intact */
__attribute__((noinline)) 
int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = OUTER_ITERATIONS;  /* Prevent unrolling */
    int sum = 0;
    int i;
    
    /* Outer loop with volatile control */
    while (outer_counter-- > 0) {
        /* Add some conditional variability to prevent optimization */
        if (rand() % 2) {
            /* Inner loop with multiple dependencies */
            sum = 0;
            
            /* Create distance-0 dependencies within iteration */
            int temp1, temp2, temp3;
            int prev_a = a[0];  /* Initialize for loop-carried dependency */
            
            /* Main computational loop with cross-iteration dependencies */
            for (i = 0; i < size; i++) {
                /* Multiple uses of loaded values to create distance1_uses scenarios */
                int load_a = a[i];
                int load_b = b[i];
                
                /* Operation with higher latency (multiplication) */
                int product = load_a * load_b;
                
                /* Loop-carried dependency: sum depends on previous iteration's sum */
                sum = sum + product;
                
                /* Another operation using the same product (distance-0 use) */
                temp1 = product & 0xFF;
                
                /* Loop-carried dependency on array 'a' (distance-1) */
                int new_a;
                if (i > 0) {
                    /* a[i] depends on a[i-1] - creates distance-1 dependency */
                    new_a = prev_a + load_b;
                } else {
                    new_a = load_a;
                }
                
                /* Multiple operations with different latencies */
                temp2 = new_a ^ product;      /* Bitwise operation */
                temp3 = temp1 | temp2;        /* Another bitwise operation */
                
                /* Store with loop-carried dependency */
                a[i] = new_a + temp3;
                
                /* Update for next iteration's loop-carried dependency */
                prev_a = new_a;
                
                /* Additional arithmetic to create more scheduling pressure */
                b[i] = (b[i] * 3) + (sum & 0x7F);
            }
            
            /* Use the computed values to prevent dead code elimination */
            sum += temp1 + temp2 + temp3;
        } else {
            /* Alternative path to create control flow variability */
            for (i = 0; i < 10; i++) {
                sum += a[i] - b[i];
            }
        }
    }
    
    return sum;
}

int main() {
    int i;
    int result;
    
    /* Initialize with random data */
    srand(time(NULL));
    
    int *a = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    
    if (!a || !b) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill arrays with pseudo-random data */
    for (i = 0; i < SIZE; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Call the function with the loop structure */
    result = modulo_scheduled_loop(a, b, SIZE);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Verify some computation */
    printf("First few elements of a after computation:\n");
    for (i = 0; i < 5; i++) {
        printf("a[%d] = %d, b[%d] = %d\n", i, a[i], i, b[i]);
    }
    
    free(a);
    free(b);
    
    return 0;
}
