#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Simple LCG for pseudo-random values */
static unsigned int lcg_seed = 123456789;
static unsigned int lcg_rand() {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return (unsigned int)(lcg_seed / 65536) % 32768;
}

/* Function to create loop-carried dependencies */
int process_loop(volatile int* array, int n) {
    volatile int acc1 = 0;
    volatile int acc2 = 1;
    volatile int prev = 0;
    volatile int curr = array[0];
    
    /* 
     * Target inner loop with:
     * 1. Loop-carried dependency: acc1 depends on its previous value
     * 2. Distance-1 dependency: prev = curr; curr = array[i] creates distance-1 use
     * 3. Multiple arithmetic operations
     * 4. Array access with volatile pointer
     */
    for (int i = 0; i < n; i++) {
        /* Distance-1 dependency chain */
        prev = curr;                    /* Used in next iteration */
        curr = array[i];                /* Defines curr for this iteration */
        
        /* Loop-carried recurrence on acc1 */
        acc1 = acc1 + array[i] * acc1;  /* True recurrence: uses acc1 from prev iteration */
        
        /* Additional arithmetic to create scheduling complexity */
        acc2 = acc2 ^ (array[i] << 2);  /* Independent operation */
        
        /* More operations to increase instruction count */
        int temp = prev * 3;            /* Uses prev from previous iteration (distance-1) */
        acc1 = acc1 - temp / 2;
        
        /* Another operation with shift */
        acc2 = acc2 | (array[i] >> 1);
    }
    
    /* Combine results to prevent dead code elimination */
    return acc1 + acc2;
}

/* Outer loop to provide context */
void outer_loop(volatile int* array, int outer_iterations) {
    volatile int total = 0;
    
    for (int j = 0; j < outer_iterations; j++) {
        /* Modify array slightly each outer iteration */
        for (int k = 0; k < SIZE; k++) {
            array[k] = (array[k] + j) & 0xFF;
        }
        
        /* Call the target inner loop */
        total += process_loop(array, SIZE);
    }
    
    /* Print result to ensure side effects */
    printf("Result: %d\n", total);
}

int main() {
    /* Create and initialize array with volatile data */
    volatile int* array = (volatile int*)malloc(SIZE * sizeof(int));
    
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        array[i] = lcg_rand() % 100;
    }
    
    /* Execute the nested loop structure */
    outer_loop(array, 10);
    
    free((void*)array);
    return 0;
}
