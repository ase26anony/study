#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 1000

/* Function A: Triple-nested fully contained loops */
static int triple_nested_loop(int *arr, int n) {
    int sum = 0;
    /* Outer loop - will contain inner loops */
    for (int i = 0; i < n; i += 4) {
        /* Middle loop - fully contained in outer */
        for (int j = i; j < i + 3 && j < n; j++) {
            /* Inner loop - fully contained in middle */
            for (int k = j; k < j + 2 && k < n; k++) {
                sum += arr[k] * (k % 7);
                /* Multiple exit point */
                if (sum > LIMIT) break;
            }
            /* Loop-invariant code */
            int stride = 3;  /* Loop invariant */
            if (i + stride < n) {
                sum += arr[i + stride];
            }
        }
        /* Another inner loop at same level */
        for (int m = 0; m < 2 && i + m < n; m++) {
            sum -= arr[i + m];
        }
    }
    return sum;
}

/* Function B: Overlapping loops with goto */
static int overlapping_loops(int *arr, int n) {
    int result = 0;
    int i = 0;
    
    /* First loop */
    while (i < n / 2) {
        result += arr[i] * 2;
        i++;
        if (result > LIMIT / 2) {
            goto common_block;  /* Jump to shared block */
        }
    }
    
    /* Reset for second loop */
    i = n / 4;
    
    /* Second loop - partially overlaps with first */
    while (i < n * 3 / 4) {
        result -= arr[i];
        i++;
        if (result < -LIMIT) break;
common_block:  /* Shared basic block */
        result = (result & 0xFF) + 1;  /* Common operation */
    }
    
    /* Third loop that shares some blocks via goto */
    for (int j = 0; j < n; j += 2) {
        if (arr[j] % 3 == 0) {
            goto update_result;
        }
        result += j;
        continue;
update_result:  /* Another shared block */
        result -= arr[j];
        /* Complex control flow back */
        if (j % 4 == 0) goto end_loop;
    }
end_loop:
    
    return result;
}

/* Function C: Loop with switch and break to outer label */
static int switch_in_loop(int *arr, int n) {
    int total = 0;
    int i = 0;
    
outer_loop:
    while (i < n) {
        switch (arr[i] % 5) {
            case 0:
                total += arr[i];
                i++;
                break;  /* Breaks from switch, not loop */
            case 1:
                total -= arr[i];
                i += 2;
                break;
            case 2:
                /* This break exits the while loop entirely */
                goto exit_loop;
            case 3:
                total *= 2;
                i++;
                /* Nested switch inside case */
                switch (total % 3) {
                    case 0:
                        goto outer_loop;  /* Jumps to outer label */
                    case 1:
                        break;
                }
                break;
            case 4:
                /* Inner loop inside switch case */
                for (int j = 0; j < 3 && i + j < n; j++) {
                    total += arr[i + j] * j;
                }
                i += 3;
                break;
        }
        
        /* Multiple exit point */
        if (total > LIMIT * 2) {
            break;
        }
        
        /* Another conditional break */
        if (i > n / 2 && total < 0) {
            goto exit_loop;
        }
    }
    
exit_loop:
    
    /* Post-loop processing that might be analyzed as part of loop */
    for (int k = 0; k < 10 && k < n; k++) {
        total += k;
    }
    
    return total;
}

/* Function D: Complex nested loops with partial overlap */
static int complex_partial_overlap(int *arr, int n) {
    int acc = 0;
    
    /* Loop A */
    for (int a = 0; a < n; a += 2) {
        acc += arr[a];
        
        /* Loop B - partially overlapping with Loop C */
        for (int b = a; b < a + 5 && b < n; b++) {
            acc -= arr[b] * (b % 3);
            
            /* Early exit that jumps to shared code */
            if (acc < -500) {
                goto shared_computation;
            }
        }
        
        /* Loop C - shares some blocks with Loop B but not all */
        for (int c = a + 1; c < a + 6 && c < n; c++) {
            acc += arr[c] * 2;
shared_computation:  /* Shared block between B and C */
            acc = (acc + 1) % 100;
            if (c % 2 == 0) {
                /* Jump to block that's in both loops' flow */
                goto update_counter;
            }
            continue;
update_counter:
            arr[c] = acc;
        }
    }
    
    return acc;
}

int main() {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = (i * 3 + 7) % 100;  /* Non-trivial pattern */
    }
    
    /* Call functions with different slices to create varied loop structures */
    int result1 = triple_nested_loop(arr, SIZE);
    int result2 = overlapping_loops(arr + SIZE/4, SIZE/2);
    int result3 = switch_in_loop(arr + SIZE/2, SIZE/2);
    int result4 = complex_partial_overlap(arr, SIZE);
    
    /* Combine results to prevent elimination */
    int final_result = result1 + result2 + result3 + result4;
    
    /* Use volatile to ensure computation isn't optimized away */
    volatile int sink = final_result;
    
    printf("Result: %d\n", final_result);
    
    return final_result % 256;  /* Return non-zero to indicate execution */
}
