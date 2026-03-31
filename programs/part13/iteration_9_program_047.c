#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define INNER_SIZE 16
#define MID_SIZE 32

static volatile int sink = 0;

/* Function A: Triple-nested fully contained loops */
static int triple_nested_loop(int *arr, int n) {
    int sum = 0;
    
    /* Outer loop - fully contains inner loops */
    for (int i = 0; i < n; i += MID_SIZE) {
        /* Middle loop - fully contained in outer */
        for (int j = 0; j < MID_SIZE && (i + j) < n; j++) {
            /* Inner loop - fully contained in middle */
            for (int k = 0; k < INNER_SIZE; k++) {
                sum += arr[i + j] * k;
                if (sum > 1000000) {
                    /* Multiple exit point */
                    break;
                }
            }
            /* Loop-invariant code */
            int stride = 4;  /* Loop invariant */
            sum += arr[(i + j) * stride % n];
        }
    }
    return sum;
}

/* Function B: Overlapping loops with shared basic blocks via goto */
static int overlapping_loops(int *arr, int n) {
    int result = 0;
    int i = 0;
    
    /* First loop */
loop1_start:
    for (; i < n / 2; i++) {
        result += arr[i] * 2;
        if (result % 7 == 0) {
            /* Jump to shared block */
            goto shared_block;
        }
        if (i > n / 4) {
            /* Early exit */
            break;
        }
    }
    
    /* Second loop - partially overlaps with first */
    i = n / 4;  /* Start in the middle of first loop's range */
loop2_start:
    while (i < 3 * n / 4) {
        result -= arr[i];
        i++;
        if (result < -1000) {
            goto shared_block;
        }
    }
    goto done;
    
shared_block:
    /* Shared basic block between both loops */
    result = (result * 3) / 2;
    if (i < n / 2) {
        /* Return to first loop */
        i++;
        goto loop1_start;
    } else {
        /* Continue to second loop */
        goto loop2_start;
    }
    
done:
    return result;
}

/* Function C: Loop with switch statement containing break to exit loop */
static int switch_in_loop(int *arr, int n) {
    int total = 0;
    int state = 0;
    
    for (int idx = 0; idx < n; idx++) {
        total += arr[idx];
        
        /* Complex switch inside loop */
        switch (state) {
            case 0:
                if (total > 5000) state = 1;
                break;
            case 1:
                if (arr[idx] % 2 == 0) {
                    /* This break exits the switch, not the loop */
                    break;
                }
                /* Fall through */
            case 2:
                if (total < -1000) {
                    /* Use goto to simulate break to outer label */
                    goto exit_loop;
                }
                total *= 2;
                break;
            default:
                /* Direct break from loop inside switch */
                if (idx > n / 2) {
                    goto exit_loop;
                }
        }
        
        /* Additional control flow */
        if (idx % 10 == 0) {
            continue;
        }
        
        /* Another basic block */
        total -= idx;
    }
    
exit_loop:
    return total;
}

/* Function D: Adjacent loops with potential for hierarchy confusion */
static int adjacent_loops(int *arr, int n) {
    int sum1 = 0, sum2 = 0;
    
    /* First adjacent loop */
    int i = 0;
    while (i < n / 3) {
        sum1 += arr[i] * i;
        i++;
        if (sum1 > 10000) {
            break;
        }
    }
    
    /* Second adjacent loop - shares no blocks with first */
    for (int j = n / 2; j < 2 * n / 3; j++) {
        sum2 += arr[j] - j;
        /* Multiple exit points */
        if (sum2 < -5000) break;
        if (j % 8 == 0) continue;
        
        /* Nested mini-loop */
        for (int k = 0; k < 3; k++) {
            sum2 += k;
        }
    }
    
    return sum1 + sum2;
}

int main() {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i % 100;  /* Prevent overflow in calculations */
    }
    
    /* Call all functions to create various loop structures */
    int result = 0;
    
    /* Function A: Fully nested loops */
    result += triple_nested_loop(arr, SIZE);
    
    /* Function B: Overlapping loops with goto */
    result += overlapping_loops(arr + SIZE/4, SIZE/2);
    
    /* Function C: Loop with complex switch */
    result += switch_in_loop(arr + SIZE/3, SIZE/3);
    
    /* Function D: Adjacent loops */
    result += adjacent_loops(arr, SIZE);
    
    /* Use result to prevent elimination */
    sink = result;
    printf("Result: %d\n", result);
    
    return result % 256;
}
