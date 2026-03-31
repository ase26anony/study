#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 1000

/* Function A: Triple-nested fully contained loops */
static int triple_nested_loop(int *arr, int n) {
    int sum = 0;
    
    /* Outer loop - will contain inner loops */
    for (int i = 0; i < n / 4; i++) {
        /* Middle loop - fully contained in outer */
        for (int j = 0; j < 8; j++) {
            /* Innermost loop - fully contained in middle */
            for (int k = 0; k < 4; k++) {
                sum += arr[i * 16 + j * 4 + k];
                
                /* Multiple exit points */
                if (sum > LIMIT) {
                    sum = sum % LIMIT;
                }
            }
            
            /* Loop-invariant code with strength reduction potential */
            int stride = 2;
            if (j % stride == 0) {
                sum += arr[i * 16 + j];
            }
        }
        
        /* Another conditional break */
        if (i > n / 8 && sum > LIMIT / 2) {
            break;
        }
    }
    
    return sum;
}

/* Function B: Two loops with overlapping basic blocks via goto */
static int overlapping_loops(int *arr, int n) {
    int result = 0;
    int i = 0;
    
    /* First loop with goto to shared block */
loop1:
    while (i < n / 2) {
        result += arr[i];
        i++;
        
        /* Jump to shared code block */
        if (i % 3 == 0) {
            goto shared_block;
        }
        
        if (result > LIMIT) {
            result -= LIMIT;
        }
        
        continue;
        
shared_block:
        /* Shared basic block between two loops */
        result *= 2;
        if (result > 10000) {
            result = 10000;
        }
        /* Continue with loop1 */
    }
    
    /* Reset for second loop */
    int j = n / 2;
    
    /* Second loop that also uses the shared block */
    while (j < n) {
        result -= arr[j];
        j++;
        
        /* Same shared block accessed from second loop */
        if (j % 4 == 0) {
            goto shared_block;
        }
        
        /* Different exit condition */
        if (result < -LIMIT) {
            result = -LIMIT;
        }
    }
    
    return result;
}

/* Function C: Loop with switch statement and break to label */
static int switch_in_loop(int *arr, int n) {
    int total = 0;
    int counter = 0;
    
    for (int i = 0; i < n; i++) {
        /* Complex switch inside loop */
        switch (arr[i] % 5) {
            case 0:
                total += arr[i];
                break;
            case 1:
                total -= arr[i];
                /* This break only exits the switch, not the loop */
                break;
            case 2:
                total *= 2;
                /* Conditional break from loop via goto */
                if (total > 5000) {
                    goto loop_exit;
                }
                break;
            case 3:
                /* Nested loop inside switch case */
                for (int j = 0; j < 3; j++) {
                    total += j;
                    if (total < 0) {
                        total = 0;
                    }
                }
                break;
            case 4:
                total /= 2;
                /* Direct break from loop */
                if (i > n / 2) {
                    i = n; /* Force loop exit */
                }
                break;
        }
        
        counter++;
        if (counter > 100) {
            /* Another way to exit */
            break;
        }
    }
    
loop_exit:
    return total;
}

/* Function D: Adjacent loops with partial overlap */
static int adjacent_loops(int *arr, int n) {
    int sum1 = 0, sum2 = 0;
    
    /* First loop */
    for (int i = 0; i < n; i += 2) {
        sum1 += arr[i];
        
        /* Shared condition check */
        if (sum1 > 1000) {
            sum1 = 1000;
        }
        
        /* Jump to middle of second loop's body */
        if (i == n / 4) {
            goto partial_overlap;
        }
    }
    
    /* Second loop - partially overlaps with first */
    for (int j = 1; j < n; j += 2) {
        sum2 += arr[j];
        
partial_overlap:
        /* This label creates overlapping basic blocks */
        if (sum2 > 1000) {
            sum2 = 1000;
        }
        
        /* Complex exit condition */
        if (j > n * 3 / 4 && sum1 + sum2 > 1500) {
            break;
        }
    }
    
    return sum1 + sum2;
}

int main() {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i % 100; /* Keep values reasonable */
    }
    
    /* Call all functions to create various loop structures */
    int result1 = triple_nested_loop(arr, SIZE);
    int result2 = overlapping_loops(arr, SIZE);
    int result3 = switch_in_loop(arr + SIZE/4, SIZE/2);
    int result4 = adjacent_loops(arr + SIZE/2, SIZE/2);
    
    /* Combine results to prevent elimination */
    int final_result = result1 + result2 + result3 + result4;
    
    /* Use volatile to ensure computation isn't optimized away */
    volatile int sink = final_result;
    
    printf("Result: %d\n", final_result);
    
    return final_result % 256; /* Return non-zero to indicate execution */
}
