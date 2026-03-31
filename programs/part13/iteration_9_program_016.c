#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 1000

/* Function A: Triple-nested fully contained loops */
static int function_a(int *arr, int n) {
    int sum = 0;
    
    /* Outer loop - will contain inner loops */
    for (int i = 0; i < n; i++) {
        /* First inner loop - fully contained in outer */
        for (int j = 0; j < i; j++) {
            sum += arr[j];
            
            /* Innermost loop - fully contained in both outer loops */
            for (int k = 0; k < j; k++) {
                sum -= arr[k];
                if (sum > LIMIT) break;  /* Multiple exit point */
            }
            
            if (sum < -LIMIT) break;  /* Another exit point */
        }
        
        /* Second inner loop - also fully contained */
        int stride = 2;  /* Loop-invariant */
        for (int j = 0; j < n; j += stride) {
            sum += arr[i * stride + j % stride];  /* Complex addressing */
        }
    }
    
    return sum;
}

/* Function B: Overlapping loops with goto */
static int function_b(int *arr, int n) {
    int sum = 0;
    int i = 0;
    
    /* First loop */
    while (i < n) {
        sum += arr[i];
        i++;
        
        if (sum % 7 == 0) {
            goto common_block;  /* Jump to shared block */
        }
        
        continue;
        
    common_block:
        sum *= 2;
        break;  /* This creates overlapping control flow */
    }
    
    /* Second loop that shares the common_block */
    i = n / 2;
    while (i < n) {
        sum -= arr[i];
        i++;
        
        if (sum % 5 == 0) {
            goto common_block;  /* Both loops can jump here */
        }
    }
    
    return sum;
}

/* Function C: Loop with switch and break to outer label */
static int function_c(int *arr, int n) {
    int sum = 0;
    int i = 0;
    
outer_loop:
    while (i < n) {
        sum += arr[i];
        
        switch (sum % 4) {
            case 0:
                sum += 1;
                break;  /* Normal switch break */
            case 1:
                sum += 2;
                /* This break goes to the label, exiting the loop */
                goto exit_switch;
            case 2:
                sum += 3;
                /* Continue in loop */
                break;
            default:
                sum += 4;
                /* Complex control flow */
                if (sum > LIMIT / 2) {
                    goto outer_loop;  /* Jump to loop start */
                }
                break;
        }
        
        i++;
        continue;
        
    exit_switch:
        /* Target of the break from switch case 1 */
        i += 2;
        if (i >= n) break;
    }
    
    return sum;
}

/* Function D: Adjacent loops with partial overlap */
static int function_d(int *arr, int n) {
    int sum = 0;
    
    /* First adjacent loop */
    for (int i = 0; i < n/2; i++) {
        sum += arr[i];
        
        /* Conditional that could be shared */
        if (arr[i] > 0) {
            sum += 1;
        }
    }
    
    /* Second adjacent loop - shares some logic but not all */
    for (int i = n/2; i < n; i++) {
        sum -= arr[i];
        
        /* Same conditional structure - creates overlapping but not subset relationship */
        if (arr[i] > 0) {
            sum -= 1;
        }
        
        /* Additional block not in first loop */
        if (i % 3 == 0) {
            sum *= 2;
        }
    }
    
    return sum;
}

int main(void) {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i % 100;  /* Keep values reasonable */
    }
    
    /* Call all functions to create various loop structures */
    int result = 0;
    
    /* Function A: Fully nested loops */
    result += function_a(arr, SIZE / 4);
    
    /* Function B: Overlapping loops with goto */
    result += function_b(arr + SIZE/4, SIZE / 4);
    
    /* Function C: Loop with complex switch */
    result += function_c(arr + SIZE/2, SIZE / 4);
    
    /* Function D: Adjacent loops with partial overlap */
    result += function_d(arr + 3*SIZE/4, SIZE / 4);
    
    /* Use volatile to prevent elimination */
    volatile int sink = result;
    
    /* Print to ensure code isn't eliminated */
    printf("Result: %d\n", result);
    
    return result % 256;  /* Return non-zero to indicate execution */
}
