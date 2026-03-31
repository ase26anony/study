#include <stdio.h>
#include <stdlib.h>

/* Simple LCG for pseudo-random values */
static unsigned int lcg_seed = 123456789;
static inline unsigned int lcg_rand() {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Function containing the target loop */
int process_array(volatile int* arr, int size) {
    volatile int acc1 = 0;
    volatile int acc2 = 1;
    volatile int prev = 0;
    volatile int curr = arr[0];
    
    /* Outer loop to provide context */
    for (int outer = 0; outer < 3; outer++) {
        /* Reset for each outer iteration */
        acc1 = 0;
        acc2 = 1;
        prev = 0;
        curr = arr[0];
        
        /* TARGET INNER LOOP - designed for modulo scheduling */
        for (int i = 0; i < size; i++) {
            /* Loop-carried dependency: prev depends on curr from previous iteration */
            prev = curr;  /* distance1_uses: prev used in next iteration */
            
            /* Multiple arithmetic operations creating data flow */
            int val = arr[i];
            curr = val * 3 + 7;  /* curr defined here, used as prev in next iteration */
            
            /* Recurrence: accumulator with loop-carried dependency */
            acc1 = acc1 + val * acc2;  /* acc1 depends on its previous value */
            
            /* More arithmetic to increase scheduling complexity */
            acc2 = (acc2 << 1) | (val & 1);  /* acc2 also has loop-carried dependency */
            
            /* Additional operations to create move opportunities */
            int temp = prev * 2;  /* uses prev from previous iteration */
            acc1 = acc1 - temp / 4;
        }
    }
    
    /* Combine results to prevent dead code elimination */
    return acc1 + acc2 + prev + curr;
}

int main() {
    const int SIZE = 1024;
    volatile int* array = (volatile int*)malloc(SIZE * sizeof(int));
    
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        array[i] = (int)(lcg_rand() % 1000);
    }
    
    /* Execute the processing function */
    int result = process_array(array, SIZE);
    
    /* Print result to ensure observable side effect */
    printf("Result: %d\n", result);
    
    /* Additional verification */
    printf("Array[0] = %d, Array[%d] = %d\n", 
           array[0], SIZE-1, array[SIZE-1]);
    
    free((void*)array);
    return 0;
}
