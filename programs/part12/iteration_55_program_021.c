#include <stdio.h>
#include <stdlib.h>

/* Simple LCG for pseudo-random values */
static unsigned int lcg_seed = 123456789;
static unsigned int lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return (lcg_seed >> 16) & 0x7FFF;
}

/* Function containing the target loop */
int process_array(volatile int *arr, int n) {
    volatile int sum = 0;
    volatile int prev = 0;
    volatile int curr = 1;
    volatile int temp;
    
    /* Loop with multiple dependencies and operations */
    for (int i = 0; i < n; i++) {
        /* Array access with volatile to prevent optimization */
        temp = arr[i];
        
        /* Recurrence 1: sum depends on previous iteration's sum */
        sum = sum + temp * 3;
        
        /* Recurrence 2: curr depends on prev from previous iteration */
        /* This creates distance-1 dependency */
        prev = curr;
        curr = (prev + temp) >> 1;
        
        /* Additional arithmetic to increase scheduling complexity */
        sum = sum ^ (curr * 7);
        sum = sum + (i & 0xF);
        
        /* Another distance-1 dependency chain */
        temp = curr;
        curr = (temp * 2) - (prev & 0x1F);
    }
    
    return sum + curr + prev;
}

/* Outer loop to provide context */
void outer_loop(volatile int *arr, int size, int outer_iter) {
    volatile int total = 0;
    
    for (int j = 0; j < outer_iter; j++) {
        /* Slight variation in array access pattern */
        int offset = j % 16;
        total += process_array(arr + offset, size - offset);
        
        /* Modify array slightly between iterations */
        if (j % 3 == 0) {
            for (int k = 0; k < 16; k++) {
                arr[(j + k) % size] = lcg_rand() % 100;
            }
        }
    }
    
    printf("Final total: %d\n", total);
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int OUTER_ITER = 10;
    
    /* Allocate and initialize array with volatile values */
    volatile int *array = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = lcg_rand() % 100;
    }
    
    /* Execute the loop structure */
    outer_loop(array, ARRAY_SIZE, OUTER_ITER);
    
    /* Cleanup */
    free((void*)array);
    
    return 0;
}
