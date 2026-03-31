#include <stdio.h>
#include <stdint.h>

/* Simple LCG for pseudo-random values */
static inline uint32_t lcg(uint32_t* state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

/* Function containing the target loop */
int process_loop(volatile int* array, int size, volatile int init) {
    volatile int acc1 = init;      /* Primary accumulator with recurrence */
    volatile int acc2 = 0;         /* Secondary accumulator */
    volatile int prev = 0;         /* For distance-1 dependency */
    volatile int curr = array[0];  /* Current value */
    
    /* Loop with multiple operations and dependencies */
    for (int i = 0; i < size; i++) {
        /* Distance-1 dependency: prev from iteration i-1 used here */
        int temp = prev * 3;       /* Use prev from previous iteration */
        
        /* Recurrence: acc1 depends on its own previous value */
        acc1 = acc1 + array[i] * 7;
        
        /* Multiple arithmetic operations to create scheduling complexity */
        acc2 = acc2 ^ (array[i] << 2);
        acc2 = acc2 + (temp >> 1);
        
        /* Update distance-1 variables for next iteration */
        prev = curr;              /* curr becomes prev for next iteration */
        curr = array[i] + acc1;   /* curr depends on acc1 (recurrence) */
        
        /* More operations to increase instruction count */
        acc1 = acc1 ^ (prev & 0xFF);
        acc2 = acc2 * 3 + 1;
    }
    
    /* Combine results to prevent elimination */
    return acc1 + acc2 + prev + curr;
}

/* Outer loop to provide context */
void outer_loop(volatile int* data, int outer_iters, int inner_size) {
    volatile int total = 0;
    uint32_t seed = 42;
    
    for (int iter = 0; iter < outer_iters; iter++) {
        /* Initialize array with pseudo-random values */
        for (int i = 0; i < inner_size; i++) {
            data[i] = lcg(&seed) % 100;
        }
        
        /* Execute target inner loop */
        int result = process_loop(data, inner_size, iter);
        total += result;
        
        /* Prevent loop invariant code motion */
        asm volatile("" : "+r" (total) : : "memory");
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %d\n", total);
}

int main() {
    const int ARRAY_SIZE = 1024;
    const int OUTER_ITERS = 100;
    
    /* Volatile array to prevent optimization */
    volatile int data[ARRAY_SIZE];
    
    /* Execute the nested loop structure */
    outer_loop(data, OUTER_ITERS, ARRAY_SIZE);
    
    return 0;
}
