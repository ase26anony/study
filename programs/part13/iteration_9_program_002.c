#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 1000

/* Function A: Triple-nested fully contained loops */
static int triple_nested_loop(int *arr, int n) {
    int sum = 0;
    int stride = 4; /* Loop-invariant variable */
    
    /* Outer loop */
    for (int i = 0; i < n; i++) {
        /* Middle loop - fully contained in outer */
        for (int j = 0; j < i; j++) {
            /* Inner loop - fully contained in middle */
            for (int k = 0; k < j; k++) {
                /* Loop-invariant code with strength reduction potential */
                sum += arr[k * stride % n];
                
                /* Multiple exit points */
                if (sum > LIMIT) {
                    break; /* Exits innermost loop only */
                }
            }
            
            /* Another conditional break in middle loop */
            if (sum > LIMIT * 2) {
                break;
            }
            
            sum += arr[j];
        }
        
        /* Complex exit condition */
        if (i > n / 2 && sum > LIMIT) {
            break; /* Exits outer loop */
        }
        
        sum += arr[i];
    }
    
    return sum;
}

/* Function B: Overlapping loops with goto and shared blocks */
static int overlapping_loops(int *arr, int n) {
    int sum = 0;
    int i = 0, j = 0;
    
    /* First loop */
    for (i = 0; i < n; i++) {
        sum += arr[i];
        
        /* Conditional goto to shared block */
        if (arr[i] % 3 == 0) {
            goto shared_block;
        }
        
        continue;
        
shared_block:
        /* Shared basic block between both loops */
        sum += 1;
        
        /* Multiple exit points */
        if (sum > LIMIT) {
            break;
        }
    }
    
    /* Second loop that shares the 'shared_block' */
    for (j = n - 1; j >= 0; j--) {
        sum -= arr[j];
        
        /* Same goto target as first loop */
        if (arr[j] % 5 == 0) {
            goto shared_block;
        }
        
        /* Different exit condition */
        if (sum < 0) {
            break;
        }
    }
    
    return sum;
}

/* Function C: Loop with switch and break to outer label */
static int switch_in_loop(int *arr, int n) {
    int sum = 0;
    
    /* Outer loop */
    for (int i = 0; i < n; i++) {
        /* Switch inside loop */
        switch (arr[i] % 4) {
            case 0:
                sum += arr[i];
                break; /* This break is for switch, not loop */
                
            case 1:
                sum -= arr[i];
                /* Fall through */
                
            case 2:
                sum *= 2;
                /* Use goto to break out of loop from switch */
                if (sum > LIMIT) {
                    goto loop_exit;
                }
                break;
                
            case 3:
                /* Nested loop inside switch case */
                for (int j = 0; j < i; j++) {
                    sum += arr[j];
                    if (sum > LIMIT * 2) {
                        goto loop_exit; /* Breaks outer loop */
                    }
                }
                break;
        }
        
        /* Multiple exit points */
        if (i > n / 3 && sum < 0) {
            break;
        }
    }
    
loop_exit:
    return sum;
}

/* Function D: Irreducible control flow with while and do-while */
static int irreducible_loops(int *arr, int n) {
    int sum = 0;
    int i = 0;
    
    /* First loop structure */
    while (i < n) {
        sum += arr[i];
        i++;
        
        /* Jump to middle of next loop */
        if (sum % 7 == 0) {
            goto middle_of_second;
        }
    }
    
    i = 0;
    
    /* Second loop that can be entered in the middle */
second_loop:
    while (i < n) {
        sum -= arr[i];
        
middle_of_second:
        i++;
        
        /* Jump back to first loop */
        if (sum % 11 == 0 && i < n / 2) {
            goto second_loop;
        }
        
        /* Complex exit */
        if (sum > LIMIT || sum < -LIMIT) {
            break;
        }
    }
    
    return sum;
}

int main() {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i + 1;
    }
    
    int result = 0;
    
    /* Call all functions to create various loop structures */
    result += triple_nested_loop(arr, SIZE / 4);
    result += overlapping_loops(arr + SIZE / 4, SIZE / 4);
    result += switch_in_loop(arr + SIZE / 2, SIZE / 4);
    result += irreducible_loops(arr + 3 * SIZE / 4, SIZE / 4);
    
    /* Use volatile to prevent dead code elimination */
    volatile int sink = result;
    
    /* Print result to ensure code isn't eliminated */
    printf("Result: %d\n", result);
    
    return result % 256;
}
