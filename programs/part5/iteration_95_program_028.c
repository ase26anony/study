#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITER 4

/* Prevent inlining to ensure loop structure is preserved */
__attribute__((noinline)) 
int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent unrolling */
    int total_sum = 0;
    
    /* Outer loop with volatile control */
    while (outer_counter < OUTER_ITER) {
        int sum = 0;
        int prev_val = a[0];  /* For loop-carried dependency */
        
        /* Critical inner loop with cross-iteration dependencies */
        for (int i = 0; i < size; i++) {
            /* Multiple operations with different latencies */
            int load_a = a[i];
            int load_b = b[i];
            
            /* High-latency operation (multiplication) */
            int product = load_a * load_b;
            
            /* Loop-carried dependency: sum depends on previous iteration */
            sum = sum + product;
            
            /* Another loop-carried dependency with distance 1 */
            int temp = prev_val + load_b;
            
            /* Multiple uses of the same value (creates distance1_uses) */
            int masked1 = temp & 0xFF;
            int masked2 = temp & 0x0F;
            int combined = masked1 | masked2;
            
            /* Update array with loop-carried dependency */
            if (i > 0) {
                a[i] = a[i-1] + combined;  /* True distance-1 dependency */
            }
            
            prev_val = temp;
            
            /* Additional operations to create complex scheduling graph */
            int diff = load_a - load_b;
            int xor_result = diff ^ product;
            int shifted = xor_result << 2;
            
            /* Use volatile to prevent optimization */
            volatile int dummy = shifted;
        }
        
        total_sum += sum;
        outer_counter++;
        
        /* Conditional branch based on random to create control variability */
        if (rand() % 2) {
            /* Extra operations to affect scheduling */
            volatile int extra = total_sum & 1;
        }
    }
    
    return total_sum;
}

int main() {
    /* Initialize with random data */
    srand(time(NULL));
    
    int a[SIZE];
    int b[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    /* Call the loop function */
    int result = modulo_scheduled_loop(a, b, SIZE);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
