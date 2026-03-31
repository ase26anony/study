/* modulo-sched-trigger.c
 * Designed to trigger specific uncovered lines in GCC's modulo scheduler
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms -fdump-rtl-all -fdump-rtl-sched2 -march=native -o modulo_test modulo-sched-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 1024
#define INNER_ITERATIONS 1000
#define OUTER_ITERATIONS 5

/* Use volatile to prevent excessive optimization */
volatile int outer_counter = OUTER_ITERATIONS;

/* Mark as noinline to preserve loop structure */
__attribute__((noinline))
int modulo_scheduled_loop(int *a, int *b, int size) {
    volatile int control_var = 0;
    int sum = 0;
    int i, j;
    
    /* Outer loop with volatile control */
    for (j = 0; j < outer_counter; j++) {
        /* Create control flow variability with rand() */
        if (rand() % 2) {
            control_var = 1;
        } else {
            control_var = 0;
        }
        
        /* Critical inner loop with cross-iteration dependencies */
        /* This should trigger modulo scheduling */
        sum = 0;
        
        /* Initialize first element with dependency chain */
        int prev = a[0];
        
        for (i = 0; i < size; i++) {
            /* Multiple operations with different latencies */
            int load_a = a[i];           /* Load operation */
            int load_b = b[i];           /* Load operation */
            
            /* Integer multiply (higher latency on x86) */
            int product = load_a * load_b;
            
            /* Loop-carried dependency: sum depends on previous iteration */
            sum = sum + product;         /* distance-1 dependency */
            
            /* Another distance-1 dependency with different operation */
            int temp_sum = sum;
            
            /* Use bitwise operations (different latency profile) */
            int masked = temp_sum & 0xFF;
            
            /* Chain of operations creating complex dependency graph */
            int shifted = masked << 2;
            int xored = shifted ^ product;
            
            /* Array update with distance-1 dependency */
            /* a[i] depends on a[i-1] from previous iteration */
            if (i > 0) {
                /* True loop-carried dependency */
                int prev_val = a[i-1];   /* distance-1 use */
                int delta = prev_val * 3; /* Another multiply */
                a[i] = (a[i] + delta + xored) & 0x7FFFFFFF;
                
                /* Multiple uses of the same value to create distance1_uses */
                int use1 = prev_val + 1;  /* First use */
                int use2 = prev_val * 2;  /* Second use - different operation */
                b[i] = (use1 + use2) & 0xFF;
                
                /* Third use with bitwise operation */
                int use3 = prev_val & 0xF;
                sum += use3;
            } else {
                /* Boundary case - still create work */
                a[i] = (a[i] + xored) & 0x7FFFFFFF;
            }
            
            /* Additional arithmetic to increase register pressure */
            int extra1 = product + (i & 0xF);
            int extra2 = extra1 * 3;
            int extra3 = extra2 - (sum & 0xF);
            
            /* Use volatile to prevent elimination */
            volatile int sink = extra3;
            (void)sink;
            
            /* Update previous value for next iteration */
            prev = a[i];
        }
        
        /* Mix in control_var to prevent dead code elimination */
        sum = sum * (control_var + 1);
    }
    
    return sum;
}

int main() {
    int i;
    int result;
    
    /* Initialize arrays with pseudo-random data */
    int *a = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *b = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    if (!a || !b) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    srand(time(NULL));
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
    }
    
    /* Call the loop function multiple times to ensure execution */
    result = 0;
    for (i = 0; i < 3; i++) {
        result += modulo_scheduled_loop(a, b, INNER_ITERATIONS);
        
        /* Shuffle data slightly between calls */
        a[rand() % ARRAY_SIZE] = rand() % 1000;
        b[rand() % ARRAY_SIZE] = rand() % 1000;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional print to ensure execution */
    printf("Array sample: a[0]=%d, b[0]=%d\n", a[0], b[0]);
    
    free(a);
    free(b);
    
    return 0;
}
