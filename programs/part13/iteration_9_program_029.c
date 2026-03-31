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
            /* Inner loop - fully contained in middle */
            for (int k = 0; k < j; k++) {
                sum += arr[i * 16 + j * 8 + k];
                if (sum > LIMIT) break;  /* Multiple exit point */
            }
            sum += arr[j] * 2;
        }
        /* Loop-invariant code */
        int stride = 4;  /* Loop invariant */
        sum += arr[i * stride];
    }
    return sum;
}

/* Function B: Loops with overlapping basic blocks via goto */
static int function_b(int *arr, int n) {
    int result = 0;
    int i = 0, j = 0;
    
    /* First loop - shares common block with second loop */
    for (i = 0; i < n/2; i++) {
        result += arr[i] * 3;
        if (result % 7 == 0) {
            goto common_block;  /* Jump to shared basic block */
        }
        result -= arr[i];
    }
    
    /* Second loop - partially overlaps with first */
    for (j = n/2; j < n; j++) {
        result += arr[j] * 2;
common_block:  /* Shared basic block label */
        result += 1;  /* Common code executed by both loops */
        if (result > LIMIT * 2) break;
    }
    
    return result;
}

/* Function C: Loop with switch and break to outside */
static int function_c(int *arr, int n) {
    int total = 0;
    int state = 0;
    
    for (int i = 0; i < n; i++) {
        total += arr[i];
        
        /* Switch with break targeting outside the loop */
        switch (state) {
            case 0:
                if (total > 500) state = 1;
                break;
            case 1:
                if (total > 750) state = 2;
                break;
            case 2:
                /* This break exits the switch, not the loop */
                break;
            default:
                /* Complex control flow: break to label outside loop */
                goto loop_exit;
        }
        
        /* Additional exit point */
        if (total > LIMIT) {
            break;
        }
        
        /* Nested while loop inside for */
        int counter = 0;
        while (counter < 5) {
            total += counter;
            counter++;
            if (total % 11 == 0) break;
        }
    }
    
loop_exit:
    return total;
}

/* Function D: Adjacent loops with shared computation */
static int function_d(int *arr, int n) {
    int acc1 = 0, acc2 = 0;
    
    /* First adjacent loop */
    for (int i = 0; i < n/3; i++) {
        acc1 += arr[i] * i;
        if (acc1 > 300) {
            /* Early exit creates additional basic block */
            break;
        }
    }
    
    /* Shared computation block */
    int shared = acc1 % 13;
    
    /* Second adjacent loop - not nested, but shares some control flow */
    for (int j = n/3; j < 2*n/3; j++) {
        acc2 += arr[j] + shared;
        /* Multiple exit points */
        if (acc2 > 400) break;
        if (j % 5 == 0) continue;
        acc2 -= 1;
    }
    
    return acc1 + acc2;
}

int main(void) {
    /* Initialize array with sequential values */
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i % 100;  /* Keep values reasonable */
    }
    
    /* Call functions with different loop structures */
    int result_a = function_a(arr, 32);      /* Triple nested */
    int result_b = function_b(arr, 64);      /* Overlapping with goto */
    int result_c = function_c(arr, 128);     /* Switch with complex break */
    int result_d = function_d(arr, 96);      /* Adjacent loops */
    
    /* Combine results to prevent elimination */
    int final_result = result_a + result_b + result_c + result_d;
    
    /* Use volatile to ensure computation isn't optimized away */
    volatile int sink = final_result;
    
    printf("Result: %d\n", final_result);
    return final_result % 256;
}
