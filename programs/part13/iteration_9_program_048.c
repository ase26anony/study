#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define INNER_SIZE 8
#define MID_SIZE 16

static volatile int sink;

/* Function A: Triple-nested fully contained loops */
static int function_a(int *arr, int n) {
    int sum = 0;
    
    /* Outer loop - fully contains inner loops */
    for (int i = 0; i < n; i += MID_SIZE) {
        /* Middle loop - fully contained in outer */
        for (int j = 0; j < MID_SIZE && (i + j) < n; j++) {
            /* Innermost loop - fully contained in middle */
            for (int k = 0; k < INNER_SIZE; k++) {
                sum += arr[i + j] * k;
                if (sum > 1000000) break;  /* Multiple exit point */
            }
            /* Loop-invariant code */
            int stride = 2;
            sum += arr[i + j * stride % n];
        }
        
        /* Another inner loop at same level as middle loop */
        for (int m = 0; m < INNER_SIZE && i + m < n; m++) {
            sum -= arr[i + m];
            if (sum < 0) break;
        }
    }
    return sum;
}

/* Function B: Loops with overlapping basic blocks via goto */
static int function_b(int *arr, int n) {
    int result = 0;
    int i = 0;
    
    /* First loop */
loop1:
    while (i < n / 2) {
        result += arr[i] * 3;
        i++;
        
        /* Shared basic block via goto */
        if (result % 7 == 0)
            goto shared_block;
        
        if (i % 5 == 0)
            break;
        continue;
        
    shared_block:
        result >>= 1;
        /* This block is shared between both loops */
        if (result > 1000) {
            goto next_loop;
        }
    }
    
    /* Second loop that shares basic block with first */
    i = n / 2;
next_loop:
    while (i < n) {
        result -= arr[i];
        i += 2;
        
        /* Jump to shared block */
        if (result % 11 == 0)
            goto shared_block;
        
        /* Multiple exit points */
        if (result < -5000) break;
        if (i > n - 10) break;
    }
    
    return result;
}

/* Function C: Loop with switch and break to outer label */
static int function_c(int *arr, int n) {
    int total = 0;
    int state = 0;
    
    /* Outer loop */
    for (int i = 0; i < n; i++) {
        total += arr[i];
        
        /* Switch inside loop with break to label */
        switch (state) {
            case 0:
                if (total > 5000) {
                    state = 1;
                    /* This break goes to switch, not loop */
                    break;
                }
                total += i;
                break;
            case 1:
                if (total < -5000) {
                    /* Break out of loop entirely */
                    goto loop_exit;
                }
                total -= arr[i] * 2;
                break;
            default:
                /* Complex control flow */
                if (i % 3 == 0) {
                    continue;  /* Skip to next iteration */
                }
        }
        
        /* Another inner loop */
        for (int j = 0; j < 4 && i + j < n; j++) {
            total += arr[i + j] / (j + 1);
            if (total > 10000) {
                /* Break inner loop only */
                break;
            }
        }
        
        if (i % 20 == 0) {
            /* Early exit from outer loop */
            break;
        }
    }
loop_exit:
    
    return total;
}

/* Function D: Irreducible control flow with overlapping loops */
static int function_d(int *arr, int n) {
    int acc = 0;
    int i = 0, j = n - 1;
    
    /* Two loops that partially overlap */
    
    /* Loop X */
start_x:
    while (i < n / 3) {
        acc += arr[i] * 2;
        i++;
        
        if (acc % 13 == 0) {
            goto overlap_block;
        }
        
        if (i > n / 4) {
            goto start_y;
        }
    }
    
    /* Loop Y */
start_y:
    while (j > n / 2) {
        acc -= arr[j];
        j--;
        
        if (acc % 17 == 0) {
            goto overlap_block;
        }
        
        if (j < 2 * n / 3) {
            goto start_x;
        }
    }
    
    goto finish;
    
overlap_block:
    /* This block belongs to both loops */
    acc = (acc * 3) / 2;
    if (i < n / 3) {
        goto start_x;
    } else {
        goto start_y;
    }
    
finish:
    return acc;
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
    result += function_a(arr, SIZE);
    sink = result;  /* Prevent elimination */
    
    /* Function B: Overlapping loops via goto */
    result += function_b(arr + 100, SIZE - 100);
    sink = result;
    
    /* Function C: Loop with switch and complex breaks */
    result += function_c(arr + 200, SIZE - 200);
    sink = result;
    
    /* Function D: Irreducible control flow */
    result += function_d(arr + 300, SIZE - 300);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result % 256;
}
