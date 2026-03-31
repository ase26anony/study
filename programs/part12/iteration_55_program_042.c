#include <stdio.h>
#include <stdint.h>

#define ARRAY_SIZE 1024

/* Simple LCG for generating pseudo-random values */
static uint32_t lcg_seed = 123456789;
static uint32_t lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Function containing the target loop */
int process_array(volatile int* array, int size, int iterations) {
    volatile int sum = 0;
    volatile int prev = 0;
    volatile int curr = 0;
    volatile int temp1, temp2, temp3;
    
    /* Outer loop to provide context */
    for (int outer = 0; outer < iterations; outer++) {
        /* Reset values for each outer iteration */
        prev = 0;
        curr = array[0];
        sum = 0;
        
        /* TARGET INNER LOOP with loop-carried dependencies */
        for (int i = 1; i < size; i++) {
            /* Create distance-1 dependency: prev from iteration i-1 used in iteration i */
            temp1 = prev * 3;          /* Uses prev from previous iteration */
            
            /* Multiple arithmetic operations to create scheduling complexity */
            temp2 = array[i] * 7;
            temp3 = (temp1 << 2) + (temp2 >> 1);
            
            /* Recurrence: accumulator with loop-carried dependency */
            sum = sum + temp3;         /* sum depends on previous iteration's sum */
            
            /* Distance-1 assignment chain for distance1_uses analysis */
            prev = curr;               /* prev for next iteration */
            curr = array[i] + i;       /* curr for current iteration */
            
            /* Additional operations to increase instruction count */
            sum = sum ^ (temp1 & 0xFF);
            sum = sum + (curr % 256);
        }
    }
    
    return sum;
}

int main(void) {
    volatile int array[ARRAY_SIZE];
    
    /* Initialize array with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = (int)(lcg_rand() % 1000);
    }
    
    /* Execute the processing function multiple times */
    int total = 0;
    for (int run = 0; run < 10; run++) {
        total += process_array(array, ARRAY_SIZE, 5);
        
        /* Modify array slightly between runs to prevent complete optimization */
        for (int i = 0; i < ARRAY_SIZE; i += 64) {
            array[i] += run;
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return 0;
}
