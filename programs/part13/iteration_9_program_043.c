#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 100

/* Function A: Triple-nested loops with fully contained sub-loops */
static int function_a(int *arr, int n) {
    int sum = 0;
    
    /* Outer loop - will contain inner loops */
    for (int i = 0; i < n; i++) {
        /* First inner loop - fully contained in outer loop */
        for (int j = 0; j < i; j++) {
            /* Innermost loop - fully contained in both outer loops */
            for (int k = 0; k < j; k++) {
                sum += arr[k] * (i - j + k);
            }
            
            /* Multiple exit points */
            if (sum > LIMIT * 100) {
                break;
            }
        }
        
        /* Loop-invariant code */
        int stride = 2;
        for (int j = 0; j < n; j += stride) {
            sum += arr[i] * arr[j];
            if (sum < 0) break;  /* Another exit point */
        }
    }
    
    return sum;
}

/* Function B: Two loops that share common basic blocks via goto */
static int function_b(int *arr, int n) {
    int result = 0;
    int i = 0;
    
    /* First loop with goto to shared block */
loop1_start:
    while (i < n / 2) {
        result += arr[i] * 3;
        i++;
        
        /* Conditional goto to shared block */
        if (result % 7 == 0) {
            goto shared_block;
        }
        
        /* Continue in loop1 */
        if (i % 5 == 0) {
            result -= arr[i];
        }
        
        /* Normal loop exit check */
        if (i >= n / 2) goto loop1_end;
        continue;
        
shared_block:
        /* Shared basic block between two loops */
        result ^= 0xFF;
        arr[i] = result;
        /* Return to appropriate loop */
        if (i < n / 2) {
            goto loop1_continue;
        } else {
            goto loop2_continue;
        }
        
loop1_continue:
        continue;
    }
loop1_end:

    /* Second loop that also uses the shared block */
    while (i < n) {
        result += arr[i] * 2;
        i++;
        
        /* Different condition to goto shared block */
        if (result % 11 == 0) {
            goto shared_block;
        }
        
loop2_continue:
        /* Another exit point */
        if (result > LIMIT * 50) {
            break;
        }
    }
    
    return result;
}

/* Function C: Loop with switch statement containing break to exit loop */
static int function_c(int *arr, int n) {
    int total = 0;
    int state = 0;
    
    for (int i = 0; i < n; i++) {
        /* Complex switch inside loop */
        switch (state) {
            case 0:
                total += arr[i];
                if (total % 3 == 0) state = 1;
                break;
            case 1:
                total -= arr[i] * 2;
                if (total % 5 == 0) state = 2;
                break;
            case 2:
                total ^= arr[i];
                /* This break exits the switch, not the loop */
                break;
            case 3:
                /* This break exits the entire loop via goto */
                goto loop_exit;
            default:
                state = 0;
        }
        
        /* Nested while loop inside for loop */
        int j = 0;
        while (j < 3) {
            total += j;
            j++;
            
            /* Switch inside while inside for */
            switch (i % 4) {
                case 0:
                    if (total > LIMIT) goto while_exit;
                    break;
                case 1:
                    if (total < -LIMIT) goto while_exit;
                    break;
                case 2:
                    /* Break to label outside while */
                    goto while_exit;
                default:
                    break;
            }
        }
    while_exit:
        
        /* Multiple exit conditions */
        if (total > LIMIT * 20 || i > n * 3 / 4) {
            break;
        }
    }
loop_exit:
    
    return total;
}

/* Function D: Adjacent loops with partial overlap */
static int function_d(int *arr, int n) {
    int acc1 = 0, acc2 = 0;
    int i;
    
    /* First loop */
    for (i = 0; i < n; i++) {
        acc1 += arr[i];
        
        /* Conditional that might be shared with next loop */
        if (acc1 % 2 == 0) {
            /* Shared computation block */
            arr[i] = acc1 ^ 0xAA;
            goto next_iteration;
        }
        
        arr[i] = acc1;
        
next_iteration:
        if (i == n / 2) {
            /* Early exit creates different control flow */
            break;
        }
    }
    
    /* Second loop - partially overlaps with first in control flow */
    for (int j = i; j < n; j++) {
        acc2 += arr[j];
        
        /* Same conditional pattern as first loop */
        if (acc2 % 2 == 0) {
            /* Shared computation block - same as in first loop */
            arr[j] = acc2 ^ 0xAA;
            continue;
        }
        
        arr[j] = acc2;
        
        /* Different exit condition */
        if (acc2 > LIMIT * 10) {
            break;
        }
    }
    
    return acc1 + acc2;
}

int main() {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i + 1;
    }
    
    /* Call functions with different loop structures */
    int result = 0;
    
    /* Function A: Fully nested loops */
    volatile int a_result = function_a(arr, SIZE / 4);
    result += a_result;
    
    /* Function B: Goto-based overlapping loops */
    volatile int b_result = function_b(arr + SIZE / 4, SIZE / 4);
    result += b_result;
    
    /* Function C: Switch-based complex control flow */
    volatile int c_result = function_c(arr + SIZE / 2, SIZE / 4);
    result += c_result;
    
    /* Function D: Adjacent loops with shared blocks */
    volatile int d_result = function_d(arr + 3 * SIZE / 4, SIZE / 4);
    result += d_result;
    
    /* Use result to prevent elimination */
    printf("Final result: %d\n", result);
    
    return result % 256;
}
