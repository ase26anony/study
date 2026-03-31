#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define LIMIT 1000

/* Function A: Triple-nested fully contained loops */
static int triple_nested_loop(int* arr, int n) {
    int sum = 0;
    
    /* Outer loop - will contain inner loops */
    for (int i = 0; i < n; i++) {
        /* First inner loop - fully contained in outer */
        for (int j = 0; j < i; j++) {
            /* Innermost loop - fully contained in both outer loops */
            for (int k = 0; k < j; k++) {
                sum += arr[i] * arr[j] + arr[k];
                /* Multiple exit points */
                if (sum > LIMIT) {
                    break;
                }
            }
            /* Another conditional break */
            if (sum > LIMIT * 2) {
                break;
            }
        }
        
        /* Loop-invariant code for strength reduction */
        int stride = 2;  /* Loop invariant */
        for (int j = 0; j < n; j += stride) {
            sum += arr[i * stride + j % n];
            if (sum < 0) break;  /* Another exit point */
        }
    }
    return sum;
}

/* Function B: Overlapping loops with goto and shared blocks */
static int overlapping_loops(int* arr, int n) {
    int result = 0;
    int i = 0;
    
    /* First loop */
loop1_start:
    for (; i < n/2; i++) {
        result += arr[i] * 2;
        
        /* Shared basic block via goto */
        if (result % 7 == 0) {
            goto shared_block;
        }
        
        if (result > LIMIT) {
            break;
        }
    }
    
    i = n/2;
    
    /* Second loop - partially overlaps with first via shared_block */
    for (; i < n; i++) {
        result -= arr[i];
        
shared_block:
        /* This label creates a shared basic block between the two loops */
        result += i;
        
        if (result < 0) {
            break;
        }
        
        /* Jump back to first loop sometimes */
        if (i % 3 == 0 && i < n - 10) {
            goto loop1_start;
        }
    }
    
    return result;
}

/* Function C: Loop with switch and complex break */
static int switch_in_loop(int* arr, int n) {
    int total = 0;
    int state = 0;
    
    for (int i = 0; i < n; i++) {
        total += arr[i];
        
        /* Switch with break to label outside loop */
        switch (state) {
            case 0:
                if (total > 500) {
                    state = 1;
                }
                total += 1;
                break;
            case 1:
                if (total > 700) {
                    /* This break exits the switch, not the loop */
                    break;
                }
                total += arr[i] * 3;
                if (total > 900) {
                    /* Complex control flow */
                    goto exit_loop;
                }
                break;
            case 2:
                total -= 5;
                break;
        }
        
        /* Multiple basic blocks within loop */
        if (i % 2 == 0) {
            total *= 2;
        } else {
            total /= 2;
        }
        
        /* Another exit point */
        if (total > 2000) {
            break;
        }
    }
    
    return total;

exit_loop:
    /* Label outside the loop for goto from switch */
    return total + 1000;
}

/* Function D: Adjacent loops with potential intersection */
static int adjacent_loops(int* arr, int n) {
    int acc1 = 0, acc2 = 0;
    
    /* First loop */
    for (int i = 0; i < n; i += 2) {
        acc1 += arr[i];
        if (acc1 > 500) {
            /* Early exit creates more basic blocks */
            break;
        }
    }
    
    /* Second loop - adjacent but not overlapping */
    for (int i = 1; i < n; i += 2) {
        acc2 += arr[i];
        /* Shared computation pattern but different blocks */
        if (acc2 > 500) {
            break;
        }
    }
    
    /* Third loop that could intersect with both */
    int i = 0;
    while (i < n) {
        int val = arr[i];
        if (val % 2 == 0) {
            acc1 += val;
            i += 1;
        } else {
            acc2 += val;
            i += 2;
        }
        
        /* Conditional that could create shared edges */
        if (acc1 + acc2 > 1500) {
            goto finish;
        }
    }

finish:
    return acc1 + acc2;
}

int main() {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i % 100;  /* Keep values reasonable */
    }
    
    /* Call all functions to create various loop structures */
    int result = 0;
    
    /* Function A: Fully contained nested loops */
    result += triple_nested_loop(arr, 64);
    
    /* Function B: Overlapping loops with goto */
    result += overlapping_loops(arr + 64, 128);
    
    /* Function C: Loop with switch and complex break */
    result += switch_in_loop(arr + 192, 128);
    
    /* Function D: Adjacent loops */
    result += adjacent_loops(arr + 320, 128);
    
    /* Use volatile to prevent elimination */
    volatile int sink = result;
    
    /* Print to ensure code isn't eliminated */
    printf("Result: %d\n", result);
    
    return result % 256;
}
