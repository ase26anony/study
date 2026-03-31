#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Simple LCG for pseudo-random values */
static unsigned int seed = 123456789;
unsigned int lcg_rand() {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Function to create loop-carried dependencies */
int process_loop(int *volatile arr, int n, int init) {
    volatile int acc1 = init;      /* Main accumulator with recurrence */
    volatile int acc2 = 0;         /* Secondary accumulator */
    volatile int prev = 0;         /* For distance-1 dependency */
    volatile int curr = 1;         /* For distance-1 dependency */
    int temp1, temp2, temp3;
    
    /* Tight inner loop with multiple operations and dependencies */
    for (int i = 0; i < n; i++) {
        /* Distance-1 dependency chain: prev used in next iteration */
        prev = curr;                    /* Move current to previous */
        curr = arr[i] & 0xFF;           /* New current value */
        
        /* Recurrence: acc1 depends on its previous value (loop-carried) */
        temp1 = acc1 * 3;               /* Multiply accumulator */
        temp2 = temp1 + prev;           /* Use prev from current iteration */
        acc1 = temp2 >> 1;              /* Update accumulator for next iteration */
        
        /* Additional arithmetic operations for scheduling complexity */
        temp3 = arr[i] * acc1;          /* Array access with accumulator */
        acc2 += temp3 & 0x3F;           /* Update secondary accumulator */
        
        /* More operations to increase instruction count */
        arr[i] = (arr[i] ^ acc2) + prev; /* Modify array with dependencies */
    }
    
    /* Mix results to prevent dead code elimination */
    return acc1 + acc2 + prev + curr;
}

/* Outer loop to provide context */
void outer_loop(int *volatile arr, int outer_iter) {
    volatile int result = 0;
    
    for (int j = 0; j < outer_iter; j++) {
        /* Initialize array with pseudo-random values */
        for (int i = 0; i < SIZE; i++) {
            arr[i] = lcg_rand() % 1000;
        }
        
        /* Execute the target inner loop multiple times */
        result += process_loop(arr, SIZE, j * 7);
        
        /* Small variation in loop bound */
        int reduced_size = SIZE - (j % 8);
        result += process_loop(arr, reduced_size, result & 0xFF);
    }
    
    /* Ensure result is used */
    printf("Final result: %d\n", result);
}

int main() {
    /* Volatile pointer to prevent optimization */
    int *volatile data = (int*)malloc(SIZE * sizeof(int));
    if (!data) return 1;
    
    /* Execute with multiple outer iterations */
    outer_loop(data, 5);
    
    /* Additional test with different parameters */
    printf("Second test:\n");
    outer_loop(data, 3);
    
    free(data);
    return 0;
}
