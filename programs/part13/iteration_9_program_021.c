#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 1000

/* Function A: Triple-nested fully contained loops */
static int function_a(int *arr, int n) {
    int sum = 0;
    int stride = 3; /* Loop-invariant variable */
    
    /* Outer loop - will contain inner loops */
    for (int i = 0; i < n; i += stride) {
        /* First inner loop - fully contained in outer */
        for (int j = 0; j < 5; j++) {
            sum += arr[i + j];
            
            /* Second inner loop - fully contained in first inner */
            for (int k = 0; k < 2; k++) {
                sum -= arr[i + j + k];
                if (sum < 0) break; /* Multiple exit point */
            }
            
            /* Another exit condition */
            if (sum > LIMIT) break;
        }
        
        /* Another exit from outer loop */
        if (i > n / 2 && sum > LIMIT / 2) break;
    }
    
    return sum;
}

/* Function B: Overlapping loops with goto and shared blocks */
static int function_b(int *arr, int n) {
    int sum = 0;
    int i = 0;
    
    /* First loop */
    while (i < n) {
        sum += arr[i];
        i++;
        
        /* Shared basic block via label */
        if (sum % 7 == 0) {
            goto shared_block;
        }
        
        if (i > n / 3) break;
        
        continue;
        
    shared_block:
        sum += 100;
        /* Falls through to second loop entry */
    }
    
    /* Second loop that shares the shared_block */
    i = n / 2;
    while (i < n) {
        if (i % 2 == 0) {
            goto shared_block; /* Jump to shared block */
        }
        
        sum -= arr[i];
        i++;
        
        /* Multiple exit points */
        if (sum < -LIMIT) break;
        if (i > n * 3 / 4) break;
    }
    
    return sum;
}

/* Function C: Loop with switch and break to outer label */
static int function_c(int *arr, int n) {
    int sum = 0;
    int state = 0;
    
    /* Loop with complex switch inside */
    for (int i = 0; i < n; i++) {
        switch (state) {
            case 0:
                sum += arr[i];
                if (sum % 11 == 0) {
                    state = 1;
                    /* This break breaks the switch, not the loop */
                    break;
                }
                break;
                
            case 1:
                sum -= arr[i] * 2;
                if (sum % 13 == 0) {
                    /* This will exit the entire loop */
                    goto loop_exit;
                }
                state = 0;
                break;
                
            default:
                /* Unreachable but creates additional basic block */
                sum = 0;
                break;
        }
        
        /* Another exit point */
        if (sum > LIMIT * 2) {
            break;
        }
        
        continue;
        
    loop_exit:
        /* Target of goto from inside switch */
        sum += 1000;
        break;
    }
    
    return sum;
}

/* Function D: Adjacent loops with partial overlap */
static int function_d(int *arr, int n) {
    int sum = 0;
    int i, j;
    
    /* First loop */
    i = 0;
    while (i < n / 2) {
        sum += arr[i];
        
        /* Conditional jump to block shared with next loop */
        if (sum % 17 == 0) {
            goto partial_shared;
        }
        
        i++;
        if (i > n / 4) break;
    }
    
    /* Second loop - partially overlaps with first */
    j = n / 4;
    while (j < n * 3 / 4) {
        sum -= arr[j];
        
    partial_shared:
        /* This block can be reached from both loops */
        sum += arr[j % n];
        
        j++;
        if (j > n * 2 / 3) break;
    }
    
    return sum;
}

int main(void) {
    int arr[SIZE];
    int result = 0;
    
    /* Initialize array with sequential values */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i % 100; /* Keep values reasonable */
    }
    
    /* Call all functions to create various loop structures */
    result += function_a(arr, SIZE / 2);
    result += function_b(arr + SIZE / 4, SIZE / 2);
    result += function_c(arr + SIZE / 3, SIZE / 3);
    result += function_d(arr, SIZE);
    
    /* Use volatile to prevent dead code elimination */
    volatile int sink = result;
    
    /* Print to ensure code isn't eliminated */
    printf("Result: %d\n", result);
    
    return result % 256;
}
