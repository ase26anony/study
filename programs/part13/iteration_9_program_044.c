#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 1000

/* Function A: Triple-nested fully contained loops */
static int triple_nested_loop(int *arr, int n) {
    int sum = 0;
    int stride = 4; /* Loop-invariant variable */
    
    /* Outer loop - will contain inner loops */
    for (int i = 0; i < n; i++) {
        /* First inner loop - fully contained */
        for (int j = 0; j < 5; j++) {
            /* Innermost loop - fully contained */
            for (int k = 0; k < 3; k++) {
                sum += arr[i * stride + j + k];
            }
            /* Multiple exit point */
            if (sum > LIMIT) break;
        }
        /* Another exit point */
        if (i > n/2 && sum < 0) break;
    }
    return sum;
}

/* Function B: Overlapping loops with goto */
static int overlapping_loops(int *arr, int n) {
    int result = 0;
    int i = 0;
    
    /* First loop */
    for (i = 0; i < n/2; i++) {
        result += arr[i] * 2;
        if (result > LIMIT) {
            goto common_block; /* Jump to shared block */
        }
    }
    
    /* Second loop - partially overlaps via goto */
    for (i = n/2; i < n; i++) {
        result -= arr[i];
        if (result < 0) {
            goto common_block; /* Jump to same shared block */
        }
    }
    
    return result;
    
common_block: /* Shared basic block between the two loops */
    result = (result * 3) / 2;
    return result;
}

/* Function C: Loop with switch and break to outer label */
static int switch_in_loop(int *arr, int n) {
    int total = 0;
    int accumulator = 0;
    
    for (int i = 0; i < n; i++) {
        accumulator += arr[i];
        
        /* Switch with break to exit loop */
        switch (accumulator % 4) {
            case 0:
                total += arr[i];
                break;
            case 1:
                total -= arr[i];
                /* This break only exits the switch */
                break;
            case 2:
                /* Complex control flow: break to label outside loop */
                if (accumulator > LIMIT/2) {
                    goto loop_exit;
                }
                total *= 2;
                break;
            case 3:
                total /= 2;
                break;
        }
        
        /* Another conditional break */
        if (total > LIMIT || total < -LIMIT) {
            break;
        }
    }
    
    return total;

loop_exit: /* Label for switch break to target */
    return total * 3;
}

/* Function D: Adjacent loops with shared computation */
static int adjacent_loops(int *arr, int n) {
    int sum1 = 0, sum2 = 0;
    
    /* First adjacent loop */
    for (int i = 0; i < n/3; i++) {
        sum1 += arr[i] * arr[i];
        /* Loop-invariant code */
        int invariant = n * 2;
        sum1 += invariant % 7;
    }
    
    /* Shared computation block */
    int shared = sum1 % 256;
    
    /* Second adjacent loop - not nested, but shares some control flow */
    for (int i = n/3; i < 2*n/3; i++) {
        sum2 += arr[i] - shared;
        if (sum2 > LIMIT) {
            sum2 = shared; /* Use shared value */
            break;
        }
    }
    
    return sum1 + sum2;
}

int main(void) {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = (i * 3 + 1) % 100;
    }
    
    /* Call functions with different slices to create various loop structures */
    int result = 0;
    
    /* Function A: Fully nested loops */
    result += triple_nested_loop(arr, SIZE/4);
    
    /* Function B: Overlapping loops with goto */
    result += overlapping_loops(arr + SIZE/4, SIZE/4);
    
    /* Function C: Loop with switch break */
    result += switch_in_loop(arr + SIZE/2, SIZE/4);
    
    /* Function D: Adjacent loops */
    result += adjacent_loops(arr + 3*SIZE/4, SIZE/4);
    
    /* Use volatile to prevent dead code elimination */
    volatile int sink = result;
    
    /* Print to ensure code isn't eliminated */
    printf("Result: %d\n", result);
    
    return result % 256;
}
