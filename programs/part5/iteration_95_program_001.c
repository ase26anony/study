#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define OUTER_ITERATIONS 4

/* Prevent inlining to ensure the loop structure is preserved */
__attribute__((noinline)) 
int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int outer_counter = 0;  /* Prevent outer loop unrolling */
    int total_sum = 0;
    
    /* Outer loop with volatile control */
    while (outer_counter < OUTER_ITERATIONS) {
        int sum = 0;
        int prev_a = a[0];  /* For loop-carried dependency */
        
        /* Inner loop with multiple dependencies */
        for (int i = 0; i < size; i++) {
            /* Create distance-0 dependencies within iteration */
            int load_a = a[i];
            int load_b = b[i];
            
            /* Integer multiply with higher latency */
            int product = load_a * load_b;
            
            /* Loop-carried dependency (distance-1) */
            sum = sum + product;
            
            /* Multiple uses of the same value to create distance1_uses */
            int temp1 = product & 0xFF;      /* Bitwise operation */
            int temp2 = product | 0x100;     /* Another bitwise operation */
            int combined = temp1 + temp2;    /* Use both */
            
            /* Another loop-carried dependency with distance-1 */
            /* Update a[i] based on previous iteration's value */
            if (i > 0) {
                a[i] = prev_a + load_b + combined;
                prev_a = a[i];  /* Update for next iteration */
            } else {
                a[i] = load_a + load_b;
                prev_a = a[i];
            }
            
            /* More operations to create complex scheduling graph */
            int diff = load_a - load_b;
            int shifted = diff << 2;
            int masked = shifted & 0x3FF;
            
            /* Use masked value in multiple ways */
            sum = sum + (masked >> 1);
            if (i % 2 == 0) {
                sum = sum - (masked & 0xF);
            }
        }
        
        total_sum += sum;
        outer_counter++;
        
        /* Add some control flow variability */
        if (rand() % 2 == 0) {
            total_sum = total_sum ^ 0xABCD;  /* Bitwise XOR */
        }
    }
    
    return total_sum;
}

int main() {
    /* Initialize with random data */
    int a[SIZE];
    int b[SIZE];
    
    srand(time(NULL));
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Call the loop function */
    int result = modulo_scheduled_loop(a, b, SIZE);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Also print some array values to ensure they're used */
    printf("Sample a[0]: %d, a[SIZE-1]: %d\n", a[0], a[SIZE-1]);
    
    return 0;
}
