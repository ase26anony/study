#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 1000

/* Function A: Triple-nested fully contained loops */
static int function_a(int *arr, int n) {
    int sum = 0;
    
    /* Outer loop - fully contains inner loops */
    for (int i = 0; i < n; i++) {
        /* Middle loop - fully contained in outer */
        for (int j = 0; j < i; j++) {
            /* Innermost loop - fully contained in middle */
            for (int k = 0; k < j; k++) {
                sum += arr[i * 16 + j * 8 + k];
            }
            
            /* Multiple exit points */
            if (sum > LIMIT) {
                break;
            }
        }
        
        /* Loop-invariant code */
        int stride = 4;  /* Loop invariant */
        sum += arr[i * stride];
    }
    
    return sum;
}

/* Function B: Overlapping loops with goto and shared blocks */
static int function_b(int *arr, int n) {
    int result = 0;
    int i = 0, j = 0;
    
    /* First loop - partially overlaps with second */
    for (i = 0; i < n; i++) {
        result += arr[i];
        
        /* Conditional goto to shared block */
        if (result % 3 == 0) {
            goto shared_block;
        }
        
        continue_block:
        result -= arr[i] / 2;
    }
    
    /* Reset for second loop */
    result = 0;
    
    /* Second loop - shares block with first via goto */
    for (j = n - 1; j >= 0; j--) {
        result += arr[j] * 2;
        
        /* Different condition to goto same shared block */
        if (result % 5 == 0) {
            goto shared_block;
        }
        
        result -= arr[j];
    }
    
    return result;
    
shared_block:
    /* Shared basic block between the two loops */
    result = (result * 3) / 2;
    
    /* Jump back to appropriate location */
    if (i < n) {
        goto continue_block;
    }
    
    return result;
}

/* Function C: Loop with switch and break to outside */
static int function_c(int *arr, int n) {
    int total = 0;
    int state = 0;
    
    /* Loop with switch that can break out */
    for (int i = 0; i < n; i++) {
        total += arr[i];
        
        /* Switch with break to outside loop */
        switch (state) {
            case 0:
                if (total > LIMIT / 2) {
                    state = 1;
                }
                break;
            case 1:
                if (total > LIMIT * 3 / 4) {
                    /* This break exits the switch, not the loop */
                    break;
                }
                /* Fall through */
            case 2:
                /* Complex condition for loop exit */
                if (total > LIMIT) {
                    goto loop_exit;  /* Exit loop */
                }
                break;
            default:
                state = 0;
                break;
        }
        
        /* Multiple exit points */
        if (i > n / 2 && total < 0) {
            break;
        }
    }
    
loop_exit:
    return total;
}

/* Function D: Adjacent loops with potential overlap */
static int function_d(int *arr, int n) {
    int acc1 = 0, acc2 = 0;
    
    /* First adjacent loop */
    int m = n / 2;
    for (int i = 0; i < m; i++) {
        acc1 += arr[i];
        
        /* Early exit creates multiple blocks */
        if (acc1 > LIMIT) {
            break;
        }
    }
    
    /* Second adjacent loop - potentially overlapping if optimized */
    for (int i = m; i < n; i++) {
        acc2 += arr[i] * 2;
        
        /* Different exit condition */
        if (acc2 < -LIMIT) {
            break;
        }
    }
    
    /* Common tail computation */
    return acc1 + acc2;
}

int main(void) {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i % 100;
    }
    
    /* Call functions with different loop structures */
    int result = 0;
    
    /* Function A: Fully nested loops */
    result += function_a(arr, 32);
    
    /* Function B: Overlapping loops with goto */
    result += function_b(arr + 100, 64);
    
    /* Function C: Loop with switch and external break */
    result += function_c(arr + 200, 128);
    
    /* Function D: Adjacent loops */
    result += function_d(arr + 300, 256);
    
    /* Use volatile to prevent elimination */
    volatile int sink = result;
    
    /* Print to ensure code isn't eliminated */
    printf("Result: %d\n", result);
    
    return result % 256;
}
