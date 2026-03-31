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
                if (sum > LIMIT) break;  /* Multiple exit point */
            }
            sum += arr[j] * 2;  /* Loop-invariant code opportunity */
        }
        /* Additional basic block in outer loop */
        if (i % 2 == 0) {
            sum -= arr[i];
        }
    }
    return sum;
}

/* Function B: Overlapping loops with goto and shared blocks */
static int function_b(int *arr, int n) {
    int result = 0;
    int i = 0;
    
    /* First loop with partial overlap */
    while (i < n) {
        result += arr[i];
        if (result > LIMIT / 2) {
            goto shared_block;  /* Jump to shared basic block */
        }
        i++;
    }
    
    i = n / 2;  /* Reset for second loop */
    
    /* Second loop that overlaps with first via shared block */
    while (i < n) {
        result += arr[i] * 3;
        if (result < 0) break;
        i++;
        if (i == n - 1) {
shared_block:
            result += 100;  /* Shared basic block */
            break;
        }
    }
    
    return result;
}

/* Function C: Loop with switch and complex break */
static int function_c(int *arr, int n) {
    int total = 0;
    int state = 0;
    
    for (int i = 0; i < n; i++) {
        total += arr[i];
        
        /* Switch with break to outer label */
        switch (state) {
            case 0:
                if (total > 500) {
                    state = 1;
                    break;  /* This breaks from switch only */
                }
                total += i;
                break;
            case 1:
                if (total > 750) {
                    goto loop_exit;  /* Breaks from entire loop */
                }
                total -= arr[i];
                break;
            default:
                total *= 2;
                break;
        }
        
        /* Another basic block in loop */
        if (i % 3 == 0) {
            total += 1;
        }
    }
    
loop_exit:
    return total;
}

/* Function D: Adjacent loops with potential overlap */
static int function_d(int *arr, int n) {
    int acc1 = 0, acc2 = 0;
    
    /* First adjacent loop */
    for (int i = 0; i < n/2; i++) {
        acc1 += arr[i * 2];
        if (acc1 > 300) {
            /* Early exit creates additional basic block */
            break;
        }
    }
    
    /* Second adjacent loop - shares no blocks but compiler might
       create shared prologue/epilogue blocks */
    for (int j = n/2; j < n; j++) {
        acc2 += arr[j] * arr[j - n/2];
        /* Multiple exit points */
        if (acc2 > 400) break;
        if (j == n - 1) acc2 += 50;
    }
    
    return acc1 + acc2;
}

int main(void) {
    int arr[SIZE];
    int final_result = 0;
    
    /* Initialize array with sequential values */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i % 100;  /* Keep values reasonable */
    }
    
    /* Call all functions to create various loop structures */
    final_result += function_a(arr, 32);    /* Triple nested */
    final_result += function_b(arr, 64);    /* Overlapping with goto */
    final_result += function_c(arr, 128);   /* Switch with break */
    final_result += function_d(arr, 256);   /* Adjacent loops */
    
    /* Use volatile to prevent dead code elimination */
    volatile int sink = final_result;
    
    /* Print to ensure code isn't eliminated */
    printf("Result: %d\n", final_result);
    
    return final_result % 256;  /* Return non-constant value */
}
