#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 1000

/* Function A: Triple-nested fully contained loops */
static int triple_nested_loop(int *arr, int n) {
    int sum = 0;
    int stride = 3; /* Loop-invariant variable */
    
    /* Outer loop - will contain inner loops */
    for (int i = 1; i < n - 1; i++) {
        /* First inner loop - fully contained in outer */
        for (int j = 1; j < i && j < 10; j++) {
            sum += arr[i * stride + j];
            if (sum > LIMIT) break; /* Multiple exit point */
        }
        
        /* Second inner loop - also fully contained */
        for (int k = 0; k < 5; k++) {
            sum -= arr[i + k];
            /* Nested while loop inside for */
            int m = 0;
            while (m < 3) {
                sum += m;
                m++;
                if (sum < -LIMIT) break; /* Another exit point */
            }
        }
    }
    return sum;
}

/* Function B: Overlapping loops with goto creating shared blocks */
static int overlapping_loops(int *arr, int n) {
    int result = 0;
    int i = 0;
    
    /* First loop */
    for (i = 0; i < n/2; i++) {
        result += arr[i];
        if (result > LIMIT) {
            goto common_block; /* Jump to shared block */
        }
    }
    
    /* Shared basic block via label */
common_block:
    result *= 2;
    
    /* Second loop that also uses the common block */
    for (int j = n/2; j < n; j++) {
        result -= arr[j];
        if (j % 3 == 0) {
            goto common_block; /* Both loops can reach common_block */
        }
        if (result < 0) break;
    }
    
    /* Third loop that's adjacent but doesn't share blocks */
    for (int k = 0; k < 10; k++) {
        result += k;
    }
    
    return result;
}

/* Function C: Loop with switch containing break to exit loop */
static int switch_in_loop(int *arr, int n) {
    int total = 0;
    int accumulator = 0;
    
    for (int i = 0; i < n; i++) {
        accumulator += arr[i];
        
        /* Switch with break that can exit the loop */
        switch (i % 4) {
            case 0:
                total += arr[i];
                break;
            case 1:
                total -= arr[i];
                if (accumulator > LIMIT) {
                    goto loop_exit; /* Alternative exit */
                }
                break;
            case 2:
                total *= 2;
                /* This break exits the switch, not the loop */
                break;
            case 3:
                /* This will exit the entire loop when condition met */
                if (total > 10000) {
                    i = n; /* Force loop exit */
                }
                break;
        }
        
        /* Multiple basic blocks within loop */
        if (total < 0) {
            total = -total;
        } else {
            total += 1;
        }
    }
loop_exit:
    
    /* Another loop with complex control flow */
    int j = 0;
    while (j < n) {
        if (j % 2 == 0) {
            for (int k = 0; k < 3; k++) {
                total += arr[j + k];
                if (total > 5000) break; /* Exits inner for only */
            }
        }
        j++;
        if (j > 50) break; /* Exits while loop */
    }
    
    return total;
}

/* Function D: Mixed loops with partial overlap */
static int mixed_partial_overlap(int *arr, int n) {
    int val = 0;
    
    /* Loop 1 */
    int x = 0;
    while (x < n) {
        val += arr[x];
        x += 2;
        if (val > 1000) {
            /* Jump to block shared with next loop */
            goto partial_shared;
        }
    }
    
    /* Loop 2 - partially overlaps with Loop 1 */
    for (int y = 1; y < n; y += 2) {
        val -= arr[y];
partial_shared:
        val += y;
        if (y % 5 == 0) {
            /* This creates another shared block possibility */
            val *= 2;
        }
    }
    
    return val;
}

int main() {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i % 100; /* Prevent overflow in calculations */
    }
    
    /* Call all functions to create various loop structures */
    int result1 = triple_nested_loop(arr, SIZE / 4);
    int result2 = overlapping_loops(arr + SIZE / 4, SIZE / 4);
    int result3 = switch_in_loop(arr + SIZE / 2, SIZE / 4);
    int result4 = mixed_partial_overlap(arr + 3 * SIZE / 4, SIZE / 4);
    
    /* Combine results to prevent elimination */
    int final_result = result1 + result2 + result3 + result4;
    
    /* Use volatile to ensure computation isn't optimized away */
    volatile int sink = final_result;
    
    printf("Result: %d\n", final_result);
    
    return final_result % 256; /* Return non-constant value */
}
