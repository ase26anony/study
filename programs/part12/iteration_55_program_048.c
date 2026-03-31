#include <stdio.h>
#include <stdlib.h>

/* Simple pseudo-random generator to create varying data */
static unsigned int seed = 12345;
static unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
}

/* Function containing the target loop */
int process_loop(volatile int *arr, int n, volatile int init) {
    volatile int acc1 = init;      /* First accumulator with recurrence */
    volatile int acc2 = 0;         /* Second accumulator */
    volatile int prev = 0;         /* Previous value for distance-1 dependency */
    volatile int curr = arr[0];    /* Current value */
    
    /* Tight inner loop with loop-carried dependencies */
    for (int i = 0; i < n; i++) {
        /* Distance-1 dependency: prev from iteration i-1 used here */
        int temp = prev * 3;       /* Use prev from previous iteration */
        
        /* Recurrence: acc1 depends on its own previous value */
        acc1 = acc1 + arr[i] * 2;  /* True loop-carried dependency */
        
        /* Multiple arithmetic operations to create scheduling complexity */
        acc2 = acc2 ^ (arr[i] + i);  /* XOR operation */
        curr = arr[i] << 2;          /* Shift operation */
        
        /* Distance-1 assignment chain */
        prev = curr + temp;          /* prev will be used in next iteration */
        
        /* More arithmetic to increase instruction count */
        acc1 = acc1 - (i & 0x1F);    /* Modify accumulator */
        acc2 = acc2 | (prev >> 1);   /* OR with shifted prev */
    }
    
    /* Combine results to prevent dead code elimination */
    return acc1 + acc2 + prev;
}

/* Outer loop to provide context */
void outer_loop(volatile int *arr, int outer_iter, int inner_size) {
    volatile int total = 0;
    
    for (int j = 0; j < outer_iter; j++) {
        /* Vary the initial value slightly */
        volatile int init = j * 7;
        
        /* Call the inner loop function */
        int result = process_loop(arr + j * 16, inner_size, init);
        
        /* Accumulate results */
        total += result;
        
        /* Modify array elements for next iteration */
        for (int k = 0; k < 16; k++) {
            arr[j * 16 + k] = lcg_rand() % 100;
        }
    }
    
    /* Print to ensure side effects are observable */
    printf("Total result: %d\n", total);
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int OUTER_ITER = 8;
    const int INNER_SIZE = 128;  /* Compile-time known inner loop bound */
    
    /* Allocate and initialize array with volatile data */
    volatile int *array = (volatile int*)malloc(ARRAY_SIZE * sizeof(int));
    if (!array) return 1;
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = lcg_rand() % 100;
    }
    
    /* Execute the nested loop structure */
    outer_loop(array, OUTER_ITER, INNER_SIZE);
    
    /* Additional loop with different pattern to increase coverage chance */
    volatile int sum = 0;
    volatile int chain = 1;
    
    /* Another loop with different recurrence pattern */
    for (int i = 0; i < INNER_SIZE; i++) {
        /* Pointer-chase-like recurrence */
        chain = chain * array[i] + 1;
        
        /* Distance-1 dependency through multiple variables */
        volatile int tmp = chain;
        chain = tmp + (chain >> 3);
        
        /* Multiple uses of chain with different latencies */
        sum += chain * 2 - tmp;
        
        /* Array access with stride */
        int idx = (i * 3) % ARRAY_SIZE;
        sum ^= array[idx];
    }
    
    printf("Final sum: %d\n", sum);
    
    free((void*)array);
    return 0;
}
